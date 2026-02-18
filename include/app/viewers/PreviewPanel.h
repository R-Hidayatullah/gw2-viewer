#pragma once
#include <memory>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

struct AppState;

namespace panels
{

    class PreviewPanel
    {
    public:
        explicit PreviewPanel(std::shared_ptr<AppState> state);
        ~PreviewPanel();
        void Render();

    private:
        void RenderModeSelector();
        void RenderHexView();
        void RenderTextView();
        void RenderImageView();
        void RenderAudioView();
        void RenderJsonView(const nlohmann::json &node, int depth = 0);
        void RenderEmptyState();

        // Texture handle for image preview
        unsigned int m_TexId = 0;
        int m_TexW = 0;
        int m_TexH = 0;
        std::string m_TexSource; // path of loaded texture

        // Hex view scroll
        float m_HexScrollY = 0.f;

        // Json search
        char m_JsonSearch[256] = {};

        std::shared_ptr<AppState> m_State;

        void LoadTextureFromBytes(const std::vector<uint8_t> &bytes);
        void FreeTexture();
    };

} // namespace panels
