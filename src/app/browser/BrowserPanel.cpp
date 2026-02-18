#include "app/browser/BrowserPanel.h"
#include "app/AppState.h"

#include <imgui.h>
#include <nlohmann/json.hpp>

#include <fstream>
#include <filesystem>
#include <algorithm>
#include <sstream>
#include <cstring>

namespace fs = std::filesystem;

// File-type icons (unicode symbols rendered as ASCII fallback)
static const char *GetFileIcon(const FileEntry &e)
{
    if (e.isDir)
        return "[DIR]";
    std::string n = e.name;
    auto ext = [&](const char *x)
    {
        return n.size() > strlen(x) &&
               n.substr(n.size() - strlen(x)) == x;
    };
    if (ext(".png") || ext(".jpg") || ext(".tga") || ext(".dds"))
        return "[IMG]";
    if (ext(".mp3") || ext(".ogg") || ext(".wav"))
        return "[AUD]";
    if (ext(".json"))
        return "[JSN]";
    if (ext(".txt") || ext(".xml") || ext(".csv"))
        return "[TXT]";
    if (ext(".dat"))
        return "[DAT]";
    return "[BIN]";
}

static std::string FormatSize(uint64_t bytes)
{
    if (bytes < 1024)
        return std::to_string(bytes) + " B";
    if (bytes < 1024 * 1024)
        return std::to_string(bytes / 1024) + " KB";
    return std::to_string(bytes / (1024 * 1024)) + " MB";
}

namespace panels
{

    BrowserPanel::BrowserPanel(std::shared_ptr<AppState> state)
        : m_State(std::move(state))
    {
    }

    void BrowserPanel::Render()
    {
        ImGui::Begin("Browser");

        RenderToolbar();
        ImGui::Separator();
        RenderEntryList();

        ImGui::End();
    }

    void BrowserPanel::RenderToolbar()
    {
        // Breadcrumb / root path
        if (!m_State->browserRoot.empty())
        {
            ImGui::TextDisabled("Root:");
            ImGui::SameLine();
            ImGui::TextWrapped("%s", m_State->browserRoot.c_str());
        }
        else
        {
            ImGui::TextDisabled("No directory loaded");
        }

        // Search
        ImGui::SetNextItemWidth(-1);
        ImGui::InputText("##search", m_SearchBuf, sizeof(m_SearchBuf));
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Filter entries by name");

        // Toggle
        ImGui::Checkbox("Files only", &m_ShowOnlyFiles);
        ImGui::SameLine();
        ImGui::TextDisabled("%zu entries", m_State->browserEntries.size());
    }

    void BrowserPanel::RenderEntryList()
    {
        ImGui::BeginChild("##BrowserList", {0, 0}, false, ImGuiWindowFlags_HorizontalScrollbar);

        std::string filter(m_SearchBuf);
        std::transform(filter.begin(), filter.end(), filter.begin(), ::tolower);

        // Table with icon | name | size
        if (ImGui::BeginTable("##BrowserTable", 3,
                              ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg |
                                  ImGuiTableFlags_ScrollY | ImGuiTableFlags_BordersInnerV))
        {
            ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed, 45.f);
            ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Size", ImGuiTableColumnFlags_WidthFixed, 60.f);
            ImGui::TableHeadersRow();

            for (int i = 0; i < (int)m_State->browserEntries.size(); ++i)
            {
                const auto &e = m_State->browserEntries[i];

                if (m_ShowOnlyFiles && e.isDir)
                    continue;

                if (!filter.empty())
                {
                    std::string nm = e.name;
                    std::transform(nm.begin(), nm.end(), nm.begin(), ::tolower);
                    if (nm.find(filter) == std::string::npos)
                        continue;
                }

                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::TextDisabled("%s", GetFileIcon(e));

                ImGui::TableSetColumnIndex(1);
                bool selected = (m_State->selectedEntry == i);
                if (ImGui::Selectable(e.name.c_str(), selected,
                                      ImGuiSelectableFlags_SpanAllColumns |
                                          ImGuiSelectableFlags_AllowDoubleClick))
                {
                    m_State->selectedEntry = i;
                    if (ImGui::IsMouseDoubleClicked(0))
                        OpenEntry(i);
                    else
                        PopulateInspector(i);
                }

                ImGui::TableSetColumnIndex(2);
                if (!e.isDir)
                    ImGui::TextDisabled("%s", FormatSize(e.size).c_str());
            }

            ImGui::EndTable();
        }

