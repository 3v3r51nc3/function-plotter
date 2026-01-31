#pragma once
#include "render/RendererGL.h"
#include "ui/GuiManager.h"
#include "render/Scene.h"
#include "core/Config.h"
#include "Animation.h"

struct GLFWwindow;

class App {
public:
    App();
    int Run();

    void OnResize(int w, int h);

private:
    bool CreateMainWindow();
    void DestroyMainWindow();

private:
    GLFWwindow* m_window = nullptr;
    std::string m_title = "Function Visualizer (OpenGL)";

    RendererGL m_renderer;
    GuiManager  m_gui;
    Scene       m_scene;
    AppConfig   m_cfg;
    ScaleAnimation m_scaleAnim;

    float  m_targetScale = 100.0f;
    float m_scaleVel = 0.0f;
    double m_prevTime = 0.0;
};