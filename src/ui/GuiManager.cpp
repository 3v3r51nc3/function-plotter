#include "GuiManager.h"
#include "render/RendererGL.h"
#include "render/Scene.h"
#include "core/Config.h"

#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <GLFW/glfw3.h>

void GuiManager::Init(GLFWwindow* window, RendererGL& renderer) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsLight();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 150");
}

void GuiManager::Shutdown() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void GuiManager::BeginFrame() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void GuiManager::EndFrame(RendererGL& renderer) {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

static void HelpMarker(const char* d) { ImGui::SameLine(); ImGui::TextDisabled("(?)"); if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", d); }

void GuiManager::ShowMainMenu(AppConfig& cfg, Scene& scene) {
    // Dockable control bar with fixed height/width per side, or floating window
    ImGuiViewport* vp = ImGui::GetMainViewport();
    const float topHeight = 130.0f;
    const float sideWidth = 280.0f;
    ImGuiWindowFlags flags;
    if (cfg.panelLocation == PANEL_FLOAT) {
        ImGui::SetNextWindowPos(ImVec2(vp->Pos.x + vp->Size.x - 420.0f, vp->Pos.y + 20.0f), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(360.0f, 320.0f), ImGuiCond_FirstUseEver);
        flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings;
    } else {
        if (cfg.panelLocation == PANEL_TOP) {
            ImGui::SetNextWindowPos(vp->Pos);
            ImGui::SetNextWindowSize(ImVec2(vp->Size.x, topHeight));
        } else if (cfg.panelLocation == PANEL_LEFT) {
            ImGui::SetNextWindowPos(vp->Pos);
            ImGui::SetNextWindowSize(ImVec2(sideWidth, vp->Size.y));
        } else { // PANEL_RIGHT
            ImGui::SetNextWindowPos(ImVec2(vp->Pos.x + vp->Size.x - sideWidth, vp->Pos.y));
            ImGui::SetNextWindowSize(ImVec2(sideWidth, vp->Size.y));
        }
        flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoNav;
    }

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10, 6));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8, 4));
    ImGui::SetNextWindowBgAlpha(0.92f);
    ImGui::Begin("ControlBar", nullptr, flags);

    if (ImGui::BeginTabBar("ControlTabs", ImGuiTabBarFlags_FittingPolicyResizeDown)) {
        if (ImGui::BeginTabItem("Function")) {
            if (ImGui::InputTextWithHint("f(x)", "e.g. sin(x)", cfg.funcExprBuf(), cfg.funcExprBufSize(),
                ImGuiInputTextFlags_EnterReturnsTrue) ||
                ImGui::IsItemDeactivatedAfterEdit()) {
                scene.SetExpression(cfg.funcExprBuf());
            }
            if (scene.HasError()) ImGui::TextColored({ 1,0,0,1 }, "%s", scene.GetLastError().c_str());

            ImGui::ColorEdit4("Function color", (float*)&cfg.funcColor);
            ImGui::DragInt("Samples", &cfg.samples, 1, 64, 16384);
            HelpMarker("More samples = smoother line, but slower. 256–2048 is usually enough.");

            bool causal = (cfg.sampleDomainMode == SAMPLE_DOMAIN_CAUSAL);
            if (ImGui::Checkbox("Start at x = 0 (causal)", &causal)) {
                cfg.sampleDomainMode = causal ? SAMPLE_DOMAIN_CAUSAL : SAMPLE_DOMAIN_SYMMETRIC;
            }
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("View")) {
            ImGui::SliderInt("Grid spacing (px)", &cfg.gridSpacing, 1, 5000);
            ImGui::SliderInt("Scale (%)", &cfg.gridScale, 10, 500);
            ImGui::ColorEdit4("Grid color", (float*)&cfg.gridColor);
            ImGui::ColorEdit4("Axis color", (float*)&cfg.axisColor);
            ImGui::ColorEdit4("Background", (float*)&cfg.backgroundColor);
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Prefs")) {
            if (ImGui::Button("Save")) cfg.Save("config.ini");
            ImGui::SameLine();
            if (ImGui::Button("Load")) cfg.Load("config.ini", scene);

            ImGui::Spacing();
            ImGui::Text("FPS %.3f ms/frame (%.1f F/s)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

            const char* locs[] = { "Top", "Left", "Right", "Floating" };
            ImGui::Text("Panel position");
            ImGui::SameLine();
            ImGui::Combo("##panel_loc", &cfg.panelLocation, locs, IM_ARRAYSIZE(locs));
            ImGui::EndTabItem();
        }
    }
    ImGui::EndTabBar();

    ImGui::End();
    ImGui::PopStyleVar(3);
}