#ifndef ENGINE_CORE_LOGGER_H
#define ENGINE_CORE_LOGGER_H

#ifndef GLEW_STATIC
#define GLEW_STATIC
#endif
#include <GL/glew.h>
#include <string>

namespace engine {
namespace core {

class Logger {
public:
    static void initOpenGLDebugOutput();
    static void printContextInfo();
    static GLenum checkGLError(const char* contextLabel);
    static int getErrorCount();
    static void resetErrorCount();

    static void info(const std::string& message);
    static void warn(const std::string& message);
    static void error(const std::string& message);

private:
    static void GLAPIENTRY debugMessageCallback(
        GLenum source,
        GLenum type,
        GLuint id,
        GLenum severity,
        GLsizei length,
        const GLchar* message,
        const void* userParam);

    static int s_glErrorCount;
};

} // namespace core
} // namespace engine

#endif // ENGINE_CORE_LOGGER_H
