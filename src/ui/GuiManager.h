#pragma once
#include <imgui.h>

struct GLFWwindow;
class RendererGL;
class Scene;
struct AppConfig;

class GuiManager {
public:
    void Init(GLFWwindow* window, RendererGL& renderer);
    void Shutdown();

    void BeginFrame();
    void EndFrame(RendererGL& renderer);

    void ShowMainMenu(AppConfig& cfg, Scene& scene);
};