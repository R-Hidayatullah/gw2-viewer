#include "app/AboutDialog.h"

#include <imgui.h>

namespace dialogs {

void AboutDialog::Render(bool* open)
{
    if (!*open) return;

    ImGui::SetNextWindowSize({480, 340}, ImGuiCond_Always);
    ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(),
                            ImGuiCond_Always, {0.5f, 0.5f});

    ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoMove;

    if (!ImGui::Begin("About GW2 Asset Viewer", open, flags)) {
        ImGui::End();
        return;
    }

    // Title
    float cx = ImGui::GetContentRegionAvail().x;
    auto  centeredText = [&](const char* txt, ImVec4 col = {1,1,1,1}) {
        float tw = ImGui::CalcTextSize(txt).x;
        ImGui::SetCursorPosX((cx - tw) * 0.5f);
        ImGui::TextColored(col, "%s", txt);
    };

    ImGui::Spacing();
    centeredText("GW2 Asset Viewer", {0.87f, 0.70f, 0.25f, 1.f});
    centeredText("Version 0.1.0");
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::TextWrapped(
        "A standalone viewer for Guild Wars 2 asset files (.dat), supporting "
        "hex/binary inspection, image preview, audio waveform, and JSON parsing.");

    ImGui::Spacing();
    ImGui::SeparatorText("Technology Stack");
    ImGui::Spacing();

    struct LibInfo { const char* name; const char* desc; };
    static LibInfo libs[] = {
        { "ImGui (Docking)",  "Immediate-mode GUI"       },
        { "GLAD 3.3 Core",    "OpenGL loader"            },
        { "GLFW 3.4",         "Window & input"           },
        { "GLM 1.0.3",        "Math library"             },
        { "stb_image",        "Image decoding"           },
        { "nlohmann/json",    "JSON parsing"             },
        { "dr_libs",          "Audio file decoding"      },
        { "miniaudio",        "Audio playback"           },
        { "half (7.11)",      "Half-float support"       },
    };

    if (ImGui::BeginTable("##LibTable", 2,
        ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerV))
    {
        ImGui::TableSetupColumn("Library",     ImGuiTableColumnFlags_WidthFixed, 150.f);
        ImGui::TableSetupColumn("Description", ImGuiTableColumnFlags_WidthStretch);
        for (auto& l : libs) {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::TextColored({0.6f,0.8f,1.f,1.f}, "%s", l.name);
            ImGui::TableSetColumnIndex(1);
            ImGui::TextDisabled("%s", l.desc);
        }
        ImGui::EndTable();
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    centeredText("Built with C++20  |  OpenGL 3.3 Core", {0.5f,0.5f,0.55f,1.f});

    ImGui::Spacing();
    float btnW = 80.f;
    ImGui::SetCursorPosX((cx - btnW) * 0.5f);
    if (ImGui::Button("Close", {btnW, 0}))
        *open = false;

    ImGui::End();
}

} // namespace dialogs
