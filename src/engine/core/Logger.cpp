#include "engine/core/Logger.h"
#include <iostream>

namespace engine {
namespace core {

int Logger::s_glErrorCount = 0;

void GLAPIENTRY Logger::debugMessageCallback(
    GLenum /*source*/,
    GLenum type,
    GLuint id,
    GLenum severity,
    GLsizei /*length*/,
    const GLchar* message,
    const void* /*userParam*/)
{
    if (severity == GL_DEBUG_SEVERITY_NOTIFICATION) {
        return;
    }
    if (type == GL_DEBUG_TYPE_ERROR) {
        ++s_glErrorCount;
    }
    std::cerr << "[GL DEBUG] id=0x" << std::hex << id << std::dec
              << " type=" << (type == GL_DEBUG_TYPE_ERROR ? "ERROR" : "OTHER")
              << " severity=0x" << std::hex << severity << std::dec
              << " msg: " << message << std::endl;
}

void Logger::initOpenGLDebugOutput() {
    if (GLEW_KHR_debug || glDebugMessageCallback != nullptr) {
        glEnable(GL_DEBUG_OUTPUT);
        glDebugMessageCallback(Logger::debugMessageCallback, nullptr);
    }
}

void Logger::printContextInfo() {
    const GLubyte* version  = glGetString(GL_VERSION);
    const GLubyte* vendor   = glGetString(GL_VENDOR);
    const GLubyte* renderer = glGetString(GL_RENDERER);
    GLint maxLights = 0;
    glGetIntegerv(GL_MAX_LIGHTS, &maxLights);

    std::cout << "==========================================================" << std::endl;
    std::cout << " Echoes of November - 2.5D Narrative Adventure (CS0045)"   << std::endl;
    std::cout << "==========================================================" << std::endl;
    std::cout << "OpenGL Version : " << (version  ? reinterpret_cast<const char*>(version)  : "Unknown") << std::endl;
    std::cout << "OpenGL Vendor  : " << (vendor   ? reinterpret_cast<const char*>(vendor)   : "Unknown") << std::endl;
    std::cout << "OpenGL Renderer: " << (renderer ? reinterpret_cast<const char*>(renderer) : "Unknown") << std::endl;
    std::cout << "GL_MAX_LIGHTS  : " << maxLights << std::endl;
    std::cout << "==========================================================" << std::endl;
}

GLenum Logger::checkGLError(const char* contextLabel) {
    GLenum lastErr = GL_NO_ERROR;
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
        lastErr = err;
        ++s_glErrorCount;
        std::cerr << "[OpenGL Error 0x" << std::hex << err << std::dec
                  << "] in " << (contextLabel ? contextLabel : "unknown") << std::endl;
    }
    return lastErr;
}

int Logger::getErrorCount() {
    return s_glErrorCount;
}

void Logger::resetErrorCount() {
    s_glErrorCount = 0;
}

void Logger::info(const std::string& message) {
    std::cout << "[INFO] " << message << std::endl;
}

void Logger::warn(const std::string& message) {
    std::cout << "[WARN] " << message << std::endl;
}

void Logger::error(const std::string& message) {
    std::cerr << "[ERROR] " << message << std::endl;
}

} // namespace core
} // namespace engine
