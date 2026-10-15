#include "parallax/config/config_system.hpp"
#include "parallax/core/context.hpp"
#include "parallax/core/style.hpp"
#include "parallax/core/typography.hpp"
#include "parallax/widgets/button.hpp"
#include "parallax/widgets/dropdown.hpp"
#include "parallax/widgets/notification.hpp"
#include "parallax/layout/grid.hpp"

#include <nlohmann/json.hpp>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <windows.h>

namespace fs = std::filesystem;
using json = nlohmann::json;

namespace parallax::config {

// -----------------------------------------------------------------------------
// Base64 Helpers
// -----------------------------------------------------------------------------
static const char s_base64Chars[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789+/";

static std::string Base64Encode(const std::string& input) {
    std::string ret;
    int val = 0, valb = -6;
    for (unsigned char c : input) {
        val = (val << 8) + c;
        valb += 8;
        while (valb >= 0) {
            ret.push_back(s_base64Chars[(val >> valb) & 0x3F]);
            valb -= 6;
        }
    }
    if (valb > -6) ret.push_back(s_base64Chars[((val << 8) >> (valb + 8)) & 0x3F]);
    while (ret.size() % 4) ret.push_back('=');
    return ret;
}

static std::string Base64Decode(const std::string& input) {
    std::string ret;
    std::vector<int> T(256, -1);
    for (int i = 0; i < 64; i++) T[s_base64Chars[i]] = i;

    int val = 0, valb = -8;
    for (unsigned char c : input) {
        if (T[c] == -1) break;
        val = (val << 6) + T[c];
        valb += 6;
        if (valb >= 0) {
            ret.push_back(char((val >> valb) & 0xFF));
            valb -= 8;
        }
    }
    return ret;
}

// -----------------------------------------------------------------------------
// ConfigManager Implementation
// -----------------------------------------------------------------------------
ConfigManager& ConfigManager::Get() noexcept {
    static ConfigManager s_instance;
    return s_instance;
}

ConfigManager::ConfigManager() {
    try {
        fs::create_directories(m_configDir);
    } catch (...) {}

    RefreshProfileList();
}

void ConfigManager::SetConfigDirectory(const std::string& dir) {
    m_configDir = dir;
    try {
        fs::create_directories(m_configDir);
    } catch (...) {}
    RefreshProfileList();
}

const std::string& ConfigManager::GetConfigDirectory() const noexcept {
    return m_configDir;
}

void ConfigManager::RefreshProfileList() {
    m_profiles.clear();

    try {
        if (fs::exists(m_configDir)) {
            for (const auto& entry : fs::directory_iterator(m_configDir)) {
                if (entry.is_regular_file() && entry.path().extension() == ".json") {
                    m_profiles.push_back(entry.path().stem().string());
                }
            }
        }
    } catch (...) {}

    // Ensure default profiles exist if empty
    if (m_profiles.empty()) {
        m_profiles.push_back("Default");
        m_profiles.push_back("Legit");
        m_profiles.push_back("Rage");
    }

    // Sort alphabetically
    std::sort(m_profiles.begin(), m_profiles.end());
}

const std::vector<std::string>& ConfigManager::GetProfileList() const noexcept {
    return m_profiles;
}

const std::string& ConfigManager::GetActiveProfile() const noexcept {
    return m_activeProfile;
}

void ConfigManager::SetActiveProfile(const std::string& profile) {
    m_activeProfile = profile;
}

void ConfigManager::BindBool(const std::string& key, bool* ptr, bool defaultVal) {
    if (!ptr) return;
    ConfigEntry entry;
    entry.key = key;
    entry.type = ConfigType::Bool;
    entry.ptr = ptr;
    entry.defaultScalar.bVal = defaultVal;
    m_entries[key] = entry;
}

void ConfigManager::BindInt(const std::string& key, int* ptr, int defaultVal) {
    if (!ptr) return;
    ConfigEntry entry;
    entry.key = key;
    entry.type = ConfigType::Int;
    entry.ptr = ptr;
    entry.defaultScalar.iVal = defaultVal;
    m_entries[key] = entry;
}

void ConfigManager::BindFloat(const std::string& key, float* ptr, float defaultVal) {
    if (!ptr) return;
    ConfigEntry entry;
    entry.key = key;
    entry.type = ConfigType::Float;
    entry.ptr = ptr;
    entry.defaultScalar.fVal = defaultVal;
    m_entries[key] = entry;
}

void ConfigManager::BindColor(const std::string& key, ImVec4* ptr, const ImVec4& defaultVal) {
    if (!ptr) return;
    ConfigEntry entry;
    entry.key = key;
    entry.type = ConfigType::Color;
    entry.ptr = ptr;
    entry.defaultColor = defaultVal;
    m_entries[key] = entry;
}

void ConfigManager::BindKeybind(const std::string& key, parallax::Keybind* ptr) {
    if (!ptr) return;
    ConfigEntry entry;
    entry.key = key;
    entry.type = ConfigType::Keybind;
    entry.ptr = ptr;
    entry.defaultKeybind = *ptr;
    m_entries[key] = entry;
}

void ConfigManager::BindString(const std::string& key, std::string* ptr, const std::string& defaultVal) {
    if (!ptr) return;
    ConfigEntry entry;
    entry.key = key;
    entry.type = ConfigType::String;
    entry.ptr = ptr;
    entry.defaultString = defaultVal;
    m_entries[key] = entry;
}

void ConfigManager::BindMultiBool(const std::string& key, std::vector<bool>* ptr) {
    if (!ptr) return;
    ConfigEntry entry;
    entry.key = key;
    entry.type = ConfigType::MultiBool;
    entry.ptr = ptr;
    entry.defaultMultiBool = *ptr;
    m_entries[key] = entry;
}

bool ConfigManager::SaveProfile(const std::string& profileName) {
    if (profileName.empty()) return false;

    json root;
    root["schema_version"] = 1;
    root["profile_name"] = profileName;

    json values = json::object();

    for (const auto& [key, entry] : m_entries) {
        if (!entry.ptr) continue;

        switch (entry.type) {
            case ConfigType::Bool:
                values[key] = *reinterpret_cast<bool*>(entry.ptr);
                break;
            case ConfigType::Int:
                values[key] = *reinterpret_cast<int*>(entry.ptr);
                break;
            case ConfigType::Float:
                values[key] = *reinterpret_cast<float*>(entry.ptr);
                break;
            case ConfigType::Color: {
                const ImVec4& c = *reinterpret_cast<ImVec4*>(entry.ptr);
                values[key] = { {"r", c.x}, {"g", c.y}, {"b", c.z}, {"a", c.w} };
                break;
            }
            case ConfigType::Keybind: {
                const auto& kb = *reinterpret_cast<parallax::Keybind*>(entry.ptr);
                values[key] = {
                    {"vkey", kb.vkey},
                    {"mode", static_cast<int>(kb.mode)},
                    {"active", kb.active}
                };
                break;
            }
            case ConfigType::String:
                values[key] = *reinterpret_cast<std::string*>(entry.ptr);
                break;
            case ConfigType::MultiBool: {
                const auto& vec = *reinterpret_cast<std::vector<bool>*>(entry.ptr);
                json arr = json::array();
                for (bool b : vec) arr.push_back(b);
                values[key] = arr;
                break;
            }
        }
    }

    root["values"] = values;

    // Atomic write pattern: write to .tmp, flush, and rename
    try {
        fs::create_directories(m_configDir);
        std::string tmpPath = m_configDir + "/" + profileName + ".tmp";
        std::string finalPath = m_configDir + "/" + profileName + ".json";

        std::ofstream file(tmpPath);
        if (!file.is_open()) return false;
        file << root.dump(4);
        file.close();

        if (fs::exists(finalPath)) {
            fs::remove(finalPath);
        }
        fs::rename(tmpPath, finalPath);

        RefreshProfileList();
        m_activeProfile = profileName;
        m_isDirty = false;
        return true;
    } catch (...) {
        return false;
    }
}

bool ConfigManager::LoadProfile(const std::string& profileName) {
    if (profileName.empty()) return false;

    std::string filePath = m_configDir + "/" + profileName + ".json";
    if (!fs::exists(filePath)) return false;

    try {
        std::ifstream file(filePath);
        if (!file.is_open()) return false;

        json root = json::parse(file);
        if (!root.contains("values") || !root["values"].is_object()) return false;

        const auto& values = root["values"];

        for (auto& [key, entry] : m_entries) {
            if (!entry.ptr || !values.contains(key)) continue;

            const auto& val = values[key];

            switch (entry.type) {
                case ConfigType::Bool:
                    if (val.is_boolean()) *reinterpret_cast<bool*>(entry.ptr) = val.get<bool>();
                    break;
                case ConfigType::Int:
                    if (val.is_number_integer()) *reinterpret_cast<int*>(entry.ptr) = val.get<int>();
                    break;
                case ConfigType::Float:
                    if (val.is_number()) *reinterpret_cast<float*>(entry.ptr) = val.get<float>();
                    break;
                case ConfigType::Color:
                    if (val.is_object() && val.contains("r") && val.contains("g") && val.contains("b") && val.contains("a")) {
                        ImVec4& c = *reinterpret_cast<ImVec4*>(entry.ptr);
                        c.x = val["r"].get<float>();
                        c.y = val["g"].get<float>();
                        c.z = val["b"].get<float>();
                        c.w = val["a"].get<float>();
                    }
                    break;
                case ConfigType::Keybind:
                    if (val.is_object() && (val.contains("vkey") || val.contains("key")) && val.contains("mode")) {
                        auto& kb = *reinterpret_cast<parallax::Keybind*>(entry.ptr);
                        kb.vkey = val.contains("vkey") ? val["vkey"].get<int>() : val["key"].get<int>();
                        kb.mode = static_cast<parallax::KeybindMode>(val["mode"].get<int>());
                        if (val.contains("active")) kb.active = val["active"].get<bool>();
                    }
                    break;
                case ConfigType::String:
                    if (val.is_string()) *reinterpret_cast<std::string*>(entry.ptr) = val.get<std::string>();
                    break;
                case ConfigType::MultiBool:
                    if (val.is_array()) {
                        auto& vec = *reinterpret_cast<std::vector<bool>*>(entry.ptr);
                        vec.clear();
                        for (const auto& item : val) {
                            if (item.is_boolean()) vec.push_back(item.get<bool>());
                        }
                    }
                    break;
            }
        }

        m_activeProfile = profileName;
        m_isDirty = false;
        return true;
    } catch (...) {
        return false;
    }
}

bool ConfigManager::DeleteProfile(const std::string& profileName) {
    if (profileName.empty()) return false;

    std::string filePath = m_configDir + "/" + profileName + ".json";
    try {
        if (fs::exists(filePath)) {
            fs::remove(filePath);
            RefreshProfileList();
            if (!m_profiles.empty()) {
                m_activeProfile = m_profiles[0];
                LoadProfile(m_activeProfile);
            }
            return true;
        }
    } catch (...) {}
    return false;
}

bool ConfigManager::CreateProfile(const std::string& profileName) {
    if (profileName.empty()) return false;
    return SaveProfile(profileName);
}

void ConfigManager::ResetToDefaults() {
    for (auto& [key, entry] : m_entries) {
        if (!entry.ptr) continue;

        switch (entry.type) {
            case ConfigType::Bool:
                *reinterpret_cast<bool*>(entry.ptr) = entry.defaultScalar.bVal;
                break;
            case ConfigType::Int:
                *reinterpret_cast<int*>(entry.ptr) = entry.defaultScalar.iVal;
                break;
            case ConfigType::Float:
                *reinterpret_cast<float*>(entry.ptr) = entry.defaultScalar.fVal;
                break;
            case ConfigType::Color:
                *reinterpret_cast<ImVec4*>(entry.ptr) = entry.defaultColor;
                break;
            case ConfigType::Keybind:
                *reinterpret_cast<parallax::Keybind*>(entry.ptr) = entry.defaultKeybind;
                break;
            case ConfigType::String:
                *reinterpret_cast<std::string*>(entry.ptr) = entry.defaultString;
                break;
            case ConfigType::MultiBool:
                *reinterpret_cast<std::vector<bool>*>(entry.ptr) = entry.defaultMultiBool;
                break;
        }
    }
    m_isDirty = true;
}

std::string ConfigManager::ExportToBase64() {
    json root;
    root["schema_version"] = 1;
    root["profile_name"] = m_activeProfile;

    json values = json::object();
    for (const auto& [key, entry] : m_entries) {
        if (!entry.ptr) continue;
        switch (entry.type) {
            case ConfigType::Bool: values[key] = *reinterpret_cast<bool*>(entry.ptr); break;
            case ConfigType::Int: values[key] = *reinterpret_cast<int*>(entry.ptr); break;
            case ConfigType::Float: values[key] = *reinterpret_cast<float*>(entry.ptr); break;
            case ConfigType::Color: {
                const ImVec4& c = *reinterpret_cast<ImVec4*>(entry.ptr);
                values[key] = { {"r", c.x}, {"g", c.y}, {"b", c.z}, {"a", c.w} };
                break;
            }
            case ConfigType::Keybind: {
                const auto& kb = *reinterpret_cast<parallax::Keybind*>(entry.ptr);
                values[key] = { {"vkey", kb.vkey}, {"mode", static_cast<int>(kb.mode)}, {"active", kb.active} };
                break;
            }
            case ConfigType::String: values[key] = *reinterpret_cast<std::string*>(entry.ptr); break;
            case ConfigType::MultiBool: {
                const auto& vec = *reinterpret_cast<std::vector<bool>*>(entry.ptr);
                json arr = json::array();
                for (bool b : vec) arr.push_back(b);
                values[key] = arr;
                break;
            }
        }
    }
    root["values"] = values;
    return Base64Encode(root.dump());
}

bool ConfigManager::ImportFromBase64(const std::string& base64Str) {
    if (base64Str.empty()) return false;
    try {
        std::string jsonStr = Base64Decode(base64Str);
        json root = json::parse(jsonStr);
        if (!root.contains("values") || !root["values"].is_object()) return false;

        const auto& values = root["values"];
        for (auto& [key, entry] : m_entries) {
            if (!entry.ptr || !values.contains(key)) continue;
            const auto& val = values[key];
            switch (entry.type) {
                case ConfigType::Bool:
                    if (val.is_boolean()) *reinterpret_cast<bool*>(entry.ptr) = val.get<bool>();
                    break;
                case ConfigType::Int:
                    if (val.is_number_integer()) *reinterpret_cast<int*>(entry.ptr) = val.get<int>();
                    break;
                case ConfigType::Float:
                    if (val.is_number()) *reinterpret_cast<float*>(entry.ptr) = val.get<float>();
                    break;
                case ConfigType::Color:
                    if (val.is_object() && val.contains("r")) {
                        ImVec4& c = *reinterpret_cast<ImVec4*>(entry.ptr);
                        c.x = val["r"].get<float>();
                        c.y = val["g"].get<float>();
                        c.z = val["b"].get<float>();
                        c.w = val["a"].get<float>();
                    }
                    break;
                case ConfigType::Keybind:
                    if (val.is_object() && (val.contains("vkey") || val.contains("key"))) {
                        auto& kb = *reinterpret_cast<parallax::Keybind*>(entry.ptr);
                        kb.vkey = val.contains("vkey") ? val["vkey"].get<int>() : val["key"].get<int>();
                        kb.mode = static_cast<parallax::KeybindMode>(val["mode"].get<int>());
                    }
                    break;
                case ConfigType::String:
                    if (val.is_string()) *reinterpret_cast<std::string*>(entry.ptr) = val.get<std::string>();
                    break;
                case ConfigType::MultiBool:
                    if (val.is_array()) {
                        auto& vec = *reinterpret_cast<std::vector<bool>*>(entry.ptr);
                        vec.clear();
                        for (const auto& item : val) if (item.is_boolean()) vec.push_back(item.get<bool>());
                    }
                    break;
            }
        }
        m_isDirty = true;
        return true;
    } catch (...) {
        return false;
    }
}

bool ConfigManager::CopyConfigToClipboard() {
    std::string b64 = ExportToBase64();
    if (b64.empty()) return false;
    ImGui::SetClipboardText(b64.c_str());
    return true;
}

bool ConfigManager::ImportConfigFromClipboard() {
    const char* clip = ImGui::GetClipboardText();
    if (!clip || clip[0] == '\0') return false;
    return ImportFromBase64(clip);
}

void ConfigManager::MarkDirty() noexcept {
    m_isDirty = true;
    m_autoSaveTimer = 0.0f;
}

void ConfigManager::Update(float dt) {
    if (!m_isDirty) return;

    m_autoSaveTimer += dt;
    if (m_autoSaveTimer >= m_autoSaveDelay) {
        SaveProfile(m_activeProfile);
        m_isDirty = false;
        m_autoSaveTimer = 0.0f;
    }
}

void ConfigManager::RenderConfigTab() {
    Context& ctx = Context::Get();
    const float dpi = ctx.GetDpiScale();
    const auto& colors = StyleManager::Get().Colors();
    const auto& metrics = StyleManager::Get().Metrics();
    (void)metrics;

    layout::Grid::Begin(2, 14.0f);

    // =========================================================================
    // Column 1: Profile Manager & Actions
    // =========================================================================
    layout::Groupbox::Begin("Profile Manager & Serialization", "sliders");
    ImGui::TextColored(colors.TextMuted, "Save, load, and manage atomic configuration profiles:");
    ImGui::Spacing();

    // Profile Dropdown Selector
    int activeIdx = 0;
    for (size_t i = 0; i < m_profiles.size(); ++i) {
        if (m_profiles[i] == m_activeProfile) {
            activeIdx = static_cast<int>(i);
            break;
        }
    }

    if (Dropdown("Active Profile Selector", m_profiles, &activeIdx)) {
        if (activeIdx >= 0 && activeIdx < static_cast<int>(m_profiles.size())) {
            m_activeProfile = m_profiles[activeIdx];
            if (LoadProfile(m_activeProfile)) {
                Notify(NotificationLevel::Info, "Profile Loaded", "Loaded settings from " + m_activeProfile, 3.0f);
            }
        }
    }

    ImGui::Spacing();
    ImGui::Spacing();

    // Save and Load Buttons
    const float halfBtnW = (ImGui::GetContentRegionAvail().x - 10.0f * dpi) * 0.5f;

    if (Button("Save Active Profile", "save", ButtonVariant::Primary, ImVec2(halfBtnW, 36.0f * dpi))) {
        if (SaveProfile(m_activeProfile)) {
            Notify(NotificationLevel::Success, "Profile Saved", "Saved atomic configuration to " + m_activeProfile + ".json", 3.5f);
        } else {
            Notify(NotificationLevel::Danger, "Save Failed", "Could not write profile to disk.", 4.0f);
        }
    }

    ImGui::SameLine(0.0f, 10.0f * dpi);

    if (Button("Reload From Disk", "refresh-cw", ButtonVariant::Secondary, ImVec2(halfBtnW, 36.0f * dpi))) {
        if (LoadProfile(m_activeProfile)) {
            Notify(NotificationLevel::Info, "Profile Reloaded", "Re-applied settings from disk.", 3.0f);
        }
    }

    ImGui::Spacing();

    // Create New Profile & Delete Profile Buttons
    if (Button("Create New Profile", "plus", ButtonVariant::Secondary, ImVec2(halfBtnW, 36.0f * dpi))) {
        m_showCreateModal = true;
        m_newProfileBuf[0] = '\0';
    }

    ImGui::SameLine(0.0f, 10.0f * dpi);

    if (Button("Delete Profile", "trash-2", ButtonVariant::Danger, ImVec2(halfBtnW, 36.0f * dpi))) {
        m_showDeleteConfirm = true;
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Profile Details Card
    ImGui::TextColored(colors.TextSecondary, "Active Profile Details:");
    ImGui::BulletText("Name: %s.json", m_activeProfile.c_str());
    ImGui::BulletText("Total Bound Variables: %zu items", m_entries.size());
    ImGui::BulletText("Storage Schema: Version 1 (RFC 8259 JSON)");
    ImGui::BulletText("Integrity: Atomic write with .tmp swap");

    layout::Groupbox::End();

    layout::Grid::NextColumn();

    // =========================================================================
    // Column 2: Profile Sharing & Clipboard Code
    // =========================================================================
    layout::Groupbox::Begin("Cloud & Clipboard Sharing", "share-2");
    ImGui::TextColored(colors.TextMuted, "Export configs as compact Base64 codes to share with friends:");
    ImGui::Spacing();

    if (Button("Export to Clipboard (Base64)", "copy", ButtonVariant::Primary, ImVec2(-1.0f, 38.0f * dpi))) {
        if (CopyConfigToClipboard()) {
            Notify(NotificationLevel::Success, "Config Copied", "Base64 configuration code copied to clipboard!", 4.0f);
        }
    }

    ImGui::Spacing();

    if (Button("Import from Clipboard", "download", ButtonVariant::Secondary, ImVec2(-1.0f, 38.0f * dpi))) {
        if (ImportConfigFromClipboard()) {
            Notify(NotificationLevel::Success, "Config Imported", "Applied configuration code from clipboard!", 4.0f);
        } else {
            Notify(NotificationLevel::Warning, "Import Failed", "Clipboard does not contain valid Base64 JSON config.", 4.0f);
        }
    }

    ImGui::Spacing();

    if (Button("Paste Code Manually", "terminal", ButtonVariant::Ghost, ImVec2(-1.0f, 34.0f * dpi))) {
        m_showImportModal = true;
        m_importBuf[0] = '\0';
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Reset Defaults
    ImGui::TextColored(colors.TextSecondary, "Factory Reset:");
    ImGui::TextWrapped("Restore all bound controls to their default factory baseline values:");
    ImGui::Spacing();

    if (Button("Reset Current Profile to Defaults", "alert-circle", ButtonVariant::Danger, ImVec2(-1.0f, 36.0f * dpi))) {
        ResetToDefaults();
        Notify(NotificationLevel::Warning, "Defaults Restored", "All settings reset to baseline defaults.", 3.5f);
    }

    layout::Groupbox::End();

    layout::Grid::End();

    // =========================================================================
    // Modal: Create Profile Dialog
    // =========================================================================
    if (m_showCreateModal) {
        ImGui::OpenPopup("Create New Profile##Modal");
    }

    if (ImGui::BeginPopupModal("Create New Profile##Modal", &m_showCreateModal, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove)) {
        PushFont(FontWeight::SemiBold);
        ImGui::Text("Enter Profile Name:");
        PopFont();
        ImGui::Spacing();

        ImGui::SetNextItemWidth(280.0f * dpi);
        ImGui::InputTextWithHint("##NewProfName", "e.g. Tournament_Legit", m_newProfileBuf, sizeof(m_newProfileBuf));
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        if (Button("Create", "check", ButtonVariant::Primary, ImVec2(135.0f * dpi, 32.0f * dpi))) {
            if (m_newProfileBuf[0] != '\0') {
                CreateProfile(m_newProfileBuf);
                Notify(NotificationLevel::Success, "Profile Created", std::string("Created ") + m_newProfileBuf, 3.5f);
                m_showCreateModal = false;
                ImGui::CloseCurrentPopup();
            }
        }
        ImGui::SameLine(0.0f, 10.0f * dpi);
        if (Button("Cancel", "x", ButtonVariant::Secondary, ImVec2(135.0f * dpi, 32.0f * dpi))) {
            m_showCreateModal = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    // =========================================================================
    // Modal: Delete Profile Confirmation Dialog
    // =========================================================================
    if (m_showDeleteConfirm) {
        ImGui::OpenPopup("Delete Profile Confirmation##Modal");
    }

    if (ImGui::BeginPopupModal("Delete Profile Confirmation##Modal", &m_showDeleteConfirm, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove)) {
        PushFont(FontWeight::SemiBold);
        ImGui::TextColored(colors.Danger, "Delete Profile: %s?", m_activeProfile.c_str());
        PopFont();
        ImGui::Spacing();
        ImGui::TextWrapped("This operation will permanently delete the JSON configuration file from disk.");
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        if (Button("Yes, Delete", "trash-2", ButtonVariant::Danger, ImVec2(135.0f * dpi, 32.0f * dpi))) {
            std::string toDel = m_activeProfile;
            if (DeleteProfile(toDel)) {
                Notify(NotificationLevel::Warning, "Profile Deleted", "Permanently removed " + toDel, 3.5f);
            }
            m_showDeleteConfirm = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine(0.0f, 10.0f * dpi);
        if (Button("Cancel", "x", ButtonVariant::Secondary, ImVec2(135.0f * dpi, 32.0f * dpi))) {
            m_showDeleteConfirm = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    // =========================================================================
    // Modal: Manual Base64 Import Dialog
    // =========================================================================
    if (m_showImportModal) {
        ImGui::OpenPopup("Import Base64 Code##Modal");
    }

    if (ImGui::BeginPopupModal("Import Base64 Code##Modal", &m_showImportModal, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove)) {
        PushFont(FontWeight::SemiBold);
        ImGui::Text("Paste Base64 Configuration String:");
        PopFont();
        ImGui::Spacing();

        ImGui::InputTextMultiline("##ManualImportBuf", m_importBuf, sizeof(m_importBuf), ImVec2(340.0f * dpi, 100.0f * dpi));
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        if (Button("Apply Code", "check", ButtonVariant::Primary, ImVec2(165.0f * dpi, 32.0f * dpi))) {
            if (ImportFromBase64(m_importBuf)) {
                Notify(NotificationLevel::Success, "Code Applied", "Loaded configuration from Base64 string.", 3.5f);
                m_showImportModal = false;
                ImGui::CloseCurrentPopup();
            } else {
                Notify(NotificationLevel::Danger, "Invalid Code", "Failed to parse Base64 configuration string.", 4.0f);
            }
        }
        ImGui::SameLine(0.0f, 10.0f * dpi);
        if (Button("Cancel", "x", ButtonVariant::Secondary, ImVec2(165.0f * dpi, 32.0f * dpi))) {
            m_showImportModal = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

} // namespace parallax::config
