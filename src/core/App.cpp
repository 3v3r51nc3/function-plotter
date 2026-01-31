#include "App.h"
#include <GLFW/glfw3.h>
#include <algorithm>
#include <cmath>

#ifdef max
#undef max
#endif

#ifdef min
#undef min
#endif

static App *g_app = nullptr;

static void glfw_error_callback(int error, const char *description)
{
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

static void glfw_resize_callback(GLFWwindow *window, int width, int height)
{
    if (g_app)
        g_app->OnResize(width, height);
}

App::App()
{
    g_app = this;
}

bool App::CreateMainWindow()
{
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return false;

    // GL 3.2 + GLSL 150 (required for macOS)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // 3.2+ only
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // Required on Mac
#endif

    // Get primary monitor for maximized window
    GLFWmonitor *monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode *mode = glfwGetVideoMode(monitor);

    m_window = glfwCreateWindow(mode->width, mode->height, m_title.c_str(), nullptr, nullptr);
    if (m_window == nullptr)
        return false;

    glfwMaximizeWindow(m_window);
    glfwMakeContextCurrent(m_window);
    glfwSwapInterval(1); // Enable vsync

    glfwSetFramebufferSizeCallback(m_window, glfw_resize_callback);

    return true;
}

void App::DestroyMainWindow()
{
    if (m_window)
    {
        glfwDestroyWindow(m_window);
        m_window = nullptr;
    }
    glfwTerminate();
}

int App::Run()
{
    if (!CreateMainWindow())
        return 1;
    if (!m_renderer.Init(m_window))
    {
        DestroyMainWindow();
        return 1;
    }

    // ImGui init
    m_gui.Init(m_window, m_renderer);

    // Fonts (optional, Cyrillic)
    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard | ImGuiConfigFlags_NavEnableGamepad;

    // Cross-platform font loading
    ImFont *font = nullptr;

#ifdef _WIN32
    font = io.Fonts->AddFontFromFileTTF(
        "C:\\Windows\\Fonts\\arial.ttf", 14.0f, nullptr,
        io.Fonts->GetGlyphRangesCyrillic());

#elif __APPLE__
    font = io.Fonts->AddFontFromFileTTF(
        "/System/Library/Fonts/Helvetica.ttc", 14.0f, nullptr,
        io.Fonts->GetGlyphRangesCyrillic());

#else
    // Linux: try common distro paths, then fallback
    const char *paths[] = {
        "/usr/share/fonts/dejavu/DejaVuSans.ttf",          // Fedora
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", // Debian/Ubuntu
        "/usr/share/fonts/noto/NotoSans-Regular.ttf",
        "/usr/share/fonts/truetype/noto/NotoSans-Regular.ttf"};

    for (const char *p : paths)
    {
        if (FILE *f = fopen(p, "rb"))
        {
            fclose(f);
            font = io.Fonts->AddFontFromFileTTF(
                p, 14.0f, nullptr, io.Fonts->GetGlyphRangesCyrillic());
            break;
        }
    }

    if (!font)
        font = io.Fonts->AddFontDefault();
#endif

    // Safety: never crash even if everything fails
    if (!font)
        font = io.Fonts->AddFontDefault();

    (void)font;

    // Load config, then ensure origin is centered at startup
    m_cfg.Load("config.ini", m_scene);
    m_cfg.panX = 0;
    m_cfg.panY = 0;

    // Main loop
    while (!glfwWindowShouldClose(m_window))
    {
        glfwPollEvents();

        // Begin GUI frame
        m_gui.BeginFrame();

        double now = ImGui::GetTime();
        float dt = (m_prevTime == 0.0) ? 0.0f : float(now - m_prevTime);
        m_prevTime = now;
        if (dt > 0.1f)
            dt = 0.1f;

        // convert current scale to exponent form (logarithmic zoom space)
        auto ToExp = [](float scale)
        { return std::log(scale / 100.0f); };
        auto FromExp = [](float exp)
        { return 100.0f * std::exp(exp); };

        static float zoomExp = ToExp(m_cfg.gridScale); // current exp
        static float targetExp = zoomExp;              // target exp
        static float expVel = 0.0f;                    // velocity in exp space

        // 1. accumulate target from wheel input
        if (io.MouseWheel != 0.0f)
        {
            float step = 0.15f; // 15% per wheel unit
            targetExp += io.MouseWheel * std::log(1.0f + step);
        }

        // 2. reset on R
        if (ImGui::IsKeyPressed(ImGuiKey_R))
        {
            targetExp = 0.0f; // exp(0) = 1 → scale=100
            m_cfg.panX = 0;
            m_cfg.panY = 0;
        }

        // 3. spring smoothing in exp space
        const float omega = 12.0f; // responsiveness
        float x = zoomExp - targetExp;
        float a = -2.0f * omega * expVel - (omega * omega) * x;
        expVel += a * dt;
        zoomExp += expVel * dt;

        // 4. convert back to scale, clamp (avoid std::clamp portability)
        {
            float s = FromExp(zoomExp);
            if (s < 10.0f)
                s = 10.0f;
            if (s > 500.0f)
                s = 500.0f;
            m_cfg.gridScale = (int)s;
        }

        // snap when close
        if (fabs(m_cfg.gridScale - FromExp(targetExp)) < 0.01f && fabs(expVel) < 0.01f)
        {
            zoomExp = targetExp;
            expVel = 0.0f;
            m_cfg.gridScale = FromExp(targetExp);
        }

        // GUI panels
        m_gui.ShowMainMenu(m_cfg, m_scene);

        ImVec2 winSize = ImGui::GetIO().DisplaySize;
        // Define plot viewport excluding docked control panel
        const float topHeight = 130.0f;
        const float sideWidth = 280.0f;
        ImVec2 plotPos = ImVec2(0, 0);
        ImVec2 plotSize = winSize;
        if (m_cfg.panelLocation == PANEL_TOP)
        {
            plotPos.y += topHeight;
            plotSize.y -= topHeight;
        }
        else if (m_cfg.panelLocation == PANEL_LEFT)
        {
            plotPos.x += sideWidth;
            plotSize.x -= sideWidth;
        }
        else if (m_cfg.panelLocation == PANEL_RIGHT)
        {
            plotSize.x -= sideWidth;
        }

        ImVec2 center = ImVec2(plotPos.x + plotSize.x * 0.5f, plotPos.y + plotSize.y * 0.5f);
        center.x += (float)m_cfg.panX;
        center.y += (float)m_cfg.panY;

        // Panning with left mouse drag (when UI doesn't capture mouse)
        ImGuiIO &iio = ImGui::GetIO();
        if (iio.MouseDown[ImGuiMouseButton_Left] && !iio.WantCaptureMouse)
        {
            m_cfg.panX += (int)iio.MouseDelta.x;
            m_cfg.panY += (int)iio.MouseDelta.y;
        }

        // Build clear color from config
        auto bg = m_cfg.backgroundColor;
        float clearR = bg.x * bg.w;
        float clearG = bg.y * bg.w;
        float clearB = bg.z * bg.w;
        float clearA = bg.w;

        // Render pass
        m_renderer.BeginFrame(clearR, clearG, clearB, clearA);
        {
            // Background layers (grid, axes)
            m_scene.DrawBackground(plotPos, plotSize, m_cfg);
            // Function curve
            m_scene.DrawFunction(center, plotPos, plotSize, m_cfg);
            // ImGui draw
            m_gui.EndFrame(m_renderer);
        }
        m_renderer.EndFrame();

        glfwSwapBuffers(m_window);
    }

    // Save config on exit
    m_cfg.Save("config.ini");

    // Shutdown
    m_gui.Shutdown();
    m_renderer.Cleanup();
    DestroyMainWindow();
    return 0;
}

void App::OnResize(int w, int h)
{
    m_renderer.OnResize(w, h);
}
