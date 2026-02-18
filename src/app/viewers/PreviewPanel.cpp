#include "app/viewers/PreviewPanel.h"
#include "app/AppState.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <glad/glad.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <cstring>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cctype>
#include <filesystem>

namespace panels
{

    PreviewPanel::PreviewPanel(std::shared_ptr<AppState> state)
        : m_State(std::move(state))
    {
    }

    PreviewPanel::~PreviewPanel()
    {
        FreeTexture();
    }

    void PreviewPanel::FreeTexture()
    {
        if (m_TexId)
        {
            glDeleteTextures(1, &m_TexId);
            m_TexId = 0;
            m_TexW = 0;
            m_TexH = 0;
            m_TexSource.clear();
        }
    }

    void PreviewPanel::LoadTextureFromBytes(const std::vector<uint8_t> &bytes)
    {
        if (bytes.empty())
            return;
        if (m_TexSource == m_State->loadedFilePath)
            return; // already loaded

        FreeTexture();

        int w, h, ch;
        unsigned char *data = stbi_load_from_memory(
            bytes.data(), (int)bytes.size(), &w, &h, &ch, 4);
        if (!data)
            return;

        glGenTextures(1, &m_TexId);
        glBindTexture(GL_TEXTURE_2D, m_TexId);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        stbi_image_free(data);

        m_TexW = w;
        m_TexH = h;
        m_TexSource = m_State->loadedFilePath;
    }

    // -----------------------------------------------------------

    void PreviewPanel::Render()
    {
        ImGui::Begin("Preview");
        RenderModeSelector();
        ImGui::Separator();

        if (!m_State->hasFile)
        {
            RenderEmptyState();
            ImGui::End();
            return;
        }

        switch (m_State->previewMode)
        {
        case PreviewMode::Hex:
            RenderHexView();
            break;
        case PreviewMode::Text:
            RenderTextView();
            break;
        case PreviewMode::Image:
            RenderImageView();
            break;
        case PreviewMode::Audio:
            RenderAudioView();
            break;
        case PreviewMode::Json:
            if (m_State->hasJson)
            {
                ImGui::SetNextItemWidth(200);
                ImGui::InputText("Search##jsonsearch", m_JsonSearch, sizeof(m_JsonSearch));
                ImGui::Separator();
                ImGui::BeginChild("##JsonScroll", {0, 0}, false);
                RenderJsonView(m_State->jsonData);
                ImGui::EndChild();
            }
            else
            {
                ImGui::TextColored({1, 0.5f, 0, 1}, "No JSON data loaded.");
            }
            break;
        default:
            RenderHexView();
            break;
        }

        ImGui::End();
    }

