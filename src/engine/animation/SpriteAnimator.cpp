#include "engine/animation/SpriteAnimator.h"

namespace engine {
namespace animation {

SpriteAnimator::SpriteAnimator(int columns, int rows)
    : m_columns(columns > 0 ? columns : 1)
    , m_rows(rows > 0 ? rows : 1)
    , m_state(AnimState::Idle)
    , m_currentFrame(0)
    , m_frameTimer(0.0f)
    , m_flipHorizontal(false)
{}

void SpriteAnimator::setState(AnimState state) {
    if (m_state != state) {
        m_state        = state;
        m_currentFrame = 0;
        m_frameTimer   = 0.0f;
    }
}

void SpriteAnimator::update(float dt) {
    float frameDuration = (m_state == AnimState::Walk) ? 0.14f : 0.35f;
    m_frameTimer += dt;
    while (m_frameTimer >= frameDuration) {
        m_frameTimer -= frameDuration;
        m_currentFrame = (m_currentFrame + 1) % m_columns;
    }
}

int SpriteAnimator::getCurrentRow() const {
    // Note: stbi_set_flip_vertically_on_load(true) flips image rows so:
    // top row of PNG (Idle, row 0 in image) is at v: [0.5 .. 1.0] (row index 1 from bottom)
    // bottom row of PNG (Walk, row 1 in image) is at v: [0.0 .. 0.5] (row index 0 from bottom)
    if (m_rows <= 1) {
        return 0;
    }
    return (m_state == AnimState::Idle) ? 1 : 0;
}

UVRect SpriteAnimator::getCurrentUVRect() const {
    float colWidth  = 1.0f / static_cast<float>(m_columns);
    float rowHeight = 1.0f / static_cast<float>(m_rows);

    int rowFromBottom = getCurrentRow();
    float uLeft  = static_cast<float>(m_currentFrame) * colWidth;
    float uRight = uLeft + colWidth;
    float vBot   = static_cast<float>(rowFromBottom) * rowHeight;
    float vTop   = vBot + rowHeight;

    UVRect rect;
    if (m_flipHorizontal) {
        rect.u0 = uRight;
        rect.u1 = uLeft;
    } else {
        rect.u0 = uLeft;
        rect.u1 = uRight;
    }
    rect.v0 = vBot;
    rect.v1 = vTop;
    return rect;
}

} // namespace animation
} // namespace engine
