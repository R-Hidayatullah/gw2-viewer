#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <filesystem>
#include <nlohmann/json.hpp>

namespace fs = std::filesystem;

// -------------------------------------------------------
// Entry in the browser list
// -------------------------------------------------------
struct FileEntry
{
    std::string name;
    std::string path; // full path or dat-relative path
    uint64_t size = 0;
    bool isDir = false;
};

// -------------------------------------------------------
// Preview mode selection
// -------------------------------------------------------
enum class PreviewMode : int
{
    Hex = 0,
    Text = 1,
    Image = 2,
    Audio = 3,
    Json = 4,
    None = 5
};

// -------------------------------------------------------
// Inspector property
// -------------------------------------------------------
struct Property
{
    std::string key;
    std::string value;
};

// -------------------------------------------------------
// Global application state shared across all panels
// -------------------------------------------------------
struct AppState
{
    // --- Loaded data ---
    std::string loadedFilePath;
    std::vector<uint8_t> rawBytes;
    nlohmann::json jsonData;
    bool hasJson = false;
    bool hasFile = false;

    // --- Browser ---
    std::vector<FileEntry> browserEntries;
    int selectedEntry = -1;
    std::string browserRoot;

    // --- Preview ---
    PreviewMode previewMode = PreviewMode::Hex;

    // --- Inspector ---
    std::vector<Property> inspectorProps;

    // --- Settings ---
    int themeIndex = 0; // 0=Dark, 1=Light, 2=Classic, 3=GW2
    float fontSize = 14.0f;
    bool showHexAscii = true;
    int hexColumns = 16;
    bool settingsDirty = false;

    // --- Audio playback state (simple) ---
    bool audioPlaying = false;

    // helpers
    void ClearFile()
    {
        loadedFilePath.clear();
        rawBytes.clear();
        jsonData = nullptr;
        hasJson = false;
        hasFile = false;
        inspectorProps.clear();
        previewMode = PreviewMode::Hex;
        selectedEntry = -1;
    }
};
