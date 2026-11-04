#include "debug/OpenGLDemoMode.h"
#include "engine/core/Math4x4.h"
#include "engine/core/Logger.h"
#include "engine/input/InputManager.h"
#include "engine/renderer/PrimitiveRenderer.h"
#include "engine/ui/TextRenderer.h"
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <cmath>
#include <sstream>
#include <iomanip>

namespace debug {

OpenGLDemoMode::OpenGLDemoMode()
    : m_active(false)
    , m_currentModule(1)
    , m_visitedMask(0)
    , m_timeSeconds(0.0f)
    , m_wireframeMode(false)
    , m_useCheckerTex(false)
    , m_materialPresetIdx(0)
{}

OpenGLDemoMode::~OpenGLDemoMode() {
    destroy();
}

void OpenGLDemoMode::init() {
    engine::assets::Texture2D::ensureDefaultTexturesOnDisk();
    m_texWood.loadFromFile("assets/textures/desk_wood.png", true, true);
    m_texChecker.loadFromFile("assets/textures/checker_debug.png", true, true);
    m_texSpriteSheet.loadFromFile("assets/sprites/player_elijah_sheet.png", true, false);

    m_meshBoxStatic.buildTexturedBox(1.2f, 1.2f, 1.2f, 1.0f, 1.0f, GL_STATIC_DRAW);
    m_meshPyramidDynamic.buildPyramid(1.1f, 1.35f, GL_DYNAMIC_DRAW);

    m_demoLighting.initClassroomLighting();
    selectModule(1);
}

void OpenGLDemoMode::destroy() {
    m_meshBoxStatic.destroy();
    m_meshPyramidDynamic.destroy();
    m_texWood.destroy();
    m_texChecker.destroy();
    m_texSpriteSheet.destroy();
}

void OpenGLDemoMode::toggleActive() {
    m_active = !m_active;
    if (m_active) {
        m_visitedMask |= (1 << (m_currentModule - 1));
    }
}

void OpenGLDemoMode::selectModule(int moduleNumber1To8) {
    if (moduleNumber1To8 < 1) moduleNumber1To8 = 1;
    if (moduleNumber1To8 > 8) moduleNumber1To8 = 8;
    m_currentModule = moduleNumber1To8;
    m_visitedMask |= (1 << (m_currentModule - 1));
}

void OpenGLDemoMode::nextModule() {
    int next = (m_currentModule % 8) + 1;
    selectModule(next);
}

void OpenGLDemoMode::prevModule() {
    int prev = ((m_currentModule + 6) % 8) + 1;
    selectModule(prev);
}

void OpenGLDemoMode::update(float dt) {
    if (!m_active) {
        return;
    }
    m_timeSeconds += dt;

    // For Module 5 & 6 stations, animate the apex vertex of the dynamic VBO pyramid via glMapBuffer
    if (m_currentModule == 5 || m_currentModule == 6) {
        float dy = std::cos(m_timeSeconds * 3.5f) * 0.004f;
        engine::core::Math4x4::translateVertexInMappedVBO(
            m_meshPyramidDynamic.getVBOId(), 0.0f, dy, 0.0f, 0);
    }
}

void OpenGLDemoMode::renderModule1_3D() {
    // Module 1: Immediate-mode 2D/3D Polygon (glBegin(GL_POLYGON) / glVertex2f / glEnd)
    glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT);
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);

    glPushMatrix();
    glTranslatef(0.0f, 1.35f, 0.0f);
    glRotatef(m_timeSeconds * 25.0f, 0.0f, 1.0f, 0.0f);

    glBegin(GL_POLYGON);
    glColor3f(0.95f, 0.35f, 0.35f); glVertex2f(-0.9f, -0.7f);
    glColor3f(0.98f, 0.82f, 0.32f); glVertex2f( 0.9f, -0.7f);
    glColor3f(0.35f, 0.88f, 0.65f); glVertex2f( 1.2f,  0.2f);
    glColor3f(0.38f, 0.68f, 0.98f); glVertex2f( 0.0f,  1.0f);
    glColor3f(0.82f, 0.45f, 0.95f); glVertex2f(-1.2f,  0.2f);
    glEnd();

    glPopMatrix();
    glPopAttrib();
}

