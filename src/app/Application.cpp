#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commdlg.h>
#endif

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

#include "app/Application.h"
#include "app/AppState.h"
#include "app/browser/BrowserPanel.h"
#include "app/viewers/PreviewPanel.h"
#include "app/inspector/InspectorPanel.h"
#include "app/SettingsDialog.h"
#include "app/AboutDialog.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <imgui.h>
#include <imgui_internal.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

#include <nlohmann/json.hpp>

#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <filesystem>

namespace fs = std::filesystem;

// -----------------------------------------------------------
// Utility: portable open-file dialog using system calls
// -----------------------------------------------------------
static std::string OpenFileDialog(const char *filter = nullptr)
{
#ifdef _WIN32
    // Use WinAPI common dialog
    char buf[MAX_PATH] = {};
    OPENFILENAMEA ofn{};
    ofn.lStructSize = sizeof(ofn);
    ofn.lpstrFilter = filter ? filter : "All Files\0*.*\0";
    ofn.lpstrFile = buf;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
    if (GetOpenFileNameA(&ofn))
        return buf;
#endif
    return {};
}

// -----------------------------------------------------------
// GW2 custom theme
// -----------------------------------------------------------
static void ApplyGW2Theme()
{
    ImGuiStyle &s = ImGui::GetStyle();
    ImVec4 *c = s.Colors;

    s.WindowRounding = 4.0f;
    s.FrameRounding = 3.0f;
    s.ScrollbarRounding = 3.0f;
    s.GrabRounding = 3.0f;
    s.TabRounding = 3.0f;
    s.FramePadding = {6, 4};
    s.ItemSpacing = {8, 4};

    // GW2 colour palette: deep charcoal + gold accent
    ImVec4 bg0 = {0.09f, 0.09f, 0.10f, 1.f};
    ImVec4 bg1 = {0.13f, 0.13f, 0.14f, 1.f};
    ImVec4 bg2 = {0.18f, 0.18f, 0.20f, 1.f};
    ImVec4 bg3 = {0.24f, 0.24f, 0.27f, 1.f};
    ImVec4 gold = {0.87f, 0.70f, 0.25f, 1.f};
    ImVec4 goldH = {1.00f, 0.82f, 0.30f, 1.f};
    ImVec4 goldA = {0.87f, 0.70f, 0.25f, 0.40f};
    ImVec4 txt = {0.92f, 0.90f, 0.85f, 1.f};
    ImVec4 txtD = {0.55f, 0.53f, 0.50f, 1.f};
    ImVec4 red = {0.80f, 0.20f, 0.20f, 1.f};

    c[ImGuiCol_Text] = txt;
    c[ImGuiCol_TextDisabled] = txtD;
    c[ImGuiCol_WindowBg] = bg0;
    c[ImGuiCol_ChildBg] = bg1;
    c[ImGuiCol_PopupBg] = bg1;
    c[ImGuiCol_Border] = bg3;
    c[ImGuiCol_BorderShadow] = {0, 0, 0, 0};
    c[ImGuiCol_FrameBg] = bg2;
    c[ImGuiCol_FrameBgHovered] = bg3;
    c[ImGuiCol_FrameBgActive] = goldA;
    c[ImGuiCol_TitleBg] = bg0;
    c[ImGuiCol_TitleBgActive] = {0.16f, 0.10f, 0.04f, 1.f};
    c[ImGuiCol_TitleBgCollapsed] = bg0;
    c[ImGuiCol_MenuBarBg] = bg1;
    c[ImGuiCol_ScrollbarBg] = bg0;
    c[ImGuiCol_ScrollbarGrab] = bg3;
    c[ImGuiCol_ScrollbarGrabHovered] = gold;
    c[ImGuiCol_ScrollbarGrabActive] = goldH;
    c[ImGuiCol_CheckMark] = gold;
    c[ImGuiCol_SliderGrab] = gold;
    c[ImGuiCol_SliderGrabActive] = goldH;
    c[ImGuiCol_Button] = bg2;
    c[ImGuiCol_ButtonHovered] = bg3;
    c[ImGuiCol_ButtonActive] = goldA;
    c[ImGuiCol_Header] = goldA;
    c[ImGuiCol_HeaderHovered] = {0.87f, 0.70f, 0.25f, 0.55f};
    c[ImGuiCol_HeaderActive] = gold;
    c[ImGuiCol_Separator] = bg3;
    c[ImGuiCol_SeparatorHovered] = gold;
    c[ImGuiCol_SeparatorActive] = goldH;
    c[ImGuiCol_ResizeGrip] = goldA;
    c[ImGuiCol_ResizeGripHovered] = gold;
    c[ImGuiCol_ResizeGripActive] = goldH;
    c[ImGuiCol_Tab] = bg2;
    c[ImGuiCol_TabHovered] = bg3;
    c[ImGuiCol_TabActive] = {0.25f, 0.18f, 0.05f, 1.f};
    c[ImGuiCol_TabUnfocused] = bg1;
    c[ImGuiCol_TabUnfocusedActive] = bg2;
    c[ImGuiCol_DockingPreview] = goldA;
    c[ImGuiCol_DockingEmptyBg] = bg0;
    c[ImGuiCol_PlotLines] = gold;
    c[ImGuiCol_PlotLinesHovered] = goldH;
    c[ImGuiCol_PlotHistogram] = gold;
    c[ImGuiCol_PlotHistogramHovered] = goldH;
    c[ImGuiCol_TableHeaderBg] = bg2;
    c[ImGuiCol_TableBorderStrong] = bg3;
    c[ImGuiCol_TableBorderLight] = bg2;
    c[ImGuiCol_TableRowBg] = {0, 0, 0, 0};
    c[ImGuiCol_TableRowBgAlt] = {1, 1, 1, 0.03f};
    c[ImGuiCol_TextSelectedBg] = goldA;
    c[ImGuiCol_DragDropTarget] = gold;
    c[ImGuiCol_NavHighlight] = gold;
    c[ImGuiCol_NavWindowingHighlight] = {1, 1, 1, 0.7f};
    c[ImGuiCol_NavWindowingDimBg] = {0.8f, 0.8f, 0.8f, 0.2f};
    c[ImGuiCol_ModalWindowDimBg] = {0, 0, 0, 0.5f};
}

