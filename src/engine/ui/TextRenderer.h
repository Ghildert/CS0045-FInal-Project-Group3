#ifndef ENGINE_UI_TEXTRENDERER_H
#define ENGINE_UI_TEXTRENDERER_H

#ifndef GLEW_STATIC
#define GLEW_STATIC
#endif
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <GL/freeglut_ext.h>
#include "engine/assets/Texture2D.h"
#include <string>
#include <vector>

namespace engine {
namespace ui {

struct WrappedTextLayout {
    std::vector<std::string> lines;
    float maxLineWidth = 0.0f;
    float totalHeight  = 0.0f;
};

/**
 * Module 3 Subtopic 1: Text Rendering using FreeGLUT Extensions
 * - Uses glColor3f/glColor4f BEFORE glRasterPos2f (required by OpenGL raster pipeline)
 * - Uses glutBitmapString() for crisp HUD/dialogue text
 * - Uses glutStrokeString() with glTranslatef/glScalef for scalable vector titles
 */
class TextRenderer {
public:
    static std::string sanitizeASCII(const std::string& text);

    static int measureBitmapTextWidth(
        const std::string& text,
        void* font = GLUT_BITMAP_HELVETICA_18);

    static float measureStrokeTextWidth(
        const std::string& text,
        float scale = 0.18f,
        void* strokeFont = GLUT_STROKE_ROMAN);

    static void drawBitmapText2D(
        float x, float y,
        const std::string& text,
        void* font = GLUT_BITMAP_HELVETICA_18,
        float r = 1.0f, float g = 1.0f, float b = 1.0f, float a = 1.0f);

    static void drawBitmapTextCentered2D(
        float centerX, float y,
        const std::string& text,
        void* font = GLUT_BITMAP_HELVETICA_18,
        float r = 1.0f, float g = 1.0f, float b = 1.0f, float a = 1.0f);

    static WrappedTextLayout computeWrappedLayout(
        const std::string& text,
        float maxWidth,
        float lineSpacing,
        void* font = GLUT_BITMAP_HELVETICA_18);

    static int drawWrappedBitmapText2D(
        float x, float topLineY,
        float maxWidth, float lineSpacing,
        const std::string& text,
        void* font = GLUT_BITMAP_HELVETICA_18,
        float r = 1.0f, float g = 1.0f, float b = 1.0f, float a = 1.0f);

    static void drawStrokeText2D(
        float x, float y,
        const std::string& text,
        float scale = 0.18f,
        float lineWidth = 2.0f,
        void* strokeFont = GLUT_STROKE_ROMAN,
        float r = 1.0f, float g = 0.88f, float b = 0.55f);

    static void drawStrokeTextCentered2D(
        float centerX, float y,
        const std::string& text,
        float scale = 0.18f,
        float lineWidth = 2.0f,
        void* strokeFont = GLUT_STROKE_ROMAN,
        float r = 1.0f, float g = 0.88f, float b = 0.55f);

    static void drawPanel2D(
        float x, float y, float width, float height,
        float bgR = 0.06f, float bgG = 0.09f, float bgB = 0.16f, float bgA = 0.92f,
        float borderR = 0.58f, float borderG = 0.64f, float borderB = 0.72f,
        float cornerRadius = 10.0f);

    static void drawRoundedRect2D(
        float x, float y, float width, float height,
        float cornerRadius,
        float bgR, float bgG, float bgB, float bgA,
        float borderR, float borderG, float borderB, float borderA = 0.45f,
        float borderWidth = 1.2f,
        bool drawShadow = true);

    static void drawPillBadge2D(
        float x, float y, float width, float height,
        const std::string& label,
        float bgR, float bgG, float bgB, float bgA,
        float borderR, float borderG, float borderB,
        float textR, float textG, float textB,
        void* font = GLUT_BITMAP_HELVETICA_12);

    static void drawAccentBar2D(
        float x, float y, float width, float height,
        float r, float g, float b, float a = 1.0f);

    static void drawTexturedQuad2D(
        const assets::Texture2D& texture,
        float x, float y, float width, float height);
};

} // namespace ui
} // namespace engine

#endif // ENGINE_UI_TEXTRENDERER_H
