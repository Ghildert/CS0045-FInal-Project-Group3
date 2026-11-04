#ifndef ENGINE_ASSETS_TEXTURE2D_H
#define ENGINE_ASSETS_TEXTURE2D_H

#ifndef GLEW_STATIC
#define GLEW_STATIC
#endif
#include <GL/glew.h>
#include <string>

namespace engine {
namespace assets {

class Texture2D {
public:
    Texture2D();
    ~Texture2D();

    /**
     * Module 8 Subtopic 2: Texture Loading & Binding
     * Uses stbi_load(), glGenTextures(), glBindTexture(GL_TEXTURE_2D),
     * glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE),
     * glTexParameteri(), glTexImage2D(), glGenerateMipmap(GL_TEXTURE_2D),
     * and stbi_image_free().
     */
    bool loadFromFile(
        const std::string& filePath,
        bool nearestPixelArtFilter = true,
        bool repeatWrap = true,
        bool flipVertically = true);

    void bind() const;
    static void unbind();
    void destroy();

    GLuint getId() const { return m_textureId; }
    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }
    int getChannels() const { return m_channels; }
    bool isValid() const { return m_textureId != 0; }
    const std::string& getPath() const { return m_filePath; }

    // Ensures original pixel-art PNG textures exist in assets/textures/
    static void ensureDefaultTexturesOnDisk();

private:
    GLuint      m_textureId;
    int         m_width;
    int         m_height;
    int         m_channels;
    std::string m_filePath;
};

} // namespace assets
} // namespace engine

#endif // ENGINE_ASSETS_TEXTURE2D_H
