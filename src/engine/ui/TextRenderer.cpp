#include "engine/ui/TextRenderer.h"
#include <cmath>
#include <sstream>
#include <vector>

namespace engine {
namespace ui {

static constexpr float kPi = 3.14159265358979323846f;

std::string TextRenderer::sanitizeASCII(const std::string& text) {
    std::string out;
    out.reserve(text.size());
    for (size_t i = 0; i < text.size(); ++i) {
        unsigned char c = static_cast<unsigned char>(text[i]);
        // Convert UTF-8 em-dash (E2 80 94) and en-dash (E2 80 93) to clean ASCII " - " or "-"
        if (c == 0xE2 && i + 2 < text.size()) {
            unsigned char c1 = static_cast<unsigned char>(text[i + 1]);
            unsigned char c2 = static_cast<unsigned char>(text[i + 2]);
            if (c1 == 0x80 && (c2 == 0x94 || c2 == 0x93)) {
                out += " - ";
                i += 2;
                continue;
            }
            if (c1 == 0x80 && (c2 == 0x98 || c2 == 0x99)) {
                out += '\'';
                i += 2;
                continue;
            }
            if (c1 == 0x80 && (c2 == 0x9C || c2 == 0x9D)) {
                out += '"';
                i += 2;
                continue;
            }
        }
        if (c >= 32 && c < 127) {
            out += static_cast<char>(c);
        } else if (c == '\n' || c == '\t') {
            out += ' ';
        }
    }
    return out;
}

int TextRenderer::measureBitmapTextWidth(const std::string& text, void* font) {
    std::string clean = sanitizeASCII(text);
    return glutBitmapLength(font, reinterpret_cast<const unsigned char*>(clean.c_str()));
}

float TextRenderer::measureStrokeTextWidth(const std::string& text, float scale, void* strokeFont) {
    std::string clean = sanitizeASCII(text);
    int rawLen = glutStrokeLength(strokeFont, reinterpret_cast<const unsigned char*>(clean.c_str()));
    return static_cast<float>(rawLen) * scale;
}

void TextRenderer::drawBitmapText2D(
    float x, float y,
    const std::string& text,
    void* font,
    float r, float g, float b, float a)
{
    std::string clean = sanitizeASCII(text);
    const float ix = std::round(x);
    const float iy = std::round(y);

    glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT);
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Only draw a subtle drop shadow behind light text on dark backgrounds;
    // dark text on light surfaces (such as the white smartphone screen) renders
    // in a single pixel-aligned pass so glyphs stay razor-sharp without ghosting.
    if ((r + g + b) > 1.55f) {
        glColor4f(0.0f, 0.02f, 0.06f, a * 0.48f);
        glRasterPos2f(ix + 1.0f, iy - 1.0f);
        glutBitmapString(font, reinterpret_cast<const unsigned char*>(clean.c_str()));
    }

    // Module 3 Subtopic 1 Rule: Call glColor* BEFORE glRasterPos2f!
    glColor4f(r, g, b, a);
    glRasterPos2f(ix, iy);
    glutBitmapString(font, reinterpret_cast<const unsigned char*>(clean.c_str()));

    glPopAttrib();
}

void TextRenderer::drawBitmapTextCentered2D(
    float centerX, float y,
    const std::string& text,
    void* font,
    float r, float g, float b, float a)
{
    int width = measureBitmapTextWidth(text, font);
    float x = centerX - static_cast<float>(width) * 0.5f;
    drawBitmapText2D(x, y, text, font, r, g, b, a);
}

WrappedTextLayout TextRenderer::computeWrappedLayout(
    const std::string& text,
    float maxWidth,
    float lineSpacing,
    void* font)
{
    WrappedTextLayout layout;
    std::string clean = sanitizeASCII(text);
    std::istringstream words(clean);
    std::string word;
    std::string currentLine;

    auto pushLine = [&](const std::string& lineStr) {
        if (lineStr.empty()) return;
        int w = glutBitmapLength(font, reinterpret_cast<const unsigned char*>(lineStr.c_str()));
        if (static_cast<float>(w) > layout.maxLineWidth) {
            layout.maxLineWidth = static_cast<float>(w);
        }
        layout.lines.push_back(lineStr);
    };

    while (words >> word) {
        std::string candidate = currentLine.empty() ? word : (currentLine + " " + word);
        int w = glutBitmapLength(font, reinterpret_cast<const unsigned char*>(candidate.c_str()));
        if (!currentLine.empty() && static_cast<float>(w) > maxWidth) {
            pushLine(currentLine);
            currentLine = word;
        } else {
            currentLine = candidate;
        }
    }
    if (!currentLine.empty()) {
        pushLine(currentLine);
    }
    if (layout.lines.empty()) {
        layout.lines.push_back("");
    }
    layout.totalHeight = static_cast<float>(layout.lines.size()) * lineSpacing;
    return layout;
}

int TextRenderer::drawWrappedBitmapText2D(
    float x, float topLineY,
    float maxWidth, float lineSpacing,
    const std::string& text,
    void* font,
    float r, float g, float b, float a)
{
    WrappedTextLayout layout = computeWrappedLayout(text, maxWidth, lineSpacing, font);
    for (size_t i = 0; i < layout.lines.size(); ++i) {
        drawBitmapText2D(
            x, topLineY - static_cast<float>(i) * lineSpacing,
            layout.lines[i], font, r, g, b, a);
    }
    return static_cast<int>(layout.lines.size());
}

void TextRenderer::drawStrokeText2D(
    float x, float y,
    const std::string& text,
    float scale,
    float lineWidth,
    void* strokeFont,
    float r, float g, float b)
{
    std::string clean = sanitizeASCII(text);

    glPushAttrib(GL_ENABLE_BIT | GL_LINE_BIT | GL_CURRENT_BIT);
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_LINE_SMOOTH);

