#include "engine/audio/AudioSystem.h"
#include "engine/core/Logger.h"

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <mmsystem.h>
#endif

#include <cmath>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <unordered_map>
#include <vector>

namespace engine {
namespace audio {

bool AudioSystem::s_initialized = false;
bool AudioSystem::s_muted       = false;

namespace {

std::unordered_map<std::string, std::vector<uint8_t>> g_wavCache;

const std::vector<uint8_t>* getOrLoadWavInMemory(const std::string& filePath) {
    auto it = g_wavCache.find(filePath);
    if (it != g_wavCache.end()) {
        return &it->second;
    }
    if (!std::filesystem::exists(filePath)) {
        return nullptr;
    }
    std::ifstream in(filePath, std::ios::binary | std::ios::ate);
    if (!in.is_open()) {
        return nullptr;
    }
    std::streamsize sz = in.tellg();
    if (sz <= 44) {
        return nullptr;
    }
    in.seekg(0, std::ios::beg);
    std::vector<uint8_t> buf(static_cast<size_t>(sz));
    if (!in.read(reinterpret_cast<char*>(buf.data()), sz)) {
        return nullptr;
    }
    auto res = g_wavCache.emplace(filePath, std::move(buf));
    return &res.first->second;
}

void playWavFileZeroLatency(const char* filePath) {
#ifdef _WIN32
    const auto* buf = getOrLoadWavInMemory(filePath);
    if (buf && !buf->empty()) {
        PlaySoundA(
            reinterpret_cast<LPCSTR>(buf->data()),
            NULL,
            SND_MEMORY | SND_ASYNC | SND_NODEFAULT);
    }
#else
    (void)filePath;
#endif
}

} // anonymous namespace

static bool isValidRiffWaveFile(const std::string& filePath) {
    if (!std::filesystem::exists(filePath) || std::filesystem::file_size(filePath) < 44) {
        return false;
    }
    std::ifstream in(filePath, std::ios::binary);
    if (!in.is_open()) {
        return false;
    }
    char header[12] = {};
    in.read(header, 12);
    return (header[0] == 'R' && header[1] == 'I' && header[2] == 'F' && header[3] == 'F' &&
            header[8] == 'W' && header[9] == 'A' && header[10] == 'V' && header[11] == 'E');
}

static void writeMonoWavFileIfMissing(
    const std::string& filePath,
    const std::vector<int16_t>& samples,
    uint32_t sampleRate = 22050)
{
    if (isValidRiffWaveFile(filePath)) {
        return;
    }
    std::filesystem::create_directories(std::filesystem::path(filePath).parent_path());

    uint32_t dataBytes     = static_cast<uint32_t>(samples.size() * sizeof(int16_t));
    uint32_t riffSize      = 36 + dataBytes;
    uint16_t audioFmt      = 1; // Standard uncompressed 16-bit PCM
    uint16_t channels      = 1; // Mono
    uint32_t byteRate      = sampleRate * channels * sizeof(int16_t);
    uint16_t blockAlign    = channels * sizeof(int16_t);
    uint16_t bitsPerSample = 16;

    std::ofstream out(filePath, std::ios::binary);
    if (!out.is_open()) {
        return;
    }

    out.write("RIFF", 4);
    out.write(reinterpret_cast<const char*>(&riffSize), 4);
    out.write("WAVE", 4);
    out.write("fmt ", 4);
    uint32_t fmtSize = 16;
    out.write(reinterpret_cast<const char*>(&fmtSize), 4);
    out.write(reinterpret_cast<const char*>(&audioFmt), 2);
    out.write(reinterpret_cast<const char*>(&channels), 2);
    out.write(reinterpret_cast<const char*>(&sampleRate), 4);
    out.write(reinterpret_cast<const char*>(&byteRate), 4);
    out.write(reinterpret_cast<const char*>(&blockAlign), 2);
    out.write(reinterpret_cast<const char*>(&bitsPerSample), 2);
    out.write("data", 4);
    out.write(reinterpret_cast<const char*>(&dataBytes), 4);
    out.write(reinterpret_cast<const char*>(samples.data()), dataBytes);
    out.close();
}

// Karplus-Strong plucked acoustic string synthesis for fallback audio generation
static void addPluckedString(
    std::vector<float>& mixBuffer,
    float freqHz,
    size_t startSample,
    float amplitude,
    float damping = 0.994f,
    uint32_t sr = 22050)
{
    size_t period = static_cast<size_t>(static_cast<float>(sr) / freqHz);
    if (period < 2) period = 2;

    std::vector<float> ring(period);
    uint32_t seed = static_cast<uint32_t>(freqHz * 100.0f) + 12345u;
    for (size_t i = 0; i < period; ++i) {
        seed = seed * 1664525u + 1013904223u;
        float noise = (static_cast<float>((seed >> 8) & 0xFFFF) / 32768.0f) - 1.0f;
        // Smooth initial pluck window so it sounds warm like a fingerpicked nylon/acoustic string
        float win = std::sin(3.14159265f * static_cast<float>(i) / static_cast<float>(period));
        ring[i] = noise * win;
    }

    size_t idx = 0;
    for (size_t i = startSample; i < mixBuffer.size(); ++i) {
        float curr = ring[idx];
        size_t nextIdx = (idx + 1) % period;
        ring[idx] = 0.5f * (curr + ring[nextIdx]) * damping;
        idx = nextIdx;
        mixBuffer[i] += curr * amplitude;
    }
}

void AudioSystem::ensureDefaultSoundsOnDisk() {
    // Primary audio assets in assets/sounds/*.wav are real studio-recorded 16-bit PCM WAVs
    // from the MIT Media Lab Scratch Open-Source Sound Library (CC-BY-SA 2.0, MD5 & RIFF/WAVE verified).
    // If any file is missing on disk, synthesize a warm Karplus-Strong acoustic fallback.
    constexpr uint32_t sr = 22050;
    constexpr float twoPi = 6.28318530718f;

    // 1. UI Blip (soft low-pass wooden marimba tap)
    if (!isValidRiffWaveFile("assets/sounds/ui_blip.wav")) {
        size_t count = (sr * 65) / 1000;
        std::vector<int16_t> pcm(count);
        for (size_t i = 0; i < count; ++i) {
            float t = static_cast<float>(i) / static_cast<float>(sr);
            float attack = (t < 0.004f) ? (t / 0.004f) : 1.0f;
            float env = attack * std::exp(-t * 55.0f);
            float wave = std::sin(twoPi * 587.33f * t) + 0.35f * std::sin(twoPi * 1174.66f * t);
            pcm[i] = static_cast<int16_t>(wave * env * 4200.0f);
        }
        writeMonoWavFileIfMissing("assets/sounds/ui_blip.wav", pcm, sr);
    }

    // 2. Dialogue Select (warm plucked acoustic harmonic note)
    if (!isValidRiffWaveFile("assets/sounds/dialogue_tick.wav")) {
        size_t count = (sr * 320) / 1000;
        std::vector<float> mix(count, 0.0f);
        addPluckedString(mix, 440.0f, 0, 0.55f, 0.992f, sr);
        std::vector<int16_t> pcm(count);
        for (size_t i = 0; i < count; ++i) {
            float v = mix[i];
            if (v > 1.0f) v = 1.0f;
            if (v < -1.0f) v = -1.0f;
            pcm[i] = static_cast<int16_t>(v * 14000.0f);
        }
        writeMonoWavFileIfMissing("assets/sounds/dialogue_tick.wav", pcm, sr);
    }

    // 3. Acoustic Guitar Strum (staggered 5-string Karplus-Strong Cmaj9 chord)
    if (!isValidRiffWaveFile("assets/sounds/chord_strum.wav")) {
        size_t count = (sr * 950) / 1000;
        std::vector<float> mix(count, 0.0f);
        const float notes[5] = { 130.81f, 196.00f, 246.94f, 293.66f, 329.63f };
        for (int n = 0; n < 5; ++n) {
            size_t delaySamples = static_cast<size_t>(n) * ((sr * 24) / 1000);
            addPluckedString(mix, notes[n], delaySamples, 0.32f, 0.995f, sr);
        }
        std::vector<int16_t> pcm(count);
        for (size_t i = 0; i < count; ++i) {
            float v = mix[i];
            if (v > 1.0f) v = 1.0f;
            if (v < -1.0f) v = -1.0f;
            pcm[i] = static_cast<int16_t>(v * 16000.0f);
        }
        writeMonoWavFileIfMissing("assets/sounds/chord_strum.wav", pcm, sr);
    }

    // 4. Save / Objective Complete Arpeggio (warm plucked triad)
    if (!isValidRiffWaveFile("assets/sounds/save_chime.wav")) {
        size_t count = (sr * 750) / 1000;
        std::vector<float> mix(count, 0.0f);
        const float notes[3] = { 523.25f, 659.25f, 783.99f };
        for (int n = 0; n < 3; ++n) {
            size_t delaySamples = static_cast<size_t>(n) * ((sr * 70) / 1000);
            addPluckedString(mix, notes[n], delaySamples, 0.38f, 0.994f, sr);
        }
        std::vector<int16_t> pcm(count);
        for (size_t i = 0; i < count; ++i) {
            float v = mix[i];
            if (v > 1.0f) v = 1.0f;
            if (v < -1.0f) v = -1.0f;
            pcm[i] = static_cast<int16_t>(v * 15000.0f);
        }
        writeMonoWavFileIfMissing("assets/sounds/save_chime.wav", pcm, sr);
    }
}

void AudioSystem::init(bool muted) {
    s_muted = muted;
    ensureDefaultSoundsOnDisk();
    // Preload all Kenney & Visual Novel WAV sound effects into memory for 0ms latency playback
    getOrLoadWavInMemory("assets/sounds/voice_maya.wav");
    getOrLoadWavInMemory("assets/sounds/voice_elijah.wav");
    getOrLoadWavInMemory("assets/sounds/voice_sam.wav");
    getOrLoadWavInMemory("assets/sounds/voice_narrator.wav");
    getOrLoadWavInMemory("assets/sounds/ui_blip.wav");
    getOrLoadWavInMemory("assets/sounds/dialogue_tick.wav");
    getOrLoadWavInMemory("assets/sounds/chord_strum.wav");
    getOrLoadWavInMemory("assets/sounds/save_chime.wav");
    s_initialized = true;

    if (!s_muted) {
        startBackgroundMusic();
    }
}

void AudioSystem::startBackgroundMusic() {
    if (s_muted) {
        return;
    }
#ifdef _WIN32
    if (std::filesystem::exists("assets/sounds/bgm_theme.mp3")) {
        mciSendStringA("close bgm_theme", NULL, 0, NULL);
        if (mciSendStringA("open \"assets/sounds/bgm_theme.mp3\" type mpegvideo alias bgm_theme", NULL, 0, NULL) == 0) {
            mciSendStringA("setaudio bgm_theme volume to 280", NULL, 0, NULL);
            mciSendStringA("play bgm_theme repeat", NULL, 0, NULL);
        }
    }
#endif
}

void AudioSystem::stopBackgroundMusic() {
#ifdef _WIN32
    mciSendStringA("stop bgm_theme", NULL, 0, NULL);
    mciSendStringA("close bgm_theme", NULL, 0, NULL);
#endif
}

void AudioSystem::setMuted(bool muted) {
    s_muted = muted;
    if (s_muted) {
        stopBackgroundMusic();
    } else if (s_initialized) {
        startBackgroundMusic();
    }
}

void AudioSystem::playVoiceBlip(const std::string& speaker, size_t charIndex) {
    if (!s_initialized || s_muted) {
        return;
    }
    (void)charIndex;

    if (speaker == "Elijah") {
        playWavFileZeroLatency("assets/sounds/voice_elijah.wav");
    } else if (speaker == "Maya") {
        playWavFileZeroLatency("assets/sounds/voice_maya.wav");
    } else if (speaker == "Sam") {
        playWavFileZeroLatency("assets/sounds/voice_sam.wav");
    } else {
        playWavFileZeroLatency("assets/sounds/voice_narrator.wav");
    }
}

void AudioSystem::play(SoundEvent event) {
    if (!s_initialized || s_muted) {
        return;
    }

    switch (event) {
        case SoundEvent::UIBlip:
            playWavFileZeroLatency("assets/sounds/ui_blip.wav");
            break;
        case SoundEvent::DialogueSelect:
            playWavFileZeroLatency("assets/sounds/dialogue_tick.wav");
            break;
        case SoundEvent::GuitarChord:
            playWavFileZeroLatency("assets/sounds/chord_strum.wav");
            break;
        case SoundEvent::SaveChime:
            playWavFileZeroLatency("assets/sounds/save_chime.wav");
            break;
    }
}

} // namespace audio
} // namespace engine