void OpenGLDemoMode::renderModule2_3D() {
    // Module 2: All 10 Primitives + Line Stippling (0x00FF, 0x0C0F, 0xAAAA) + Rain & Ripples
    engine::renderer::PrimitiveRenderer::drawModule2PrimitiveShowcase(-0.2f, 1.45f, 0.0f);
    engine::renderer::PrimitiveRenderer::drawRainAndRipples(m_timeSeconds, 55);

    engine::renderer::PrimitiveRenderer::drawStippledWireBox(
        -1.6f, 0.75f, 0.8f, 1.1f, 0.7f, 0.7f,
        1.0f, 0.85f, 0.35f, 1, engine::renderer::PrimitiveRenderer::kStippleDashed, 2.2f);
    engine::renderer::PrimitiveRenderer::drawStippledWireBox(
         0.0f, 0.75f, 0.8f, 1.1f, 0.7f, 0.7f,
        0.4f, 0.92f, 0.85f, 2, engine::renderer::PrimitiveRenderer::kStippleDashDot, 2.2f);
    engine::renderer::PrimitiveRenderer::drawStippledWireBox(
         1.6f, 0.75f, 0.8f, 1.1f, 0.7f, 0.7f,
        0.95f, 0.55f, 0.88f, 1, engine::renderer::PrimitiveRenderer::kStippleDotted, 2.2f);
}

void OpenGLDemoMode::renderModule3_3D() {
    // Module 3: 3D Stroke Text rotating in 3D space + interactive mouse/keyboard feedback cube
    glPushAttrib(GL_ENABLE_BIT | GL_LINE_BIT | GL_CURRENT_BIT);
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);

    glPushMatrix();
    glTranslatef(-1.85f, 1.45f, 0.0f);
    glRotatef(std::sin(m_timeSeconds * 1.5f) * 18.0f, 0.0f, 1.0f, 0.0f);
    glScalef(0.0032f, 0.0032f, 0.0032f);
    glLineWidth(2.2f);
    glColor3f(1.0f, 0.86f, 0.42f);
    glutStrokeString(GLUT_STROKE_ROMAN, reinterpret_cast<const unsigned char*>("GLUT_STROKE_ROMAN 3D"));
    glPopMatrix();

    glPopAttrib();
}

void OpenGLDemoMode::renderModule4_3D() {
    // Module 4: Client-Side Vertex Arrays (glEnableClientState + glVertexPointer + glDrawElements)
    const auto& tex = m_useCheckerTex ? m_texChecker : m_texWood;
    tex.bind();

    glPushMatrix();
    glTranslatef(-1.25f, 1.15f, 0.0f);
    glRotatef(m_timeSeconds * 32.0f, 0.0f, 1.0f, 0.0f);
    m_meshBoxStatic.drawClientSideElements();
    glPopMatrix();

    glPushMatrix();
    glTranslatef(1.25f, 0.65f, 0.0f);
    glRotatef(-m_timeSeconds * 32.0f, 0.0f, 1.0f, 0.0f);
    m_meshPyramidDynamic.drawClientSideElements();
    glPopMatrix();

    engine::assets::Texture2D::unbind();
}

void OpenGLDemoMode::renderModule5_3D() {
    // Module 5: Static VBO (GL_STATIC_DRAW) vs Dynamic Mapped VBO (GL_DYNAMIC_DRAW)
    const auto& tex = m_useCheckerTex ? m_texChecker : m_texWood;
    tex.bind();

    glPushMatrix();
    glTranslatef(-1.35f, 1.15f, 0.0f);
    glRotatef(m_timeSeconds * 28.0f, 0.2f, 1.0f, 0.0f);
    m_meshBoxStatic.drawVBOElements();
    glPopMatrix();

    engine::assets::Texture2D::unbind();

    glPushMatrix();
    glTranslatef(1.35f, 0.65f, 0.0f);
    glRotatef(m_timeSeconds * 38.0f, 0.0f, 1.0f, 0.0f);
    m_meshPyramidDynamic.drawVBOArrays();
    glPopMatrix();
}

void OpenGLDemoMode::renderModule6_3D() {
    // Module 6: 3D Cube (GL_QUADS, glArrayElement) & 3D Pyramid (GL_TRIANGLES, Mapped 4x4 Matrix Translation)
    const auto& tex = m_useCheckerTex ? m_texChecker : m_texWood;
    tex.bind();

    glPushMatrix();
    glTranslatef(-1.35f, 1.15f, 0.0f);
    glRotatef(m_timeSeconds * 30.0f, 0.0f, 1.0f, 0.0f);
    m_meshBoxStatic.drawVBOArrayElement();
    glPopMatrix();
    engine::assets::Texture2D::unbind();

    glPushMatrix();
    glTranslatef(1.35f, 0.65f, 0.0f);
    glRotatef(-m_timeSeconds * 35.0f, 0.0f, 1.0f, 0.0f);
    m_meshPyramidDynamic.drawVBOArrays();
    glPopMatrix();
}

