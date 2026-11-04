#include "engine/ui/TextRenderer.h"

namespace engine {
namespace ui {

void TextRenderer::drawBitmapText2D(
    float x, float y,
    const std::string& text,
    void* font,
    float r, float g, float b, float a)
{
    glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT);
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_DEPTH_TEST);

    // Module 3 Subtopic 1 Rule: Call glColor* BEFORE glRasterPos2f!
    glColor4f(r, g, b, a);
    glRasterPos2f(x, y);
    glutBitmapString(font, reinterpret_cast<const unsigned char*>(text.c_str()));

    glPopAttrib();
}

void TextRenderer::drawStrokeText2D(
    float x, float y,
    const std::string& text,
    float scale,
    float lineWidth,
    void* strokeFont,
    float r, float g, float b)
{
    glPushAttrib(GL_ENABLE_BIT | GL_LINE_BIT | GL_CURRENT_BIT);
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_DEPTH_TEST);

    glLineWidth(lineWidth);
    glColor3f(r, g, b);

    glPushMatrix();
    glTranslatef(x, y, 0.0f);
    glScalef(scale, scale, 1.0f);
    glutStrokeString(strokeFont, reinterpret_cast<const unsigned char*>(text.c_str()));
    glPopMatrix();

    glPopAttrib();
}

void TextRenderer::drawPanel2D(
    float x, float y, float width, float height,
    float bgR, float bgG, float bgB, float bgA,
    float borderR, float borderG, float borderB)
{
    glPushAttrib(GL_ENABLE_BIT | GL_LINE_BIT | GL_CURRENT_BIT);
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Filled background quad
    glColor4f(bgR, bgG, bgB, bgA);
    glBegin(GL_QUADS);
    glVertex2f(x,         y);
    glVertex2f(x + width, y);
    glVertex2f(x + width, y + height);
    glVertex2f(x,         y + height);
    glEnd();

    // Outline border loop (Module 2 GL_LINE_LOOP)
    glLineWidth(1.5f);
    glColor4f(borderR, borderG, borderB, 0.95f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(x,         y);
    glVertex2f(x + width, y);
    glVertex2f(x + width, y + height);
    glVertex2f(x,         y + height);
    glEnd();

    glPopAttrib();
}

} // namespace ui
} // namespace engine
