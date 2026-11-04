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
#include <vector>

namespace engine {
namespace audio {

bool AudioSystem::s_initialized = false;
bool AudioSystem::s_muted       = false;

static void writeMonoWavFile(
    const std::string& filePath,
    const std::vector<int16_t>& samples,
    uint32_t sampleRate = 22050)
{
    if (std::filesystem::exists(filePath)) {
        return;
    }
    std::filesystem::create_directories(std::filesystem::path(filePath).parent_path());

    uint32_t dataBytes = static_cast<uint32_t>(samples.size() * sizeof(int16_t));
    uint32_t riffSize  = 36 + dataBytes;
    uint16_t audioFmt  = 1; // PCM
    uint16_t channels  = 1; // Mono
    uint32_t byteRate  = sampleRate * channels * sizeof(int16_t);
    uint16_t blockAlign = channels * sizeof(int16_t);
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

void AudioSystem::ensureDefaultSoundsOnDisk() {
    constexpr uint32_t sr = 22050;
    constexpr float twoPi = 6.28318530718f;

    // 1. UI Blip (warm 660Hz -> 880Hz chime, 70ms)
    {
        size_t count = (sr * 70) / 1000;
        std::vector<int16_t> pcm(count);
        for (size_t i = 0; i < count; ++i) {
            float t = static_cast<float>(i) / static_cast<float>(sr);
            float env = 1.0f - (static_cast<float>(i) / static_cast<float>(count));
            float freq = (i < count / 2) ? 659.25f : 880.0f;
            pcm[i] = static_cast<int16_t>(std::sin(twoPi * freq * t) * env * 6500.0f);
        }
        writeMonoWavFile("assets/sounds/ui_blip.wav", pcm, sr);
    }

    // 2. Dialogue Select (soft percussive wood-block tone, 55ms)
    {
        size_t count = (sr * 55) / 1000;
        std::vector<int16_t> pcm(count);
        for (size_t i = 0; i < count; ++i) {
            float t = static_cast<float>(i) / static_cast<float>(sr);
            float env = std::exp(-t * 45.0f);
            pcm[i] = static_cast<int16_t>(std::sin(twoPi * 523.25f * t) * env * 6000.0f);
        }
        writeMonoWavFile("assets/sounds/dialogue_tick.wav", pcm, sr);
    }

    // 3. Acoustic Guitar Warm Maj7 Chord Strum (240ms)
    {
        size_t count = (sr * 240) / 1000;
        std::vector<int16_t> pcm(count);
        const float notes[4] = { 261.63f, 329.63f, 392.00f, 493.88f }; // Cmaj7
        for (size_t i = 0; i < count; ++i) {
            float t = static_cast<float>(i) / static_cast<float>(sr);
            float env = std::exp(-t * 8.5f);
            float sum = 0.0f;
            for (int n = 0; n < 4; ++n) {
                sum += std::sin(twoPi * notes[n] * t) * 0.25f;
            }
            pcm[i] = static_cast<int16_t>(sum * env * 9500.0f);
        }
        writeMonoWavFile("assets/sounds/chord_strum.wav", pcm, sr);
    }

    // 4. Save / Objective Complete Arpeggio Chime (220ms)
    {
        size_t count = (sr * 220) / 1000;
        std::vector<int16_t> pcm(count);
        for (size_t i = 0; i < count; ++i) {
            float t = static_cast<float>(i) / static_cast<float>(sr);
            float env = 1.0f - (static_cast<float>(i) / static_cast<float>(count));
            float freq = (i < count / 3) ? 523.25f : ((i < (2 * count) / 3) ? 659.25f : 783.99f);
            pcm[i] = static_cast<int16_t>(std::sin(twoPi * freq * t) * env * 7200.0f);
        }
        writeMonoWavFile("assets/sounds/save_chime.wav", pcm, sr);
    }
}

void AudioSystem::init(bool muted) {
    s_muted = muted;
    ensureDefaultSoundsOnDisk();
    s_initialized = true;
}

void AudioSystem::play(SoundEvent event) {
    if (!s_initialized || s_muted) {
        return;
    }

    const char* path = "assets/sounds/ui_blip.wav";
    switch (event) {
        case SoundEvent::UIBlip:         path = "assets/sounds/ui_blip.wav";       break;
        case SoundEvent::DialogueSelect: path = "assets/sounds/dialogue_tick.wav"; break;
        case SoundEvent::GuitarChord:    path = "assets/sounds/chord_strum.wav";   break;
        case SoundEvent::SaveChime:      path = "assets/sounds/save_chime.wav";    break;
    }

#ifdef _WIN32
    PlaySoundA(path, NULL, SND_FILENAME | SND_ASYNC | SND_NODEFAULT);
#else
    (void)path;
#endif
}

} // namespace audio
} // namespace engine
