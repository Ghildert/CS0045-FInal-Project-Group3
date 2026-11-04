#include "engine/assets/Texture2D.h"
#include "engine/core/Logger.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#define STB_IMAGE_IMPLEMENTATION
#include "engine/assets/stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "engine/assets/stb_image_write.h"
#pragma GCC diagnostic pop

#include <filesystem>
#include <vector>

namespace engine {
namespace assets {

Texture2D::Texture2D()
    : m_textureId(0)
    , m_width(0)
    , m_height(0)
    , m_channels(0)
{}

Texture2D::~Texture2D() {
    destroy();
}

void Texture2D::destroy() {
    if (m_textureId != 0) {
        glDeleteTextures(1, &m_textureId);
        m_textureId = 0;
    }
}

bool Texture2D::loadFromFile(
    const std::string& filePath,
    bool nearestPixelArtFilter,
    bool repeatWrap,
    bool flipVertically)
{
    destroy();
    m_filePath = filePath;

    stbi_set_flip_vertically_on_load(flipVertically ? 1 : 0);
    unsigned char* data = stbi_load(filePath.c_str(), &m_width, &m_height, &m_channels, 4);
    if (!data) {
        core::Logger::error("stbi_load failed for '" + filePath + "': " +
                            (stbi_failure_reason() ? stbi_failure_reason() : "unknown reason"));
        return false;
    }

    glEnable(GL_TEXTURE_2D);
    glGenTextures(1, &m_textureId);
    glBindTexture(GL_TEXTURE_2D, m_textureId);

    // Module 8 Subtopic 2: Texture environment mode GL_MODULATE so lighting multiplies texture color
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    GLint wrapMode = repeatWrap ? GL_REPEAT : GL_CLAMP_TO_EDGE;
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode);

    GLint filterMode = nearestPixelArtFilter ? GL_NEAREST : GL_LINEAR;
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filterMode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filterMode);

    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGBA,
        m_width,
        m_height,
        0,
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        data);

    // Module 8 Subtopic 2: Generate mipmaps via GLEW
    if (glGenerateMipmap != nullptr) {
        glGenerateMipmap(GL_TEXTURE_2D);
        // Re-assert filter mode after mipmap generation so pixel-art keeps crisp GL_NEAREST
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filterMode);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filterMode);
    }

    stbi_image_free(data);
    glBindTexture(GL_TEXTURE_2D, 0);

    core::Logger::info("Loaded texture: " + filePath + " (" +
                       std::to_string(m_width) + "x" + std::to_string(m_height) +
                       ", id=" + std::to_string(m_textureId) + ")");
    return true;
}

void Texture2D::bind() const {
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, m_textureId);
}

void Texture2D::unbind() {
    glBindTexture(GL_TEXTURE_2D, 0);
}

static void writePNGIfMissing(const std::string& path, int w, int h, const std::vector<unsigned char>& rgba) {
    if (std::filesystem::exists(path)) {
        return;
    }
    std::filesystem::create_directories(std::filesystem::path(path).parent_path());
    stbi_write_png(path.c_str(), w, h, 4, rgba.data(), w * 4);
}

