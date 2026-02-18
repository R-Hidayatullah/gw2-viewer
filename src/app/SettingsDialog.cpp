#include "app/SettingsDialog.h"
#include "app/AppState.h"

#include <imgui.h>

namespace dialogs {

static const char* k_Themes[] = { "Dark", "Light", "Classic", "GW2 Gold" };

SettingsDialog::SettingsDialog(std::shared_ptr<AppState> state)
    : m_State(std::move(state))
{}

bool SettingsDialog::Render(bool* open)
{
    if (!*open) return false;

    ImGui::SetNextWindowSize({520, 400}, ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(),
                            ImGuiCond_FirstUseEver, {0.5f, 0.5f});

    if (!ImGui::Begin("Preferences", open,
            ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking))
    {
        ImGui::End();
        return *open;
    }

    if (ImGui::BeginTabBar("##SettingsTabs"))
    {
        if (ImGui::BeginTabItem("Appearance")) {
            RenderAppearanceTab();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Preview")) {
            RenderPreviewTab();
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }

    ImGui::Separator();
    ImGui::Spacing();

    // Apply / Cancel / OK buttons
    float btnW = 90.f;
    float totalW = btnW * 3 + ImGui::GetStyle().ItemSpacing.x * 2;
    ImGui::SetCursorPosX(ImGui::GetContentRegionMax().x - totalW);

    if (ImGui::Button("Apply", {btnW, 0})) {
        m_State->themeIndex     = m_ThemeIndex;
        m_State->fontSize       = m_FontSize;
        m_State->showHexAscii   = m_ShowAscii;
        m_State->hexColumns     = m_HexCols;
        m_State->settingsDirty  = true;
    }
    ImGui::SameLine();
    if (ImGui::Button("OK", {btnW, 0})) {
        m_State->themeIndex     = m_ThemeIndex;
        m_State->fontSize       = m_FontSize;
        m_State->showHexAscii   = m_ShowAscii;
        m_State->hexColumns     = m_HexCols;
        m_State->settingsDirty  = true;
        *open = false;
    }
    ImGui::SameLine();
    if (ImGui::Button("Cancel", {btnW, 0})) {
        // Revert local copies
        m_ThemeIndex = m_State->themeIndex;
        m_FontSize   = m_State->fontSize;
        m_ShowAscii  = m_State->showHexAscii;
        m_HexCols    = m_State->hexColumns;
        *open = false;
    }

    ImGui::End();
    return *open;
}

void SettingsDialog::RenderAppearanceTab()
{
    // Sync from state on first open
    if (!m_Dirty) {
        m_ThemeIndex = m_State->themeIndex;
        m_FontSize   = m_State->fontSize;
        m_Dirty      = true;
    }

    ImGui::Spacing();
    ImGui::SeparatorText("Theme");
    ImGui::Spacing();

    // Theme cards
    for (int i = 0; i < 4; ++i)
    {
        bool selected = (m_ThemeIndex == i);
        if (selected) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyle().Colors[ImGuiCol_ButtonActive]);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImGui::GetStyle().Colors[ImGuiCol_ButtonActive]);
        }
        if (ImGui::Button(k_Themes[i], {110, 32}))
            m_ThemeIndex = i;
        if (selected) ImGui::PopStyleColor(2);
        if (i < 3) ImGui::SameLine();
    }

    ImGui::Spacing();
    ImGui::SeparatorText("Font");
    ImGui::Spacing();

    ImGui::TextUnformatted("Font size:");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(80);
    ImGui::SliderFloat("##fontsize", &m_FontSize, 10.f, 24.f, "%.0f px");
    ImGui::SameLine();
    ImGui::TextDisabled("(Requires restart to fully apply large changes)");

    ImGui::Spacing();
    ImGui::SeparatorText("Window");
    ImGui::Spacing();
    ImGui::TextDisabled("Layout is saved automatically to gw2viewer_layout.ini");
    if (ImGui::Button("Reset Layout")) {
        ImGui::LoadIniSettingsFromMemory("", 0); // clear
    }
}

void SettingsDialog::RenderPreviewTab()
{
    // Sync
    if (!m_Dirty) {
        m_ShowAscii = m_State->showHexAscii;
        m_HexCols   = m_State->hexColumns;
        m_Dirty     = true;
    }

    ImGui::Spacing();
    ImGui::SeparatorText("Hex View");
    ImGui::Spacing();

    ImGui::Checkbox("Show ASCII column", &m_ShowAscii);
    ImGui::Spacing();

    ImGui::TextUnformatted("Columns:");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(80);
    ImGui::InputInt("##hcols", &m_HexCols);
    m_HexCols = std::max(1, std::min(64, m_HexCols));
    ImGui::SameLine();
    ImGui::TextDisabled("(1..64)");

    ImGui::Spacing();
    ImGui::SeparatorText("Image View");
    ImGui::Spacing();
    ImGui::TextDisabled("Images are displayed fit-to-window.");
    ImGui::TextDisabled("Supported: PNG, JPG, BMP, TGA, GIF (stb_image).");

    ImGui::Spacing();
    ImGui::SeparatorText("Audio View");
    ImGui::Spacing();
    ImGui::TextDisabled("Waveform preview uses raw bytes.");
    ImGui::TextDisabled("Full playback: integrate miniaudio.");
}

} // namespace dialogs
