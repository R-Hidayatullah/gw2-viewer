#include "app/inspector/InspectorPanel.h"
#include "app/AppState.h"

#include <imgui.h>
#include <nlohmann/json.hpp>

#include <cstring>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <numeric>
#include <filesystem>
#include <map>
#include <array>

namespace fs = std::filesystem;

// -----------------------------------------------------------
// Entropy calculation helper
// -----------------------------------------------------------
static float ByteEntropy(const std::vector<uint8_t> &data, size_t maxBytes = 65536)
{
    if (data.empty())
        return 0.f;
    size_t n = std::min(data.size(), maxBytes);
    std::array<int, 256> freq{};
    for (size_t i = 0; i < n; ++i)
        freq[data[i]]++;
    float ent = 0.f;
    for (int f : freq)
    {
        if (f == 0)
            continue;
        float p = (float)f / n;
        ent -= p * std::log2(p);
    }
    return ent;
}

namespace panels
{

    InspectorPanel::InspectorPanel(std::shared_ptr<AppState> state)
        : m_State(std::move(state))
    {
    }

    void InspectorPanel::Render()
    {
        ImGui::Begin("Inspector");

        if (!m_State->hasFile && m_State->browserEntries.empty())
        {
            ImGui::TextDisabled("No file selected.");
            ImGui::End();
            return;
        }

        RenderFileInfo();
        ImGui::Separator();
        RenderProperties();
        ImGui::Separator();
        RenderRawStats();

        ImGui::End();
    }

    void InspectorPanel::RenderFileInfo()
    {
        if (m_State->loadedFilePath.empty())
            return;

        fs::path fp(m_State->loadedFilePath);

        // Icon based on extension
        std::string ext = fp.extension().string();
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

        const char *icon = "[BIN]";
        if (ext == ".png" || ext == ".jpg" || ext == ".tga" || ext == ".dds")
            icon = "[IMG]";
        else if (ext == ".mp3" || ext == ".ogg" || ext == ".wav")
            icon = "[AUD]";
        else if (ext == ".json")
            icon = "[JSN]";
        else if (ext == ".txt" || ext == ".xml" || ext == ".lua")
            icon = "[TXT]";
        else if (ext == ".dat")
            icon = "[DAT]";

        ImGui::TextDisabled("%s", icon);
        ImGui::SameLine();
        ImGui::TextWrapped("%s", fp.filename().string().c_str());
        ImGui::Spacing();

        // Preview mode badge
        const char *modeLabels[] = {"HEX", "TXT", "IMG", "AUD", "JSON", "---"};
        int modeIdx = (int)m_State->previewMode;
        if (modeIdx < 0 || modeIdx > 5)
            modeIdx = 5;
        ImGui::TextColored({0.87f, 0.70f, 0.25f, 1.f}, "Mode: %s", modeLabels[modeIdx]);
    }

    void InspectorPanel::RenderProperties()
    {
        ImGui::TextColored({0.87f, 0.70f, 0.25f, 1.f}, "Properties");
        ImGui::Spacing();

        if (m_State->inspectorProps.empty())
        {
            ImGui::TextDisabled("No properties");
            return;
        }

        // Filter
        ImGui::SetNextItemWidth(-1);
        ImGui::InputText("##ipfilter", m_FilterBuf, sizeof(m_FilterBuf));
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Filter properties");
        ImGui::Spacing();

        std::string filt(m_FilterBuf);
        std::transform(filt.begin(), filt.end(), filt.begin(), ::tolower);

        if (ImGui::BeginTable("##PropTable", 2,
                              ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg |
                                  ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_SizingStretchProp))
        {
            ImGui::TableSetupColumn("Key", ImGuiTableColumnFlags_WidthFixed, 90.f);
            ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);

            for (const auto &p : m_State->inspectorProps)
            {
                if (!filt.empty())
                {
                    std::string kl = p.key, vl = p.value;
                    std::transform(kl.begin(), kl.end(), kl.begin(), ::tolower);
                    std::transform(vl.begin(), vl.end(), vl.begin(), ::tolower);
                    if (kl.find(filt) == std::string::npos &&
                        vl.find(filt) == std::string::npos)
                        continue;
                }

                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::TextDisabled("%s", p.key.c_str());
                ImGui::TableSetColumnIndex(1);
                ImGui::TextWrapped("%s", p.value.c_str());

                // Clipboard on right-click value
                if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(1))
                    ImGui::SetClipboardText(p.value.c_str());
            }

