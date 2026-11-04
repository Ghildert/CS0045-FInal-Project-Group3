#ifndef ENGINE_SCENE_SCENETRANSITION_H
#define ENGINE_SCENE_SCENETRANSITION_H

#include <functional>
#include <string>

namespace engine {
namespace scene {

class SceneTransition {
public:
    SceneTransition();

    void startTransition(
        const std::string& bannerTitle,
        std::function<void()> onMidpointCallback = nullptr,
        float totalDurationSeconds = 1.2f);

    void update(float dt);
    void render2D(int screenWidth, int screenHeight) const;

    bool isActive() const { return m_active; }
    float getAlpha() const;

private:
    bool                  m_active;
    float                 m_elapsed;
    float                 m_duration;
    bool                  m_midpointFired;
    std::string           m_bannerTitle;
    std::function<void()> m_midpointCallback;
};

} // namespace scene
} // namespace engine

#endif // ENGINE_SCENE_SCENETRANSITION_H