void Texture2D::ensureDefaultTexturesOnDisk() {
    const int W = 32;
    const int H = 32;

    // 1. Warm Classroom Wood Floor (pixel-art planks)
    {
        std::vector<unsigned char> img(W * H * 4);
        for (int y = 0; y < H; ++y) {
            for (int x = 0; x < W; ++x) {
                int idx = (y * W + x) * 4;
                bool plankSeamY = (y % 8 == 0);
                int stagger = ((y / 8) % 2) * 16;
                bool plankSeamX = ((x + stagger) % 32 == 0);
                int grain = ((x * 3 + y * 7) % 11) - 5;

                if (plankSeamY || plankSeamX) {
                    img[idx + 0] = 95;
                    img[idx + 1] = 62;
                    img[idx + 2] = 38;
                } else {
                    img[idx + 0] = static_cast<unsigned char>(158 + grain);
                    img[idx + 1] = static_cast<unsigned char>(108 + grain);
                    img[idx + 2] = static_cast<unsigned char>(68 + grain / 2);
                }
                img[idx + 3] = 255;
            }
        }
        writePNGIfMissing("assets/textures/wood_floor.png", W, H, img);
    }

    // 2. Classroom Plaster & Wainscoting Wall
    {
        std::vector<unsigned char> img(W * H * 4);
        for (int y = 0; y < H; ++y) {
            for (int x = 0; x < W; ++x) {
                int idx = (y * W + x) * 4;
                int dither = ((x + y) % 2) * 4;
                if (y < 10) {
                    // Warm cream upper plaster
                    img[idx + 0] = static_cast<unsigned char>(232 - dither);
                    img[idx + 1] = static_cast<unsigned char>(220 - dither);
                    img[idx + 2] = static_cast<unsigned char>(195 - dither);
                } else if (y == 10 || y == 11) {
                    // Wooden trim rail
                    img[idx + 0] = 115;
                    img[idx + 1] = 76;
                    img[idx + 2] = 48;
                } else {
                    // Lower teal-sage wainscoting panel
                    bool groove = (x % 8 == 0);
                    img[idx + 0] = groove ? 84  : static_cast<unsigned char>(112 - dither);
                    img[idx + 1] = groove ? 114 : static_cast<unsigned char>(144 - dither);
                    img[idx + 2] = groove ? 108 : static_cast<unsigned char>(136 - dither);
                }
                img[idx + 3] = 255;
            }
        }
        writePNGIfMissing("assets/textures/classroom_wall.png", W, H, img);
    }

    // 3. Desk Wood Grain Texture
    {
        std::vector<unsigned char> img(W * H * 4);
        for (int y = 0; y < H; ++y) {
            for (int x = 0; x < W; ++x) {
                int idx = (y * W + x) * 4;
                bool border = (x == 0 || x == W - 1 || y == 0 || y == H - 1);
                int grain = ((x + y * 2) % 7) - 3;
                img[idx + 0] = border ? 110 : static_cast<unsigned char>(182 + grain);
                img[idx + 1] = border ? 74  : static_cast<unsigned char>(128 + grain);
                img[idx + 2] = border ? 45  : static_cast<unsigned char>(78  + grain);
                img[idx + 3] = 255;
            }
        }
        writePNGIfMissing("assets/textures/desk_wood.png", W, H, img);
    }

    // 4. Chalkboard Texture (dark slate green with wooden frame and chalk marks)
    {
        std::vector<unsigned char> img(W * H * 4);
        for (int y = 0; y < H; ++y) {
            for (int x = 0; x < W; ++x) {
                int idx = (y * W + x) * 4;
                bool frame = (x < 2 || x >= W - 2 || y < 2 || y >= H - 2);
                bool chalkLine = (y == 10 && x >= 6 && x <= 24) || (y == 16 && x >= 6 && x <= 18);
                if (frame) {
                    img[idx + 0] = 128;
                    img[idx + 1] = 84;
                    img[idx + 2] = 48;
                } else if (chalkLine) {
                    img[idx + 0] = 225;
                    img[idx + 1] = 235;
                    img[idx + 2] = 225;
                } else {
                    int dust = ((x * 5 + y * 3) % 9);
                    img[idx + 0] = static_cast<unsigned char>(34 + dust);
                    img[idx + 1] = static_cast<unsigned char>(68 + dust);
                    img[idx + 2] = static_cast<unsigned char>(52 + dust);
                }
                img[idx + 3] = 255;
            }
        }
        writePNGIfMissing("assets/textures/chalkboard.png", W, H, img);
    }

    // 5. Sunset Window Glass Texture (semi-transparent warm amber sunset sky)
    {
        std::vector<unsigned char> img(W * H * 4);
        for (int y = 0; y < H; ++y) {
            for (int x = 0; x < W; ++x) {
                int idx = (y * W + x) * 4;
                bool mullion = (x < 2 || x >= W - 2 || y < 2 || y >= H - 2 || x == W / 2 || y == H / 2);
                if (mullion) {
                    img[idx + 0] = 90;
                    img[idx + 1] = 64;
                    img[idx + 2] = 46;
                    img[idx + 3] = 255;
                } else {
                    float t = static_cast<float>(y) / static_cast<float>(H);
                    img[idx + 0] = static_cast<unsigned char>(250 - t * 30.0f);
                    img[idx + 1] = static_cast<unsigned char>(175 - t * 50.0f);
                    img[idx + 2] = static_cast<unsigned char>(110 + t * 60.0f);
                    img[idx + 3] = 215;
                }
            }
        }
        writePNGIfMissing("assets/textures/window_glass.png", W, H, img);
    }

    // 6. UV Checker Debug Texture (for texture alignment testing)
    {
        std::vector<unsigned char> img(W * H * 4);
        for (int y = 0; y < H; ++y) {
            for (int x = 0; x < W; ++x) {
                int idx = (y * W + x) * 4;
                bool even = (((x / 4) + (y / 4)) % 2 == 0);
                img[idx + 0] = even ? 240 : 48;
                img[idx + 1] = even ? 190 : 62;
                img[idx + 2] = even ? 95  : 92;
                img[idx + 3] = 255;
            }
        }
        writePNGIfMissing("assets/textures/checker_debug.png", W, H, img);
    }
}

} // namespace assets
} // namespace engine
