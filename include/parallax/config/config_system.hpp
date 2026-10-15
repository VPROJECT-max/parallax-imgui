#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <functional>
#include <imgui.h>
#include "parallax/widgets/keybind.hpp"

namespace parallax::config {

enum class ConfigType {
    Bool,
    Int,
    Float,
    Color,
    Keybind,
    String,
    MultiBool
};

struct ConfigEntry {
    std::string key;
    ConfigType type;
    void* ptr;
    // Default values for reset
    union {
        bool bVal;
        int iVal;
        float fVal;
    } defaultScalar;
    ImVec4 defaultColor{1.0f, 1.0f, 1.0f, 1.0f};
    parallax::Keybind defaultKeybind;
    std::string defaultString;
    std::vector<bool> defaultMultiBool;
};

class ConfigManager {
public:
    static ConfigManager& Get() noexcept;

    // Directory & Profile Settings
    void SetConfigDirectory(const std::string& dir);
    const std::string& GetConfigDirectory() const noexcept;
    void RefreshProfileList();
    const std::vector<std::string>& GetProfileList() const noexcept;
    const std::string& GetActiveProfile() const noexcept;
    void SetActiveProfile(const std::string& profile);

    // Variable Binding API
    void BindBool(const std::string& key, bool* ptr, bool defaultVal = false);
    void BindInt(const std::string& key, int* ptr, int defaultVal = 0);
    void BindFloat(const std::string& key, float* ptr, float defaultVal = 0.0f);
    void BindColor(const std::string& key, ImVec4* ptr, const ImVec4& defaultVal = ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
    void BindKeybind(const std::string& key, parallax::Keybind* ptr);
    void BindString(const std::string& key, std::string* ptr, const std::string& defaultVal = "");
    void BindMultiBool(const std::string& key, std::vector<bool>* ptr);

    // Profile Actions (Atomic operations)
    bool SaveProfile(const std::string& profileName);
    bool LoadProfile(const std::string& profileName);
    bool DeleteProfile(const std::string& profileName);
    bool CreateProfile(const std::string& profileName);
    void ResetToDefaults();

    // Clipboard Sharing (Base64)
    std::string ExportToBase64();
    bool ImportFromBase64(const std::string& base64Str);
    bool CopyConfigToClipboard();
    bool ImportConfigFromClipboard();

    // Auto-Save Mechanics
    void MarkDirty() noexcept;
    void Update(float dt);

    // Render Full Config Manager UI
    void RenderConfigTab();

private:
    ConfigManager();
    ~ConfigManager() = default;

    std::string m_configDir{"configs"};
    std::string m_activeProfile{"Default"};
    std::vector<std::string> m_profiles;
    std::unordered_map<std::string, ConfigEntry> m_entries;

    bool m_isDirty{false};
    float m_autoSaveTimer{0.0f};
    float m_autoSaveDelay{1.5f}; // 1.5 second debounce

    // UI state for modals
    char m_newProfileBuf[64]{""};
    char m_importBuf[1024]{""};
    bool m_showCreateModal{false};
    bool m_showDeleteConfirm{false};
    bool m_showImportModal{false};
};

} // namespace parallax::config
