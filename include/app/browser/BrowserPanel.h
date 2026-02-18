#pragma once
#include <memory>
#include <string>
#include <nlohmann/json.hpp>

struct AppState;

namespace panels
{

    class BrowserPanel
    {
    public:
        explicit BrowserPanel(std::shared_ptr<AppState> state);
        void Render();

    private:
        void RenderToolbar();
        void RenderEntryList();
        void OpenEntry(int index);
        void RefreshDirectory(const std::string &path);
        void DetectPreviewMode(int index);
        void PopulateInspector(int index);

        std::shared_ptr<AppState> m_State;
        char m_SearchBuf[256] = {};
        bool m_ShowOnlyFiles = false;
    };

} // namespace panels
