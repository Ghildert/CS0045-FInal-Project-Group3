#ifndef ENGINE_ANIMATION_SPRITEANIMATOR_H
#define ENGINE_ANIMATION_SPRITEANIMATOR_H

namespace engine {
namespace animation {

enum class AnimState {
    Idle = 0,
    Walk = 1
};

struct UVRect {
    float u0;
    float v0;
    float u1;
    float v1;
};

class SpriteAnimator {
public:
    SpriteAnimator(int columns = 4, int rows = 2);

    void setState(AnimState state);
    AnimState getState() const { return m_state; }

    void setFlipHorizontal(bool flip) { m_flipHorizontal = flip; }
    bool isFlippedHorizontal() const { return m_flipHorizontal; }

    void update(float dt);
    UVRect getCurrentUVRect() const;

    int getCurrentFrame() const { return m_currentFrame; }
    int getCurrentRow() const;

private:
    int       m_columns;
    int       m_rows;
    AnimState m_state;
    int       m_currentFrame;
    float     m_frameTimer;
    bool      m_flipHorizontal;
};

} // namespace animation
} // namespace engine

#endif // ENGINE_ANIMATION_SPRITEANIMATOR_H
