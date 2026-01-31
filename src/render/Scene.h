#pragma once
#include <string>
#include <imgui.h>
#include <memory>

struct AppConfig;

class Scene {
public:
    Scene();
    ~Scene();

    void SetExpression(const std::string& expr);
    void DrawBackground(const ImVec2& plotPos, const ImVec2& plotSize, const AppConfig& cfg);
    void DrawFunction(const ImVec2& center, const ImVec2& plotPos, const ImVec2& plotSize, const AppConfig& cfg);

    bool HasError() const;
    const std::string& GetLastError() const;

private:
    float Eval(float x);

    struct Impl;
    std::unique_ptr<Impl> impl;
};