    glLineWidth(lineWidth);

    // Subtle dark drop shadow behind vector stroke text
    glColor4f(0.0f, 0.02f, 0.06f, 0.65f);
    glPushMatrix();
    glTranslatef(x + 1.5f, y - 1.5f, 0.0f);
    glScalef(scale, scale, 1.0f);
    glutStrokeString(strokeFont, reinterpret_cast<const unsigned char*>(clean.c_str()));
    glPopMatrix();

    glColor3f(r, g, b);
    glPushMatrix();
    glTranslatef(x, y, 0.0f);
    glScalef(scale, scale, 1.0f);
    glutStrokeString(strokeFont, reinterpret_cast<const unsigned char*>(clean.c_str()));
    glPopMatrix();

    glPopAttrib();
}

void TextRenderer::drawStrokeTextCentered2D(
    float centerX, float y,
    const std::string& text,
    float scale,
    float lineWidth,
    void* strokeFont,
    float r, float g, float b)
{
    float width = measureStrokeTextWidth(text, scale, strokeFont);
    float x = centerX - width * 0.5f;
    drawStrokeText2D(x, y, text, scale, lineWidth, strokeFont, r, g, b);
}

static void buildRoundedRectVertices(
    float x, float y, float w, float h, float r,
    std::vector<std::pair<float, float>>& outVerts)
{
    outVerts.clear();
    float maxR = (w < h ? w : h) * 0.5f;
    if (r > maxR) r = maxR;
    if (r < 0.0f) r = 0.0f;

    if (r < 0.5f) {
        outVerts.push_back({ x,     y     });
        outVerts.push_back({ x + w, y     });
        outVerts.push_back({ x + w, y + h });
        outVerts.push_back({ x,     y + h });
        return;
    }

    constexpr int kSegs = 12;
    outVerts.reserve(kSegs * 4 + 4);

    // 4 corner arc centers: Bottom-Left, Bottom-Right, Top-Right, Top-Left
    const float cx[4] = { x + r,     x + w - r, x + w - r, x + r     };
    const float cy[4] = { y + r,     y + r,     y + h - r, y + h - r };
    const float a0[4] = { kPi,       1.5f * kPi, 0.0f,     0.5f * kPi };

    for (int corner = 0; corner < 4; ++corner) {
        for (int s = 0; s <= kSegs; ++s) {
            float ang = a0[corner] + (static_cast<float>(s) / static_cast<float>(kSegs)) * (0.5f * kPi);
            outVerts.push_back({
                cx[corner] + std::cos(ang) * r,
                cy[corner] + std::sin(ang) * r
            });
        }
    }
}

