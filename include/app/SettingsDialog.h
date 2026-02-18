#pragma once
#include <memory>
#include <nlohmann/json.hpp>

struct AppState;

namespace dialogs
{

    class SettingsDialog
    {
    public:
        explicit SettingsDialog(std::shared_ptr<AppState> state);
        // Returns true while open; call each frame
        bool Render(bool *open);

    private:
        void RenderAppearanceTab();
        void RenderPreviewTab();
        void RenderAboutTab();

        std::shared_ptr<AppState> m_State;

        // Local copies edited before applying
        int m_ThemeIndex = 0;
        float m_FontSize = 14.0f;
        bool m_ShowAscii = true;
        int m_HexCols = 16;
        bool m_Dirty = false;
    };

} // namespace dialogs