    void PreviewPanel::RenderModeSelector()
    {
        // Tab bar for switching preview mode
        static const char *labels[] = {"Hex", "Text", "Image", "Audio", "JSON"};
        static PreviewMode modes[] = {
            PreviewMode::Hex, PreviewMode::Text,
            PreviewMode::Image, PreviewMode::Audio, PreviewMode::Json};
        for (int i = 0; i < 5; ++i)
        {
            bool active = (m_State->previewMode == modes[i]);
            if (active)
            {
                ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyle().Colors[ImGuiCol_ButtonActive]);
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImGui::GetStyle().Colors[ImGuiCol_ButtonActive]);
            }
            if (ImGui::SmallButton(labels[i]))
                m_State->previewMode = modes[i];
            if (active)
                ImGui::PopStyleColor(2);
            if (i < 4)
                ImGui::SameLine();
        }

        // Show filename right-aligned
        if (!m_State->loadedFilePath.empty())
        {
            std::filesystem::path p(m_State->loadedFilePath);
            std::string fn = p.filename().string();
            float x = ImGui::GetContentRegionMax().x - ImGui::CalcTextSize(fn.c_str()).x - 4;
            if (x > ImGui::GetCursorPosX())
            {
                ImGui::SameLine(x);
                ImGui::TextDisabled("%s", fn.c_str());
            }
        }
    }

    void PreviewPanel::RenderEmptyState()
    {
        ImVec2 avail = ImGui::GetContentRegionAvail();
        ImVec2 center = {
            ImGui::GetCursorScreenPos().x + avail.x * 0.5f,
            ImGui::GetCursorScreenPos().y + avail.y * 0.5f};

        const char *msg1 = "No file loaded";
        const char *msg2 = "Use File > Open File or double-click a browser entry";
        ImGui::SetCursorScreenPos({center.x - ImGui::CalcTextSize(msg1).x * 0.5f,
                                   center.y - 20});
        ImGui::TextDisabled("%s", msg1);
        ImGui::SetCursorScreenPos({center.x - ImGui::CalcTextSize(msg2).x * 0.5f,
                                   center.y});
        ImGui::TextDisabled("%s", msg2);
    }

    // -----------------------------------------------------------
    // Hex view
    // -----------------------------------------------------------
    void PreviewPanel::RenderHexView()
    {
        const auto &bytes = m_State->rawBytes;
        const int cols = m_State->hexColumns;
        const bool ascii = m_State->showHexAscii;

        if (bytes.empty())
        {
            ImGui::TextDisabled("(empty file)");
            return;
        }

        // Header controls
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, {4, 2});
        ImGui::TextDisabled("Cols:");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(50);
        int tmpCols = cols;
        if (ImGui::InputInt("##hcols", &tmpCols, 0, 0))
        {
            m_State->hexColumns = std::max(1, std::min(64, tmpCols));
        }
        ImGui::SameLine(0, 16);
        bool tmpAscii = ascii;
        ImGui::Checkbox("ASCII", &tmpAscii);
        m_State->showHexAscii = tmpAscii;
        ImGui::SameLine(0, 16);
        ImGui::TextDisabled("0x%zX bytes (%zu)", bytes.size(), bytes.size());
        ImGui::PopStyleVar();
        ImGui::Separator();

        // Monospace font size
        float charW = ImGui::CalcTextSize("FF").x * 0.55f;
        (void)charW;

        ImGui::BeginChild("##HexView", {0, 0}, false, ImGuiWindowFlags_HorizontalScrollbar);

        // Estimate row height
        float rowH = ImGui::GetTextLineHeightWithSpacing();

        // Column header
        ImGui::TextDisabled("Offset   ");
        for (int c = 0; c < cols; ++c)
        {
            ImGui::SameLine(0, 2);
            ImGui::TextDisabled("%02X", c);
        }
        if (ascii)
        {
            ImGui::SameLine(0, 8);
            ImGui::TextDisabled("ASCII");
        }

        size_t totalRows = (bytes.size() + cols - 1) / cols;

        ImGuiListClipper clipper;
        clipper.Begin((int)totalRows);
        while (clipper.Step())
        {
            for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; ++row)
            {
                size_t baseOff = (size_t)row * cols;

                // Offset
                ImGui::TextDisabled("%08zX ", baseOff);

                for (int c = 0; c < cols; ++c)
                {
                    size_t off = baseOff + c;
                    ImGui::SameLine(0, 2);
                    if (off < bytes.size())
                    {
                        uint8_t b = bytes[off];
                        // Colour coding
                        if (b == 0x00)
                            ImGui::TextDisabled("00");
                        else if (b >= 0x20 && b < 0x7F)
                            ImGui::TextColored({0.6f, 0.9f, 0.6f, 1.f}, "%02X", b);
                        else
                            ImGui::Text("%02X", b);
                    }
                    else
                    {
                        ImGui::TextDisabled("  ");
                    }
                }

                if (ascii)
                {
                    ImGui::SameLine(0, 8);
                    std::string asc;
                    for (int c = 0; c < cols; ++c)
                    {
                        size_t off = baseOff + c;
                        if (off < bytes.size())
                        {
                            uint8_t b = bytes[off];
                            asc += (b >= 0x20 && b < 0x7F) ? (char)b : '.';
                        }
                    }
                    ImGui::TextDisabled("%s", asc.c_str());
                }
            }
        }
        clipper.End();

        ImGui::EndChild();
    }

    // -----------------------------------------------------------
    // Text view
    // -----------------------------------------------------------
    void PreviewPanel::RenderTextView()
    {
        const auto &bytes = m_State->rawBytes;
        if (bytes.empty())
        {
            ImGui::TextDisabled("(empty)");
            return;
        }

        // Limit display
        constexpr size_t MAX_CHARS = 512 * 1024;
        size_t showLen = std::min(bytes.size(), MAX_CHARS);

        if (bytes.size() > MAX_CHARS)
        {
            ImGui::TextColored({1, 0.8f, 0, 1},
                               "Large file - showing first %zu KB", MAX_CHARS / 1024);
            ImGui::Separator();
        }

        std::string text(reinterpret_cast<const char *>(bytes.data()), showLen);

        ImGui::BeginChild("##TextView", {0, 0}, false);
        ImGui::InputTextMultiline("##tv", text.data(), text.size() + 1,
                                  {-1, -1},
                                  ImGuiInputTextFlags_ReadOnly);
        ImGui::EndChild();
    }

    // -----------------------------------------------------------
    // Image view
    // -----------------------------------------------------------
    void PreviewPanel::RenderImageView()
    {
        if (m_State->rawBytes.empty())
        {
            ImGui::TextDisabled("No image data");
            return;
        }

        LoadTextureFromBytes(m_State->rawBytes);

        if (!m_TexId)
        {
            ImGui::TextColored({1, 0.4f, 0.4f, 1},
                               "Cannot decode image (unsupported format or corrupt data)");
            return;
        }

        ImGui::TextDisabled("Dimensions: %d x %d px", m_TexW, m_TexH);
        ImGui::Separator();

        ImVec2 avail = ImGui::GetContentRegionAvail();
        avail.y -= 4;

        // Fit image to available space
        float scaleX = avail.x / (float)m_TexW;
        float scaleY = avail.y / (float)m_TexH;
        float scale = std::min(scaleX, scaleY);
        scale = std::min(scale, 1.0f); // don't upscale beyond natural size

        ImVec2 imgSz = {(float)m_TexW * scale, (float)m_TexH * scale};

        // Center
        float offX = (avail.x - imgSz.x) * 0.5f;
        float offY = (avail.y - imgSz.y) * 0.5f;
        if (offX > 0)
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offX);
        if (offY > 0)
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + offY);

        ImGui::Image((ImTextureID)(intptr_t)m_TexId, imgSz);

        if (ImGui::IsItemHovered())
        {
            ImGui::BeginTooltip();
            ImGui::TextDisabled("Click and drag to pan | Scroll to zoom");
            ImGui::EndTooltip();
        }
    }

    // -----------------------------------------------------------
    // Audio view (waveform placeholder + playback controls)
    // -----------------------------------------------------------
    void PreviewPanel::RenderAudioView()
    {
        const auto &bytes = m_State->rawBytes;
        ImGui::TextDisabled("Audio file  |  %zu bytes", bytes.size());
        ImGui::Separator();

        // Waveform placeholder (draw a mock wave using ImDrawList)
        ImVec2 avail = ImGui::GetContentRegionAvail();
        ImVec2 wavePos = ImGui::GetCursorScreenPos();
        float waveH = std::min(avail.y * 0.4f, 120.f);
        ImVec2 waveSz = {avail.x - 8, waveH};

        ImDrawList *dl = ImGui::GetWindowDrawList();
        dl->AddRectFilled(wavePos, {wavePos.x + waveSz.x, wavePos.y + waveSz.y},
                          IM_COL32(25, 25, 30, 255), 4.f);
        dl->AddRect(wavePos, {wavePos.x + waveSz.x, wavePos.y + waveSz.y},
                    IM_COL32(80, 80, 100, 200), 4.f);

        // Synthesise a fake waveform from byte data
        float midY = wavePos.y + waveSz.y * 0.5f;
        int pts = (int)waveSz.x;
        for (int i = 1; i < pts; ++i)
        {
            size_t idx0 = (size_t)(i - 1) * bytes.size() / pts;
            size_t idx1 = (size_t)i * bytes.size() / pts;
            idx0 = std::min(idx0, bytes.size() - 1);
            idx1 = std::min(idx1, bytes.size() - 1);

            float v0 = (bytes[idx0] / 255.f) * 2.f - 1.f;
            float v1 = (bytes[idx1] / 255.f) * 2.f - 1.f;
            float amp = waveSz.y * 0.45f;

            dl->AddLine(
                {wavePos.x + i - 1, midY + v0 * amp},
                {wavePos.x + i, midY + v1 * amp},
                IM_COL32(100, 200, 120, 220), 1.2f);
        }
        // Centre line
        dl->AddLine({wavePos.x, midY}, {wavePos.x + waveSz.x, midY},
                    IM_COL32(60, 60, 80, 180));

        ImGui::Dummy(waveSz);
        ImGui::Spacing();

        // Playback controls
        ImGui::Spacing();
        float btnW = 36.f;
        float totalBtns = btnW * 4 + ImGui::GetStyle().ItemSpacing.x * 3;
        ImGui::SetCursorPosX((avail.x - totalBtns) * 0.5f);

        // Rewind
        if (ImGui::Button("|<##rw", {btnW, 0}))
            m_State->audioPlaying = false;
        ImGui::SameLine();

        // Play/Pause
        const char *ppLabel = m_State->audioPlaying ? "||##pp" : ">##pp";
        if (ImGui::Button(ppLabel, {btnW, 0}))
            m_State->audioPlaying = !m_State->audioPlaying;
        ImGui::SameLine();

        // Stop
        if (ImGui::Button("[]##st", {btnW, 0}))
            m_State->audioPlaying = false;
        ImGui::SameLine();

        // Fast-forward
        if (ImGui::Button(">|##ff", {btnW, 0}))
            m_State->audioPlaying = false;

        ImGui::Spacing();
        // Progress bar placeholder
        float progress = 0.f;
        ImGui::SetNextItemWidth(-1);
        ImGui::SliderFloat("##progress", &progress, 0.f, 1.f, "00:00 / 00:00");

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::TextDisabled("Audio playback requires miniaudio integration.");
        ImGui::TextDisabled("Waveform is a preview derived from raw byte data.");
    }

    // -----------------------------------------------------------
    // JSON tree view
    // -----------------------------------------------------------
    static bool JsonMatchesSearch(const nlohmann::json &node, const std::string &q)
    {
        if (q.empty())
            return true;
        std::ostringstream ss;
        ss << node;
        std::string s = ss.str();
        return s.find(q) != std::string::npos;
    }

    void PreviewPanel::RenderJsonView(const nlohmann::json &node, int depth)
    {
        std::string qStr(m_JsonSearch);

        auto renderValue = [&](const std::string &key, const nlohmann::json &val)
        {
            if (val.is_object() || val.is_array())
            {
                bool open = ImGui::TreeNodeEx(key.c_str(),
                                              ImGuiTreeNodeFlags_SpanFullWidth);
                if (open)
                {
                    RenderJsonView(val, depth + 1);
                    ImGui::TreePop();
                }
            }
            else
            {
                std::string valStr;
                if (val.is_string())
                    valStr = val.get<std::string>();
                else if (val.is_boolean())
                    valStr = val.get<bool>() ? "true" : "false";
                else if (val.is_null())
                    valStr = "null";
                else
                    valStr = val.dump();

                // Highlight match
                bool match = !qStr.empty() &&
                             (key.find(qStr) != std::string::npos ||
                              valStr.find(qStr) != std::string::npos);

                if (match)
                    ImGui::PushStyleColor(ImGuiCol_Text, {1.f, 0.85f, 0.3f, 1.f});

                ImGui::TreeNodeEx(key.c_str(),
                                  ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen |
                                      ImGuiTreeNodeFlags_SpanFullWidth);
                ImGui::SameLine();
                ImGui::TextDisabled(": ");
                ImGui::SameLine();

                if (val.is_string())
                    ImGui::TextColored({0.6f, 0.9f, 0.6f, 1.f}, "\"%s\"", valStr.c_str());
                else if (val.is_number())
                    ImGui::TextColored({0.6f, 0.8f, 1.0f, 1.f}, "%s", valStr.c_str());
                else if (val.is_boolean())
                    ImGui::TextColored({1.f, 0.7f, 0.4f, 1.f}, "%s", valStr.c_str());
                else
                    ImGui::TextDisabled("%s", valStr.c_str());

                if (match)
                    ImGui::PopStyleColor();
            }
        };

        if (node.is_object())
        {
            for (auto &[k, v] : node.items())
                renderValue(k, v);
        }
        else if (node.is_array())
        {
            for (size_t i = 0; i < node.size(); ++i)
                renderValue(std::string("[") + std::to_string(i) + "]", node[i]);
        }
    }

} // namespace panels
