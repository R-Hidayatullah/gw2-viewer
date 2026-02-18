#pragma once
#include <string>
#include <memory>
#include <nlohmann/json.hpp>

struct GLFWwindow;

// Forward declarations
namespace panels
{
    class BrowserPanel;
    class PreviewPanel;
    class InspectorPanel;
}
namespace dialogs
{
    class SettingsDialog;
    class AboutDialog;
}
struct AppState;

class Application
{
public:
    Application();
    ~Application();

    bool Init();
    void Run();
    void Shutdown();

private:
    void BeginFrame();
    void EndFrame();
    void RenderMainMenuBar();
    void RenderDockspace();
    void RenderPanels();
    void ApplyTheme(int themeIndex);
    void ApplyFont(float fontSize);

    GLFWwindow *m_Window = nullptr;
    std::shared_ptr<AppState> m_State;
    std::unique_ptr<panels::BrowserPanel> m_BrowserPanel;
    std::unique_ptr<panels::PreviewPanel> m_PreviewPanel;
    std::unique_ptr<panels::InspectorPanel> m_InspectorPanel;
    std::unique_ptr<dialogs::SettingsDialog> m_SettingsDialog;
    std::unique_ptr<dialogs::AboutDialog> m_AboutDialog;

    bool m_ShowSettings = false;
    bool m_ShowAbout = false;
    bool m_Running = true;
};
