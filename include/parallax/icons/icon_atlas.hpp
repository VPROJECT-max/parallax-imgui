#pragma once

#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>
#include <cstdint>
#include <imgui.h>

struct ID3D11Device;
struct ID3D11DeviceContext;
struct ID3D11Texture2D;
struct ID3D11ShaderResourceView;

namespace parallax::icons {

struct AtlasSubTexture {
    ImVec2 uv0{0.0f, 0.0f};
    ImVec2 uv1{1.0f, 1.0f};
    ImVec2 size{24.0f, 24.0f};
    ID3D11ShaderResourceView* srv{nullptr};
    bool valid{false};
};

struct AtlasAllocation {
    uint32_t x{0};
    uint32_t y{0};
    uint32_t width{0};
    uint32_t height{0};
    bool success{false};
};

class IconAtlas {
public:
    IconAtlas();
    ~IconAtlas();

    void Initialize(ID3D11Device* device, ID3D11DeviceContext* context);
    void Shutdown();

    // Look up or load an icon on demand
    AtlasSubTexture GetIcon(std::string_view name, float dpiScale = 1.0f, bool preserveColors = false);

    // Convenience drawing helper
    void DrawIcon(const char* name, const ImVec2& size = ImVec2(0.0f, 0.0f), const ImVec4& tint = ImVec4(1.0f, 1.0f, 1.0f, 1.0f));

    ID3D11ShaderResourceView* GetSRV() const noexcept { return m_srv; }
    uint32_t GetWidth() const noexcept { return m_width; }
    uint32_t GetHeight() const noexcept { return m_height; }

private:
    bool CreateAtlasTexture(uint32_t width, uint32_t height);
    AtlasAllocation AllocateRect(uint32_t width, uint32_t height);
    void GrowAtlas();
    void RecomputeAllUVs(uint32_t oldW, uint32_t oldH);

    ID3D11Device* m_device{nullptr};
    ID3D11DeviceContext* m_context{nullptr};
    ID3D11Texture2D* m_texture{nullptr};
    ID3D11ShaderResourceView* m_srv{nullptr};

    uint32_t m_width{512};
    uint32_t m_height{512};

    // Shelf allocator state
    uint32_t m_cursorX{1};
    uint32_t m_cursorY{1};
    uint32_t m_rowHeight{0};

    struct CachedEntry {
        uint32_t x;
        uint32_t y;
        uint32_t width;
        uint32_t height;
        AtlasSubTexture subTexture;
    };

    std::unordered_map<std::string, CachedEntry> m_entries;
};

} // namespace parallax::icons
