#include "RendererGL.h"
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

bool RendererGL::Init(GLFWwindow* window) {
    m_window = window;
    if (!m_window) return false;

    glfwGetFramebufferSize(window, &m_width, &m_height);
    glViewport(0, 0, m_width, m_height);
    
    return true;
}

void RendererGL::Cleanup() {
    // Nothing specific to clean up for OpenGL in this simple case
    m_window = nullptr;
}

void RendererGL::BeginFrame(float r, float g, float b, float a) {
    if (!m_window) return;
    
    glClearColor(r, g, b, a);
    glClear(GL_COLOR_BUFFER_BIT);
}

void RendererGL::EndFrame() {
    if (!m_window) return;
    // Swap is handled by GLFW in the main loop
}

void RendererGL::OnResize(int w, int h) {
    m_width = w;
    m_height = h;
    glViewport(0, 0, w, h);
}