void OpenGLDemoMode::renderModule7_3D() {
    // Module 7: Hierarchical Matrix Stack (Solar/Planetary Orbit using glPushMatrix/glPopMatrix)
    engine::renderer::LightingSystem::applyMaterial(
        engine::renderer::LightingSystem::polishedCeramicMaterial());

    glPushMatrix();
    glTranslatef(0.0f, 1.35f, 0.0f);

    // Central Sun Sphere
    glPushMatrix();
    glRotatef(m_timeSeconds * 25.0f, 0.0f, 1.0f, 0.0f);
    glColor3f(1.0f, 0.82f, 0.32f);
    if (m_wireframeMode) glutWireSphere(0.55, 20, 20); else glutSolidSphere(0.55, 20, 20);
    glPopMatrix();

    // Orbiting Planet Cube (glRotatef -> glTranslatef -> glScalef)
    glPushMatrix();
    glRotatef(m_timeSeconds * 55.0f, 0.0f, 1.0f, 0.0f);
    glTranslatef(1.85f, 0.0f, 0.0f);
    glColor3f(0.38f, 0.78f, 0.96f);
    if (m_wireframeMode) glutWireCube(0.45); else glutSolidCube(0.45);

    // Orbiting Moon Torus around the Planet (Nested Matrix Stack Level 3!)
    glPushMatrix();
    glRotatef(m_timeSeconds * 140.0f, 0.0f, 1.0f, 0.0f);
    glTranslatef(0.55f, 0.0f, 0.0f);
    glColor3f(0.95f, 0.55f, 0.78f);
    if (m_wireframeMode) glutWireTorus(0.05, 0.14, 12, 18); else glutSolidTorus(0.05, 0.14, 12, 18);
    glPopMatrix();

    glPopMatrix();
    glPopMatrix();
}

void OpenGLDemoMode::renderModule8_3D() {
    // Module 8: Phong Lighting + Spotlight + Materials + GLUT 3D Objects + Texture Mapping
    if (m_materialPresetIdx == 0) {
        engine::renderer::LightingSystem::applyMaterial(
            engine::renderer::LightingSystem::polishedCeramicMaterial());
    } else if (m_materialPresetIdx == 1) {
        engine::renderer::LightingSystem::applyMaterial(
            engine::renderer::LightingSystem::matteWoodMaterial());
    } else {
        engine::renderer::LightingSystem::applyMaterial(
            engine::renderer::LightingSystem::glowingLampMaterial());
    }

    // 1. GLUT Teapot
    glPushMatrix();
    glTranslatef(-1.75f, 1.25f, 0.0f);
    glRotatef(m_timeSeconds * 30.0f, 0.0f, 1.0f, 0.0f);
    glColor3f(0.96f, 0.82f, 0.45f);
    if (m_wireframeMode) glutWireTeapot(0.48); else glutSolidTeapot(0.48);
    glPopMatrix();

    // 2. GLUT Sphere & Torus
    glPushMatrix();
    glTranslatef(0.0f, 1.25f, 0.0f);
    glRotatef(m_timeSeconds * 35.0f, 1.0f, 1.0f, 0.0f);
    glColor3f(0.45f, 0.85f, 0.95f);
    if (m_wireframeMode) glutWireTorus(0.16, 0.42, 18, 24); else glutSolidTorus(0.16, 0.42, 18, 24);
    glPopMatrix();

    // 3. Textured 3D Box with Mipmaps & GL_MODULATE
    const auto& tex = m_useCheckerTex ? m_texChecker : m_texWood;
    tex.bind();
    glPushMatrix();
    glTranslatef(1.75f, 1.25f, 0.0f);
    glRotatef(m_timeSeconds * 28.0f, 0.0f, 1.0f, 0.0f);
    m_meshBoxStatic.drawVBOElements();
    glPopMatrix();
    engine::assets::Texture2D::unbind();
}

