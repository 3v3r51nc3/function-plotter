#include "Scene.h"
#include "core/Config.h"
#include <cmath>
#include <sstream>
#include <exprtk.hpp>
#include <vector>
#include <algorithm>

static inline ImU32 RGBA(const ImVec4& c) {
    return IM_COL32(int(c.x * 255), int(c.y * 255), int(c.z * 255), int(c.w * 255));
}

struct Scene::Impl {
    using symbol_table_t = exprtk::symbol_table<float>;
    using expression_t = exprtk::expression<float>;
    using parser_t = exprtk::parser<float>;

    symbol_table_t symbols;
    expression_t   expression;
    parser_t       parser;
    float varX = 0.0f;
    bool valid = false;
    std::string lastError;

    Impl() {
        symbols.add_variable("x", varX);
        symbols.add_constants();
        expression.register_symbol_table(symbols);
    }
};

Scene::Scene() : impl(std::make_unique<Impl>()) {}
Scene::~Scene() = default;   // now compiler sees full Impl type

void Scene::SetExpression(const std::string& expr) {
    impl->valid = impl->parser.compile(expr, impl->expression);
    if (!impl->valid) {
        std::ostringstream oss;
        oss << "Parse error in expression: " << expr << "\n";
        for (std::size_t i = 0; i < impl->parser.error_count(); ++i) {
            auto e = impl->parser.get_error(i);
            oss << "Error " << i
                << " at pos " << e.token.position
                << " [" << exprtk::parser_error::to_str(e.mode)
                << "] " << e.diagnostic << "\n";
        }
        impl->lastError = oss.str();
        //MessageBoxA(nullptr, oss.str().c_str(), "Expression Error", MB_OK | MB_ICONERROR);
    }
    else {
        impl->lastError.clear();
    }
}

float Scene::Eval(float x) {
    if (!impl->valid) return 0.0f;
    impl->varX = x;
    return impl->expression.value();
}

void Scene::DrawBackground(const ImVec2& plotPos, const ImVec2& plotSize, const AppConfig& cfg) {
    const float centerX = plotPos.x + plotSize.x * 0.5f + (float)cfg.panX;
    const float centerY = plotPos.y + plotSize.y * 0.5f + (float)cfg.panY;
    const float step = cfg.gridSpacing * (std::max(cfg.gridScale, 1) / 100.0f); // pixels per world unit

    ImDrawList* dl = ImGui::GetBackgroundDrawList();
    dl->PushClipRect(plotPos, ImVec2(plotPos.x + plotSize.x, plotPos.y + plotSize.y), true);
    auto colGrid = IM_COL32(cfg.gridColor.x * 255, cfg.gridColor.y * 255, cfg.gridColor.z * 255, cfg.gridColor.w * 255);
    auto colAxis = IM_COL32(cfg.axisColor.x * 255, cfg.axisColor.y * 255, cfg.axisColor.z * 255, cfg.axisColor.w * 255);

    const int iStartX = (int)std::floor((plotPos.x - centerX) / step);
    const int iEndX   = (int)std::ceil((plotPos.x + plotSize.x - centerX) / step);
    const int iStartY = (int)std::floor((plotPos.y - centerY) / step);
    const int iEndY   = (int)std::ceil((plotPos.y + plotSize.y - centerY) / step);

    // vertical grid
    for (int i = iStartX; i <= iEndX; ++i) {
        float x = centerX + i * step;
        dl->AddLine(ImVec2(x, plotPos.y), ImVec2(x, plotPos.y + plotSize.y), colGrid);
    }
    // horizontal grid
    for (int i = iStartY; i <= iEndY; ++i) {
        float y = centerY + i * step;
        dl->AddLine(ImVec2(plotPos.x, y), ImVec2(plotPos.x + plotSize.x, y), colGrid);
    }

    // axes
    dl->AddLine(ImVec2(plotPos.x, centerY), ImVec2(plotPos.x + plotSize.x, centerY), colAxis);
    dl->AddLine(ImVec2(centerX, plotPos.y), ImVec2(centerX, plotPos.y + plotSize.y), colAxis);

    // arrows
    dl->AddTriangleFilled({ plotPos.x + plotSize.x - 10, centerY - 5 }, { plotPos.x + plotSize.x, centerY }, { plotPos.x + plotSize.x - 10, centerY + 5 }, colAxis);
    dl->AddTriangleFilled({ centerX - 5, plotPos.y + 10 }, { centerX, plotPos.y }, { centerX + 5, plotPos.y + 10 }, colAxis);

    // X ticks (world units)
    for (int i = iStartX; i <= iEndX; ++i) {
        if (i == 0) continue;
        float x = centerX + i * step;
        dl->AddLine({ x, centerY - 5 }, { x, centerY + 5 }, colAxis);
        dl->AddText({ x + 2, centerY + 10 }, colAxis, std::to_string(i).c_str());
    }
    // Y ticks (world units)
    for (int i = iStartY; i <= iEndY; ++i) {
        if (i == 0) continue;
        float y = centerY + i * step;
        dl->AddLine({ centerX - 5, y }, { centerX + 5, y }, colAxis);
        dl->AddText({ centerX + 10, y - 8 }, colAxis, std::to_string(-i).c_str());
    }
    dl->PopClipRect();
}

void Scene::DrawFunction(const ImVec2& center, const ImVec2& plotPos, const ImVec2& plotSize, const AppConfig& cfg) {
    const int N = (cfg.samples > 2 ? cfg.samples : 2);
    const float unit = cfg.gridSpacing * (std::max(cfg.gridScale, 1) / 100.0f); // pixels per world unit
    if (unit <= 0.0f) return;

    // Sample strictly across the visible viewport in screen space.
    std::vector<ImVec2> pts;
    pts.reserve(N);

    const bool causal = (cfg.sampleDomainMode == SAMPLE_DOMAIN_CAUSAL);
    
    // In causal mode, find the screen position where x_world = 0
    float xZeroScreen = center.x;
    int iStart = 0;
    if (causal) {
        xZeroScreen = center.x; // x=0 in world space maps to center.x in screen space
        // Find the first sample index where x_world >= 0
        for (int i = 0; i < N; ++i) {
            const float sx = plotPos.x + (float)i * (plotSize.x / (float)(N - 1));
            const float x_world = (sx - center.x) / unit;
            if (x_world >= -0.001f) { // small epsilon for floating point
                iStart = i;
                break;
            }
        }
    }
    
    for (int i = iStart; i < N; ++i) {
        const float sx = plotPos.x + (float)i * (plotSize.x / (float)(N - 1));
        const float x_world = (sx - center.x) / unit;
        const float y = Eval(x_world);
        const float sy = center.y - y * unit;
        pts.emplace_back(sx, sy);
    }

    ImDrawList* dl = ImGui::GetBackgroundDrawList();
    dl->PushClipRect(plotPos, ImVec2(plotPos.x + plotSize.x, plotPos.y + plotSize.y), true);
    for (size_t i = 1; i < pts.size(); ++i) dl->AddLine(pts[i - 1], pts[i], RGBA(cfg.funcColor), 2.0f);
    dl->PopClipRect();
}

bool Scene::HasError() const {
    return !impl->valid;
}

const std::string& Scene::GetLastError() const {
    return impl->lastError;
}