// ===========================================================
// Application
// ===========================================================

Application::Application() = default;
Application::~Application() = default;

bool Application::Init()
{
    if (!glfwInit())
    {
        std::cerr << "GLFW init failed\n";
        return false;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);

    m_Window = glfwCreateWindow(1440, 900, "GW2 Asset Viewer", nullptr, nullptr);
    if (!m_Window)
    {
        std::cerr << "Window creation failed\n";
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(m_Window);
    glfwSwapInterval(1);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "GLAD init failed\n";
        return false;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.IniFilename = "gw2viewer_layout.ini";

    // Font
    io.Fonts->AddFontFromFileTTF("assets/fonts/Roboto-Regular.ttf", 14.0f);
    if (io.Fonts->Fonts.empty())
        io.Fonts->AddFontDefault();

    ImGui_ImplGlfw_InitForOpenGL(m_Window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // Shared state
    m_State = std::make_shared<AppState>();

    // Panels
    m_BrowserPanel = std::make_unique<panels::BrowserPanel>(m_State);
    m_PreviewPanel = std::make_unique<panels::PreviewPanel>(m_State);
    m_InspectorPanel = std::make_unique<panels::InspectorPanel>(m_State);
    m_SettingsDialog = std::make_unique<dialogs::SettingsDialog>(m_State);
    m_AboutDialog = std::make_unique<dialogs::AboutDialog>();

    ApplyTheme(m_State->themeIndex);

    return true;
}

void Application::ApplyTheme(int themeIndex)
{
    switch (themeIndex)
    {
    case 0:
        ImGui::StyleColorsDark();
        break;
    case 1:
        ImGui::StyleColorsLight();
        break;
    case 2:
        ImGui::StyleColorsClassic();
        break;
    case 3:
        ApplyGW2Theme();
        break;
    default:
        ImGui::StyleColorsDark();
        break;
    }
}

void Application::ApplyFont(float fontSize)
{
    ImGuiIO &io = ImGui::GetIO();

    io.Fonts->Clear();
    io.Fonts->AddFontFromFileTTF("assets/fonts/Roboto-Regular.ttf", fontSize);
    if (io.Fonts->Fonts.empty())
        io.Fonts->AddFontDefault();

    io.Fonts->Build();

    // Recreate backend device objects
    ImGui_ImplOpenGL3_DestroyDeviceObjects();
    ImGui_ImplOpenGL3_CreateDeviceObjects();
}

void Application::Run()
{
    while (!glfwWindowShouldClose(m_Window) && m_Running)
    {
        glfwPollEvents();
        BeginFrame();

        // Apply deferred settings
        if (m_State->settingsDirty)
        {
            ApplyTheme(m_State->themeIndex);
            ApplyFont(m_State->fontSize);
            m_State->settingsDirty = false;
        }

        RenderDockspace();
        RenderMainMenuBar();
        RenderPanels();

        if (m_ShowSettings)
            m_SettingsDialog->Render(&m_ShowSettings);
        if (m_ShowAbout)
            m_AboutDialog->Render(&m_ShowAbout);

        EndFrame();
    }
}

void Application::BeginFrame()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void Application::EndFrame()
{
    ImGui::Render();
    int w, h;
    glfwGetFramebufferSize(m_Window, &w, &h);
    glViewport(0, 0, w, h);
    glClearColor(0.09f, 0.09f, 0.10f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glfwSwapBuffers(m_Window);
}

void Application::RenderDockspace()
{
    // Full-screen invisible dockspace window
    ImGuiViewport *vp = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(vp->WorkPos);
    ImGui::SetNextWindowSize(vp->WorkSize);
    ImGui::SetNextWindowViewport(vp->ID);

    ImGuiWindowFlags host_flags =
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus |
        ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0, 0});
    ImGui::Begin("##DockHost", nullptr, host_flags);
    ImGui::PopStyleVar(3);

    ImGuiID dockId = ImGui::GetID("MainDockSpace");
    if (!ImGui::DockBuilderGetNode(dockId))
    {
        // First-time layout
        ImGui::DockBuilderRemoveNode(dockId);
        ImGui::DockBuilderAddNode(dockId, ImGuiDockNodeFlags_DockSpace);
        ImGui::DockBuilderSetNodeSize(dockId, ImGui::GetContentRegionAvail());

        ImGuiID left, center, right;
        ImGui::DockBuilderSplitNode(dockId, ImGuiDir_Left, 0.22f, &left, &center);
        ImGui::DockBuilderSplitNode(center, ImGuiDir_Right, 0.30f, &right, &center);

        ImGui::DockBuilderDockWindow("Browser", left);
        ImGui::DockBuilderDockWindow("Preview", center);
        ImGui::DockBuilderDockWindow("Inspector", right);
        ImGui::DockBuilderFinish(dockId);
    }

    ImGui::DockSpace(dockId, {0, 0}, ImGuiDockNodeFlags_PassthruCentralNode);
    ImGui::End();
}

void Application::RenderMainMenuBar()
{
    if (!ImGui::BeginMainMenuBar())
        return;

    // ---- File ----
    if (ImGui::BeginMenu("File"))
    {
        if (ImGui::MenuItem("Open File...", "Ctrl+O"))
        {
            std::string path = OpenFileDialog(
                "All Files\0*.*\0"
                "DAT Archive\0*.dat\0"
                "JSON\0*.json\0"
                "Binary\0*.bin\0");
            if (!path.empty())
            {
                m_State->ClearFile();
                m_State->loadedFilePath = path;

                // Read raw bytes
                std::ifstream f(path, std::ios::binary);
                if (f)
                {
                    f.seekg(0, std::ios::end);
                    size_t sz = f.tellg();
                    f.seekg(0);
                    m_State->rawBytes.resize(sz);
                    f.read(reinterpret_cast<char *>(m_State->rawBytes.data()), sz);
                    m_State->hasFile = true;
                }

                // Try parse as JSON
                if (path.size() >= 5 &&
                    path.substr(path.size() - 5) == ".json")
                {
                    try
                    {
                        std::ifstream jf(path);
                        jf >> m_State->jsonData;
                        m_State->hasJson = true;
                        m_State->previewMode = PreviewMode::Json;
                    }
                    catch (...)
                    {
                    }
                }

                // Populate browser with parent directory
                fs::path fp(path);
                if (fp.has_parent_path())
                {
                    m_State->browserRoot = fp.parent_path().string();
                    m_State->browserEntries.clear();
                    for (auto &e : fs::directory_iterator(fp.parent_path()))
                    {
                        FileEntry fe;
                        fe.name = e.path().filename().string();
                        fe.path = e.path().string();
                        fe.isDir = e.is_directory();
                        fe.size = e.is_regular_file() ? e.file_size() : 0;
                        m_State->browserEntries.push_back(fe);
                    }
                    std::sort(m_State->browserEntries.begin(),
                              m_State->browserEntries.end(),
                              [](const FileEntry &a, const FileEntry &b)
                              {
                                  if (a.isDir != b.isDir)
                                      return a.isDir > b.isDir;
                                  return a.name < b.name;
                              });
                }
            }
        }

        if (ImGui::MenuItem("Load JSON...", "Ctrl+J"))
        {
            std::string path = OpenFileDialog("JSON Files\0*.json\0All\0*.*\0");
            if (!path.empty())
            {
                try
                {
                    std::ifstream jf(path);
                    jf >> m_State->jsonData;
                    m_State->hasJson = true;
                    m_State->loadedFilePath = path;
                    m_State->previewMode = PreviewMode::Json;

                    // Also load raw bytes
                    std::ifstream f(path, std::ios::binary);
                    f.seekg(0, std::ios::end);
                    size_t sz = f.tellg();
                    f.seekg(0);
                    m_State->rawBytes.resize(sz);
                    f.read(reinterpret_cast<char *>(m_State->rawBytes.data()), sz);
                    m_State->hasFile = true;
                }
                catch (const std::exception &ex)
                {
                    // TODO: show error toast
                    (void)ex;
                }
            }
        }

        ImGui::Separator();
        if (ImGui::MenuItem("Close File"))
            m_State->ClearFile();

        ImGui::Separator();
        if (ImGui::MenuItem("Exit", "Alt+F4"))
            m_Running = false;

        ImGui::EndMenu();
    }

    // ---- View ----
    if (ImGui::BeginMenu("View"))
    {
        if (ImGui::MenuItem("Hex View"))
            m_State->previewMode = PreviewMode::Hex;
        if (ImGui::MenuItem("Text View"))
            m_State->previewMode = PreviewMode::Text;
        if (ImGui::MenuItem("Image View"))
            m_State->previewMode = PreviewMode::Image;
        if (ImGui::MenuItem("Audio View"))
            m_State->previewMode = PreviewMode::Audio;
        if (ImGui::MenuItem("JSON View"))
            m_State->previewMode = PreviewMode::Json;
        ImGui::EndMenu();
    }

    // ---- Settings ----
    if (ImGui::BeginMenu("Settings"))
    {
        if (ImGui::MenuItem("Preferences..."))
            m_ShowSettings = true;
        ImGui::EndMenu();
    }

    // ---- Help ----
    if (ImGui::BeginMenu("Help"))
    {
        if (ImGui::MenuItem("About GW2 Viewer"))
            m_ShowAbout = true;
        ImGui::EndMenu();
    }

    // Right-aligned status
    {
        std::string status;
        if (m_State->hasFile)
        {
            fs::path p(m_State->loadedFilePath);
            status = "  " + p.filename().string() +
                     "  |  " + std::to_string(m_State->rawBytes.size()) + " bytes";
        }
        else
        {
            status = "  No file loaded";
        }
        float sw = ImGui::CalcTextSize(status.c_str()).x + 16.f;
        ImGui::SetCursorPosX(ImGui::GetContentRegionMax().x - sw);
        ImGui::TextDisabled("%s", status.c_str());
    }

    ImGui::EndMainMenuBar();
}

void Application::RenderPanels()
{
    m_BrowserPanel->Render();
    m_PreviewPanel->Render();
    m_InspectorPanel->Render();
}

void Application::Shutdown()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(m_Window);
    glfwTerminate();
}