void OpenGLDemoMode::render3D(const engine::camera::Camera3D& camera) {
    if (!m_active) {
        return;
    }

    camera.applyProjection();
    camera.applyView();

    m_demoLighting.applyLights(m_timeSeconds);
    engine::renderer::PrimitiveRenderer::drawReferenceGrid(4.5f, 0.75f);
    engine::renderer::PrimitiveRenderer::drawCoordinateAxes(1.8f);

    switch (m_currentModule) {
        case 1: renderModule1_3D(); break;
        case 2: renderModule2_3D(); break;
        case 3: renderModule3_3D(); break;
        case 4: renderModule4_3D(); break;
        case 5: renderModule5_3D(); break;
        case 6: renderModule6_3D(); break;
        case 7: renderModule7_3D(); break;
        case 8: renderModule8_3D(); break;
        default: renderModule1_3D(); break;
    }
}

void OpenGLDemoMode::render2D(const engine::camera::Camera3D& camera) {
    if (!m_active) {
        return;
    }

    const int w = camera.getViewportWidth();
    const int h = camera.getViewportHeight();

    engine::camera::Camera3D::begin2DOverlay(w, h);

    // Top Coursework Showcase Header Panel
    float topW = static_cast<float>(w) - 40.0f;
    engine::ui::TextRenderer::drawPanel2D(20.0f, static_cast<float>(h - 125), topW, 108.0f,
                                          0.05f, 0.07f, 0.14f, 0.94f, 1.0f, 0.84f, 0.36f);

    static const char* kModuleTitles[8] = {
        "MODULE 1: INTRODUCTION TO OPENGL & RENDERING PIPELINE",
        "MODULE 2: GEOMETRIC PRIMITIVES, COLORS & LINE STIPPLING",
        "MODULE 3: GLUT EXTENSIONS, BITMAP/STROKE TEXT & CALLBACKS",
        "MODULE 4: CLIENT-SIDE VERTEX ARRAYS & INDEXED DRAWING",
        "MODULE 5: VERTEX BUFFER OBJECTS (VBOs) & GPU MEMORY",
        "MODULE 6: 4x4 MATRIX OPERATIONS & HOMOGENEOUS COORDINATES",
        "MODULE 7: 3D TRANSFORMATIONS, MATRIX STACKS & VIEWING",
        "MODULE 8: PHONG LIGHTING, SPOTLIGHTS, MATERIALS & TEXTURES"
    };

    static const char* kModuleAPIs[8] = {
        "APIs: glutInit, glutCreateWindow, glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT), glBegin(GL_POLYGON), glVertex2f, glEnd, glFlush",
        "APIs: GL_POINTS, glPointSize, GL_LINES, GL_LINE_STRIP, GL_LINE_LOOP, glLineStipple(0x00FF/0x0C0F/0xAAAA), GL_TRIANGLES..GL_POLYGON, glColor3ub",
        "APIs: glColor* before glRasterPos2f, glutBitmapString, glutStrokeString, glutKeyboardFunc, glutMouseFunc, glutMotionFunc, glutEntryFunc, glutTimerFunc",
        "APIs: glEnableClientState(GL_VERTEX_ARRAY|GL_COLOR_ARRAY|GL_NORMAL_ARRAY|GL_TEXTURE_COORD_ARRAY), glVertexPointer, glDrawArrays, glDrawElements",
        "APIs: glewInit, glGenBuffers, glIsBuffer, glBindBuffer(GL_ARRAY_BUFFER/GL_ELEMENT_ARRAY_BUFFER), glBufferData(GL_STATIC_DRAW/GL_DYNAMIC_DRAW), glMapBuffer",
        "APIs: Homogeneous Vec4(x,y,z,w), 4x4 Mat4 Add/Sub/Scalar/MatVec/MatMat, BUFFER_OFFSET(i), glBufferSubData, glArrayElement, glDebugMessageCallback",
        "APIs: glMatrixMode(GL_MODELVIEW/GL_PROJECTION), glPushMatrix/glPopMatrix, glViewport, glTranslatef/glRotatef/glScalef, gluPerspective, glFrustum, glOrtho, gluLookAt",
        "APIs: glEnable(GL_LIGHTING/GL_LIGHT0/GL_LIGHT1), glLightfv(GL_SPOT_DIRECTION/CUTOFF), glMaterialfv, glutSolidTeapot, stbi_load, glTexImage2D, glGenerateMipmap"
    };

    static const char* kModuleWhereUsed[8] = {
        "Used In Game: src/main.cpp (Window init & render loop) and src/engine/core/Logger.cpp (Context info)",
        "Used In Game: src/engine/renderer/PrimitiveRenderer.cpp (Rain streaks, puddle ripples, dust motes, AABB boxes) & RhythmMinigame.cpp",
        "Used In Game: src/engine/ui/TextRenderer.cpp, InputManager.cpp, DialogueSystem.cpp, and PhoneSystem.cpp",
        "Used In Game: src/engine/renderer/MeshVBO.cpp::drawClientSideElements() (Classroom walls & environment geometry)",
        "Used In Game: src/engine/renderer/MeshVBO.cpp (Static room VBOs) & SpriteQuad3D.cpp (Dynamic character sprite VBO)",
        "Used In Game: src/engine/core/Math4x4.cpp::translateVertexInMappedVBO() (Animated desk pyramid) & MeshVBO::drawVBOArrayElement() (Chalkboard)",
        "Used In Game: src/engine/camera/Camera3D.cpp (2.5D follow camera & 2D HUD) and ClassroomScene.cpp (Desk/chair hierarchies)",
        "Used In Game: src/engine/renderer/LightingSystem.cpp (Sunset window light + ceiling/streetlights) & Texture2D.cpp (16 PNG textures/sprites)"
    };

    int idx = m_currentModule - 1;
    engine::ui::TextRenderer::drawStrokeText2D(
        36.0f, static_cast<float>(h - 52),
        kModuleTitles[idx],
        0.135f, 2.0f, GLUT_STROKE_ROMAN,
        1.0f, 0.88f, 0.46f);

    engine::ui::TextRenderer::drawBitmapText2D(
        36.0f, static_cast<float>(h - 78),
        kModuleAPIs[idx],
        GLUT_BITMAP_HELVETICA_12, 0.88f, 0.95f, 1.0f);

    engine::ui::TextRenderer::drawBitmapText2D(
        36.0f, static_cast<float>(h - 98),
        kModuleWhereUsed[idx],
        GLUT_BITMAP_HELVETICA_12, 0.68f, 0.98f, 0.78f);

    engine::ui::TextRenderer::drawBitmapText2D(
        36.0f, static_cast<float>(h - 116),
        "Demo Controls: [1-8] Select Module | [Left/Right] Prev/Next | [P] Projection | [1-4 Keys in M8: Lights] | [M] Wireframe | [T] Checker | [F2] Return to Game",
        GLUT_BITMAP_HELVETICA_12, 1.0f, 0.84f, 0.58f);

    // Module-specific interactive live data panel at the bottom
    engine::ui::TextRenderer::drawPanel2D(20.0f, 20.0f, topW, 105.0f,
                                          0.05f, 0.06f, 0.12f, 0.92f, 0.55f, 0.78f, 0.95f);

    if (m_currentModule == 1) {
        engine::ui::TextRenderer::drawBitmapText2D(
            36.0f, 92.0f,
            "RENDERING PIPELINE STAGES:  [1. Application (CPU)]  --->  [2. Geometry (Transform/Light)]  --->  [3. Rasterization (Fragments)]  --->  [4. Screen (Framebuffer)]",
            GLUT_BITMAP_HELVETICA_12, 1.0f, 0.92f, 0.55f);
        engine::ui::TextRenderer::drawBitmapText2D(
            36.0f, 66.0f,
            "Active Context: OpenGL Compatibility Profile | Double Buffered (GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH) | Active GL Errors: " +
                std::to_string(engine::core::Logger::getErrorCount()),
            GLUT_BITMAP_HELVETICA_12, 0.88f, 0.94f, 1.0f);
    } else if (m_currentModule == 3) {
        std::ostringstream cbLine;
        cbLine << "LIVE GLUT CALLBACK STATE: Mouse Pos=("
               << engine::input::InputManager::getMouseX() << ", "
               << engine::input::InputManager::getMouseY() << ") | LeftBtn="
               << (engine::input::InputManager::isMouseButtonDown(GLUT_LEFT_BUTTON) ? "DOWN" : "UP")
               << " | RightBtn="
               << (engine::input::InputManager::isMouseButtonDown(GLUT_RIGHT_BUTTON) ? "DOWN" : "UP")
               << " | glutEntryFunc="
               << (engine::input::InputManager::isMouseInsideWindow() ? "GLUT_ENTERED" : "GLUT_LEFT");
        engine::ui::TextRenderer::drawBitmapText2D(36.0f, 92.0f, cbLine.str(), GLUT_BITMAP_HELVETICA_12, 1.0f, 0.92f, 0.55f);
        engine::ui::TextRenderer::drawBitmapText2D(36.0f, 66.0f, "Font Sample 1: GLUT_BITMAP_TIMES_ROMAN_24", GLUT_BITMAP_TIMES_ROMAN_24, 0.95f, 0.82f, 0.48f);
        engine::ui::TextRenderer::drawBitmapText2D(520.0f, 66.0f, "Font Sample 2: GLUT_BITMAP_9_BY_15 (Monospace)", GLUT_BITMAP_9_BY_15, 0.68f, 0.96f, 0.82f);
    } else if (m_currentModule == 5 || m_currentModule == 6) {
        using engine::core::Mat4;
        using engine::core::Vec4;
        Mat4 T = Mat4::translation(1.5f, 2.0f, -0.5f);
        Vec4 pt = Vec4::point(1.0f, 0.0f, 0.0f);
        Vec4 dir = Vec4::direction(1.0f, 0.0f, 0.0f);
        Vec4 tPt = Mat4::multiplyVec4(T, pt);
        Vec4 tDir = Mat4::multiplyVec4(T, dir);

        std::ostringstream m6Line;
        m6Line << std::fixed << std::setprecision(1)
               << "GPU Buffers: StaticBox VBO=" << m_meshBoxStatic.getVBOId()
               << " (glIsBuffer=" << (m_meshBoxStatic.isValid() ? "TRUE" : "FALSE") << ")"
               << " | DynamicPyramid VBO=" << m_meshPyramidDynamic.getVBOId()
               << " (Mapped via glMapBuffer(GL_ARRAY_BUFFER, GL_READ_WRITE))";
        engine::ui::TextRenderer::drawBitmapText2D(36.0f, 92.0f, m6Line.str(), GLUT_BITMAP_HELVETICA_12, 1.0f, 0.92f, 0.55f);

        std::ostringstream homLine;
        homLine << std::fixed << std::setprecision(1)
                << "Module 6 Homogeneous Math: T(1.5, 2.0, -0.5) * Point(1,0,0, w=1) = ("
                << tPt.x << ", " << tPt.y << ", " << tPt.z << ", w=" << tPt.w << ")  |  "
                << "T * Direction(1,0,0, w=0) = ("
                << tDir.x << ", " << tDir.y << ", " << tDir.z << ", w=" << tDir.w << ")";
        engine::ui::TextRenderer::drawBitmapText2D(36.0f, 66.0f, homLine.str(), GLUT_BITMAP_HELVETICA_12, 0.78f, 0.96f, 0.88f);
    } else {
        std::ostringstream genLine;
        genLine << "Active Projection [P]: " << camera.getProjectionModeName()
                << "  |  Wireframe [M]: " << (m_wireframeMode ? "ON" : "OFF")
                << "  |  UV Checker [T]: " << (m_useCheckerTex ? "ON" : "OFF")
                << "  |  Material Preset [Space]: " << m_materialPresetIdx;
        engine::ui::TextRenderer::drawBitmapText2D(36.0f, 82.0f, genLine.str(), GLUT_BITMAP_HELVETICA_18, 0.92f, 0.96f, 1.0f);
    }

    engine::ui::TextRenderer::drawBitmapText2D(
        36.0f, 34.0f,
        "Station " + std::to_string(m_currentModule) + " of 8   |   Use [Left / Right Arrows] or Keys [1..8] to inspect any module   |   Press [F2] to exit Demo Mode",
        GLUT_BITMAP_HELVETICA_12, 0.82f, 0.88f, 0.96f);

    engine::camera::Camera3D::end2DOverlay();
}

void OpenGLDemoMode::onKeyPress(unsigned char key, engine::camera::Camera3D& camera) {
    if (!m_active) {
        return;
    }

    if (key >= '1' && key <= '8') {
        selectModule(key - '0');
    } else if (key == 9) { // Tab
        nextModule();
    } else if (key == 'p' || key == 'P') {
        camera.cycleProjectionMode();
    } else if (key == 'm' || key == 'M') {
        m_wireframeMode = !m_wireframeMode;
    } else if (key == 't' || key == 'T') {
        m_useCheckerTex = !m_useCheckerTex;
    } else if (key == ' ') {
        m_materialPresetIdx = (m_materialPresetIdx + 1) % 3;
    }
}

void OpenGLDemoMode::onSpecialKeyPress(int key) {
    if (!m_active) {
        return;
    }
    if (key == GLUT_KEY_RIGHT) {
        nextModule();
    } else if (key == GLUT_KEY_LEFT) {
        prevModule();
    }
}

} // namespace debug
