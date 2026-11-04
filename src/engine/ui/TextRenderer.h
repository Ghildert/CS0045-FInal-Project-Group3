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

namespace engine {
namespace ui {

/**
 * Module 3 Subtopic 1: Text Rendering using FreeGLUT Extensions
 * - Uses glColor3f/glColor4f BEFORE glRasterPos2f (required by OpenGL raster pipeline)
 * - Uses glutBitmapString() for crisp HUD/dialogue text
 * - Uses glutStrokeString() with glTranslatef/glScalef for scalable vector titles
 */
class TextRenderer {
public:
    static void drawBitmapText2D(
        float x, float y,
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

    static void drawPanel2D(
        float x, float y, float width, float height,
        float bgR = 0.06f, float bgG = 0.08f, float bgB = 0.12f, float bgA = 0.82f,
        float borderR = 0.85f, float borderG = 0.68f, float borderB = 0.38f);

    static void drawTexturedQuad2D(
        const assets::Texture2D& texture,
        float x, float y, float width, float height);
};

} // namespace ui
} // namespace engine

#endif // ENGINE_UI_TEXTRENDERER_H
