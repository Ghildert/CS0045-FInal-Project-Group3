#ifndef ENGINE_AUDIO_AUDIOSYSTEM_H
#define ENGINE_AUDIO_AUDIOSYSTEM_H

#include <string>

namespace engine {
namespace audio {

enum class SoundEvent {
    UIBlip = 0,
    DialogueSelect = 1,
    GuitarChord = 2,
    SaveChime = 3
};

class AudioSystem {
public:
    static void init(bool muted = false);
    static void ensureDefaultSoundsOnDisk();

    static void play(SoundEvent event);
    static void playVoiceBlip(const std::string& speaker, size_t charIndex = 0);
    static void startBackgroundMusic();
    static void stopBackgroundMusic();
    static void setMuted(bool muted);
    static bool isMuted() { return s_muted; }
    static void toggleMuted() { setMuted(!s_muted); }

private:
    static bool s_initialized;
    static bool s_muted;
};

} // namespace audio
} // namespace engine

#endif // ENGINE_AUDIO_AUDIOSYSTEM_H