void TextRenderer::drawRoundedRect2D(
    float x, float y, float width, float height,
    float cornerRadius,
    float bgR, float bgG, float bgB, float bgA,
    float borderR, float borderG, float borderB, float borderA,
    float borderWidth,
    bool drawShadow)
{
    glPushAttrib(GL_ENABLE_BIT | GL_LINE_BIT | GL_CURRENT_BIT);
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

    std::vector<std::pair<float, float>> verts;

    // 1. HTML/CSS soft multi-layer box-shadow (0 6px 18px rgba(0, 0, 0, 0.35))
    if (drawShadow && bgA > 0.05f) {
        const float expand[3]  = { 1.5f, 3.5f, 6.5f };
        const float offsetY[3] = { -2.0f, -4.0f, -6.0f };
        const float alphas[3]  = { 0.22f * bgA, 0.12f * bgA, 0.05f * bgA };

        for (int layer = 2; layer >= 0; --layer) {
            float ex = expand[layer];
            float oy = offsetY[layer];
            buildRoundedRectVertices(
                x - ex, y + oy - ex, width + ex * 2.0f, height + ex * 2.0f,
                cornerRadius + ex, verts);

            glColor4f(0.01f, 0.02f, 0.05f, alphas[layer]);
            glBegin(GL_TRIANGLE_FAN);
            glVertex2f(x + width * 0.5f, y + oy + height * 0.5f);
            for (const auto& pt : verts) {
                glVertex2f(pt.first, pt.second);
            }
            if (!verts.empty()) {
                glVertex2f(verts[0].first, verts[0].second);
            }
            glEnd();
        }
    }

    // 2. Main rounded surface fill with clean CSS-like subtle vertical gradient
    buildRoundedRectVertices(x, y, width, height, cornerRadius, verts);

    float topR = (bgR * 1.24f < 1.0f) ? (bgR * 1.24f) : 1.0f;
    float topG = (bgG * 1.24f < 1.0f) ? (bgG * 1.24f) : 1.0f;
    float topB = (bgB * 1.24f < 1.0f) ? (bgB * 1.24f) : 1.0f;

    glBegin(GL_TRIANGLE_FAN);
    glColor4f((bgR + topR) * 0.5f, (bgG + topG) * 0.5f, (bgB + topB) * 0.5f, bgA);
    glVertex2f(x + width * 0.5f, y + height * 0.5f);
    for (const auto& pt : verts) {
        float t = (height > 1.0f) ? ((pt.second - y) / height) : 0.5f;
        if (t < 0.0f) t = 0.0f;
        if (t > 1.0f) t = 1.0f;
        float cr = bgR + (topR - bgR) * t;
        float cg = bgG + (topG - bgG) * t;
        float cb = bgB + (topB - bgB) * t;
        glColor4f(cr, cg, cb, bgA);
        glVertex2f(pt.first, pt.second);
    }
    if (!verts.empty()) {
        float t = (height > 1.0f) ? ((verts[0].second - y) / height) : 0.0f;
        glColor4f(bgR + (topR - bgR) * t, bgG + (topG - bgG) * t, bgB + (topB - bgB) * t, bgA);
        glVertex2f(verts[0].first, verts[0].second);
    }
    glEnd();

    // 3. Top inner specular glass highlight line (mimics CSS inset 0 1px 0 rgba(255,255,255,0.12))
    if (width > cornerRadius * 2.5f && height > 16.0f) {
        glLineWidth(1.0f);
        glColor4f(1.0f, 1.0f, 1.0f, 0.12f * bgA);
        glBegin(GL_LINES);
        glVertex2f(x + cornerRadius,         y + height - 1.2f);
        glVertex2f(x + width - cornerRadius, y + height - 1.2f);
        glEnd();
    }

    // 4. Sleek anti-aliased CSS-style border outline
    if (borderA > 0.01f && borderWidth > 0.1f) {
        glLineWidth(borderWidth);
        glColor4f(borderR, borderG, borderB, borderA);
        glBegin(GL_LINE_LOOP);
        for (const auto& pt : verts) {
            glVertex2f(pt.first, pt.second);
        }
        glEnd();
    }

    glPopAttrib();
}

void TextRenderer::drawPanel2D(
    float x, float y, float width, float height,
    float bgR, float bgG, float bgB, float bgA,
    float borderR, float borderG, float borderB,
    float cornerRadius)
{
    drawRoundedRect2D(
        x, y, width, height,
        cornerRadius,
        bgR, bgG, bgB, bgA,
        borderR, borderG, borderB, 0.55f,
        1.4f,
        true);
}

void TextRenderer::drawPillBadge2D(
    float x, float y, float width, float height,
    const std::string& label,
    float bgR, float bgG, float bgB, float bgA,
    float borderR, float borderG, float borderB,
    float textR, float textG, float textB,
    void* font)
{
    float radius = height * 0.5f;
    drawRoundedRect2D(
        x, y, width, height,
        radius,
        bgR, bgG, bgB, bgA,
        borderR, borderG, borderB, 0.65f,
        1.2f,
        false);

    float textOffsetY = (font == GLUT_BITMAP_HELVETICA_18)
        ? (y + (height - 14.0f) * 0.5f)
        : (y + (height - 9.0f) * 0.5f);
    drawBitmapTextCentered2D(
        x + width * 0.5f, textOffsetY,
        label, font,
        textR, textG, textB, 1.0f);
}

void TextRenderer::drawAccentBar2D(
    float x, float y, float width, float height,
    float r, float g, float b, float a)
{
    if (width >= 4.0f && height >= 4.0f && (width < 20.0f || height < 20.0f)) {
        float rad = (width < height ? width : height) * 0.5f;
        if (rad > 4.0f) rad = 4.0f;
        drawRoundedRect2D(
            x, y, width, height, rad,
            r, g, b, a,
            r, g, b, 0.0f, 0.0f, false);
        return;
    }

    glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT);
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glColor4f(r, g, b, a);
    glBegin(GL_QUADS);
    glVertex2f(x,         y);
    glVertex2f(x + width, y);
    glVertex2f(x + width, y + height);
    glVertex2f(x,         y + height);
    glEnd();

    glPopAttrib();
}

void TextRenderer::drawTexturedQuad2D(
    const assets::Texture2D& texture,
    float x, float y, float width, float height)
{
    if (!texture.isValid()) {
        return;
    }
    glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT);
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    texture.bind();
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); glVertex2f(x,         y);
    glTexCoord2f(1.0f, 0.0f); glVertex2f(x + width, y);
    glTexCoord2f(1.0f, 1.0f); glVertex2f(x + width, y + height);
    glTexCoord2f(0.0f, 1.0f); glVertex2f(x,         y + height);
    glEnd();
    assets::Texture2D::unbind();

    glPopAttrib();
}

} // namespace ui
} // namespace engine
