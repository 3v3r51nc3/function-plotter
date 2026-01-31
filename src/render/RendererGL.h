#pragma once

#if defined(_WIN32)
    #include <windows.h>
    #include <GL/gl.h>
#elif defined(__APPLE__)
    #include <OpenGL/gl.h>
#else
    #include <GL/gl.h>
#endif

struct GLFWwindow;

class RendererGL {
public:
    RendererGL() = default;
    ~RendererGL() = default;

    bool Init(GLFWwindow* window);
    void Cleanup();

    void BeginFrame(float r, float g, float b, float a);
    void EndFrame();

    void OnResize(int w, int h);

    GLFWwindow* GetWindow() const { return m_window; }

private:
    GLFWwindow* m_window = nullptr;
    int m_width = 0;
    int m_height = 0;
};