            ImGui::EndTable();
        }
    }

    void InspectorPanel::RenderRawStats()
    {
        if (!m_State->hasFile || m_State->rawBytes.empty())
            return;

        ImGui::TextColored({0.87f, 0.70f, 0.25f, 1.f}, "Raw Statistics");
        ImGui::Spacing();

        const auto &b = m_State->rawBytes;

        // Byte distribution mini bar chart (16 buckets of 16 values)
        std::array<int, 16> buckets{};
        for (uint8_t byte : b)
            buckets[byte >> 4]++;
        int maxB = *std::max_element(buckets.begin(), buckets.end());

        ImVec2 chartOrigin = ImGui::GetCursorScreenPos();
        float chartW = ImGui::GetContentRegionAvail().x - 8;
        float chartH = 50.f;
        float barW = chartW / 16.f;

        ImDrawList *dl = ImGui::GetWindowDrawList();
        dl->AddRectFilled(chartOrigin, {chartOrigin.x + chartW, chartOrigin.y + chartH},
                          IM_COL32(18, 18, 22, 255), 2.f);

        for (int i = 0; i < 16; ++i)
        {
            float h = (maxB > 0) ? ((float)buckets[i] / maxB) * (chartH - 2) : 0;
            float x0 = chartOrigin.x + i * barW + 1;
            float y1 = chartOrigin.y + chartH - 1;
            float y0 = y1 - h;
            ImU32 col = IM_COL32(100 + i * 8, 160, 80 + i * 10, 220);
            dl->AddRectFilled({x0, y0}, {x0 + barW - 2, y1}, col, 1.f);
            // Label
            if (barW > 12)
            {
                char lbl[4];
                std::snprintf(lbl, sizeof(lbl), "%X_", i);
                lbl[1] = '\0';
                dl->AddText({x0 + 1, y1 + 2}, IM_COL32(120, 120, 140, 200), lbl);
            }
        }
        ImGui::Dummy({chartW, chartH + 14});
        ImGui::TextDisabled("Byte frequency (buckets 0x0_..0xF_)");
        ImGui::Spacing();

        // Entropy
        float entropy = ByteEntropy(b);
        ImGui::Text("Entropy: %.3f / 8.0", entropy);
        ImGui::SameLine();
        if (entropy > 7.5f)
            ImGui::TextColored({1, 0.4f, 0.4f, 1}, " (likely compressed/encrypted)");
        else if (entropy < 2.f)
            ImGui::TextColored({0.4f, 1, 0.6f, 1}, " (sparse data)");

        // Progress bar for entropy
        ImGui::ProgressBar(entropy / 8.f, {-1, 8});
        ImGui::Spacing();

        // Null byte % and printable %
        size_t nullCount = std::count(b.begin(), b.end(), (uint8_t)0);
        size_t printCount = 0;
        for (uint8_t byte : b)
            if (byte >= 0x20 && byte < 0x7F)
                printCount++;

        ImGui::TextDisabled("Null bytes:    ");
        ImGui::SameLine();
        ImGui::Text("%.1f%%", 100.0 * nullCount / b.size());

        ImGui::TextDisabled("Printable:     ");
        ImGui::SameLine();
        ImGui::Text("%.1f%%", 100.0 * printCount / b.size());

        ImGui::TextDisabled("Total bytes:   ");
        ImGui::SameLine();
        ImGui::Text("%zu", b.size());
    }

} // namespace panels
