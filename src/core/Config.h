#pragma once
#include <imgui.h>
#include <string>

enum SampleDomainMode {
    SAMPLE_DOMAIN_SYMMETRIC = 0,  // [-T/2, +T/2]
    SAMPLE_DOMAIN_CAUSAL          // [0, T]
};

enum PanelDockLocation {
    PANEL_TOP = 0,
    PANEL_LEFT,
    PANEL_RIGHT,
    PANEL_FLOAT
};

class Scene;

struct AppConfig {
    ImVec4 funcColor = ImVec4(80 / 255.f, 160 / 255.f, 255 / 255.f, 255 / 255.f);
    ImVec4 gridColor = ImVec4(0, 0, 0, 0.24f);
    ImVec4 axisColor = ImVec4(1, 0, 0, 1);
    ImVec4 backgroundColor = ImVec4(1, 1, 1, 1);
    ImVec4 quadColor = ImVec4(1, 0, 1, 0.25f);
    ImVec4 quadBorderColor = ImVec4(0, 0, 1, 0.8f);

    int   samples = 500;
    int   gridSpacing = 50;
    int gridScale = 100;

    int sampleDomainMode = SAMPLE_DOMAIN_SYMMETRIC;

    int panelLocation = PANEL_RIGHT;

    int panX = 0; // pixels
    int panY = 0; // pixels

    static constexpr int kExprBufSize = 512; 
    char funcExpr[512] = "x"; 

    // helpers for ImGui::InputText
    char* funcExprBuf() { return funcExpr; }
    const char* funcExprBuf() const { return funcExpr; }
    int funcExprBufSize() const { return kExprBufSize; }

    bool Load(const char* file, Scene& scene);
    void Save(const char* file) const;
};