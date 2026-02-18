#pragma once
#include <memory>
#include <nlohmann/json.hpp>

struct AppState;

namespace panels
{

    class InspectorPanel
    {
    public:
        explicit InspectorPanel(std::shared_ptr<AppState> state);
        void Render();

    private:
        void RenderFileInfo();
        void RenderProperties();
        void RenderRawStats();

        std::shared_ptr<AppState> m_State;
        char m_FilterBuf[128] = {};
    };

} // namespace panels