        ImGui::EndChild();
    }

    void BrowserPanel::OpenEntry(int index)
    {
        if (index < 0 || index >= (int)m_State->browserEntries.size())
            return;
        const FileEntry &e = m_State->browserEntries[index];

        if (e.isDir)
        {
            RefreshDirectory(e.path);
            return;
        }

        // Load raw bytes
        m_State->ClearFile();
        m_State->loadedFilePath = e.path;

        std::ifstream f(e.path, std::ios::binary);
        if (f)
        {
            f.seekg(0, std::ios::end);
            size_t sz = f.tellg();
            f.seekg(0);
            m_State->rawBytes.resize(sz);
            f.read(reinterpret_cast<char *>(m_State->rawBytes.data()), sz);
            m_State->hasFile = true;
        }

        // Try JSON
        std::string name = e.name;
        auto toLower = [](std::string s)
        { std::transform(s.begin(),s.end(),s.begin(),::tolower); return s; };
        std::string nl = toLower(name);
        if (nl.size() > 5 && nl.substr(nl.size() - 5) == ".json")
        {
            try
            {
                std::ifstream jf(e.path);
                jf >> m_State->jsonData;
                m_State->hasJson = true;
                m_State->previewMode = PreviewMode::Json;
            }
            catch (...)
            {
            }
        }

        if (m_State->previewMode != PreviewMode::Json)
            DetectPreviewMode(index);

        PopulateInspector(index);
    }

    void BrowserPanel::RefreshDirectory(const std::string &path)
    {
        m_State->browserRoot = path;
        m_State->browserEntries.clear();

        // Add ".." parent
        fs::path fp(path);
        if (fp.has_parent_path() && fp.parent_path() != fp)
        {
            FileEntry pe;
            pe.name = "..";
            pe.path = fp.parent_path().string();
            pe.isDir = true;
            m_State->browserEntries.push_back(pe);
        }

        try
        {
            for (auto &de : fs::directory_iterator(path))
            {
                FileEntry fe;
                fe.name = de.path().filename().string();
                fe.path = de.path().string();
                fe.isDir = de.is_directory();
                fe.size = de.is_regular_file() ? de.file_size() : 0;
                m_State->browserEntries.push_back(fe);
            }
        }
        catch (...)
        {
        }

        std::sort(m_State->browserEntries.begin(), m_State->browserEntries.end(),
                  [](const FileEntry &a, const FileEntry &b)
                  {
                      if (a.isDir != b.isDir)
                          return a.isDir > b.isDir;
                      return a.name < b.name;
                  });
    }

    void BrowserPanel::DetectPreviewMode(int index)
    {
        if (index < 0 || index >= (int)m_State->browserEntries.size())
            return;
        const FileEntry &e = m_State->browserEntries[index];

        auto toLower = [](std::string s)
        { std::transform(s.begin(),s.end(),s.begin(),::tolower); return s; };
        std::string nl = toLower(e.name);

        auto endsWith = [&](const char *suf)
        {
            size_t sl = strlen(suf);
            return nl.size() >= sl && nl.substr(nl.size() - sl) == suf;
        };

        if (endsWith(".png") || endsWith(".jpg") || endsWith(".jpeg") ||
            endsWith(".bmp") || endsWith(".tga"))
            m_State->previewMode = PreviewMode::Image;
        else if (endsWith(".mp3") || endsWith(".ogg") || endsWith(".wav"))
            m_State->previewMode = PreviewMode::Audio;
        else if (endsWith(".txt") || endsWith(".xml") || endsWith(".csv") ||
                 endsWith(".ini") || endsWith(".lua") || endsWith(".glsl"))
            m_State->previewMode = PreviewMode::Text;
        else
            m_State->previewMode = PreviewMode::Hex;
    }

    void BrowserPanel::PopulateInspector(int index)
    {
        if (index < 0 || index >= (int)m_State->browserEntries.size())
            return;
        const FileEntry &e = m_State->browserEntries[index];

        m_State->inspectorProps.clear();
        m_State->inspectorProps.push_back({"Name", e.name});
        m_State->inspectorProps.push_back({"Path", e.path});
        m_State->inspectorProps.push_back({"Type", e.isDir ? "Directory" : "File"});
        if (!e.isDir)
            m_State->inspectorProps.push_back({"Size", FormatSize(e.size)});

        // Extension
        fs::path fp(e.path);
        if (fp.has_extension())
            m_State->inspectorProps.push_back({"Extension", fp.extension().string()});

        // Last write time
        try
        {
            auto lwt = fs::last_write_time(fp);
            auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
                lwt - fs::file_time_type::clock::now() +
                std::chrono::system_clock::now());
            std::time_t t = std::chrono::system_clock::to_time_t(sctp);
            char buf[64];
            std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", std::localtime(&t));
            m_State->inspectorProps.push_back({"Modified", std::string(buf)});
        }
        catch (...)
        {
        }

        // Magic bytes
        if (!m_State->rawBytes.empty() && m_State->rawBytes.size() >= 4)
        {
            char magic[12]{};
            std::snprintf(magic, sizeof(magic), "%02X %02X %02X %02X",
                          m_State->rawBytes[0], m_State->rawBytes[1],
                          m_State->rawBytes[2], m_State->rawBytes[3]);
            m_State->inspectorProps.push_back({"Magic Bytes", std::string(magic)});
        }
    }

} // namespace panels
