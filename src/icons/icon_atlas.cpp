#include "parallax/icons/icon_atlas.hpp"
#include "parallax/icons/icon_loader.hpp"
#include "parallax/core/context.hpp"
#include <d3d11.h>
#include <cmath>
#include <algorithm>

namespace parallax::icons {

IconAtlas::IconAtlas() = default;

IconAtlas::~IconAtlas() {
    Shutdown();
}

void IconAtlas::Initialize(ID3D11Device* device, ID3D11DeviceContext* context) {
    if (!device || !context) return;
    m_device = device;
    m_context = context;

    CreateAtlasTexture(m_width, m_height);
}

void IconAtlas::Shutdown() {
    if (m_srv) {
        m_srv->Release();
        m_srv = nullptr;
    }
    if (m_texture) {
        m_texture->Release();
        m_texture = nullptr;
    }
    m_entries.clear();
    m_device = nullptr;
    m_context = nullptr;
}

bool IconAtlas::CreateAtlasTexture(uint32_t width, uint32_t height) {
    if (!m_device) return false;

    D3D11_TEXTURE2D_DESC desc{};
    desc.Width = width;
    desc.Height = height;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    desc.CPUAccessFlags = 0;
    desc.MiscFlags = 0;

    // Clear texture with transparent pixels initially
    std::vector<uint32_t> initialPixels(static_cast<size_t>(width) * height, 0x00000000);
    D3D11_SUBRESOURCE_DATA subData{};
    subData.pSysMem = initialPixels.data();
    subData.SysMemPitch = width * 4;

    ID3D11Texture2D* newTex = nullptr;
    HRESULT hr = m_device->CreateTexture2D(&desc, &subData, &newTex);
    if (FAILED(hr)) return false;

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
    srvDesc.Format = desc.Format;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Texture2D.MipLevels = 1;

    ID3D11ShaderResourceView* newSrv = nullptr;
    hr = m_device->CreateShaderResourceView(newTex, &srvDesc, &newSrv);
    if (FAILED(hr)) {
        newTex->Release();
        return false;
    }

    if (m_srv) m_srv->Release();
    if (m_texture) m_texture->Release();

    m_texture = newTex;
    m_srv = newSrv;
    m_width = width;
    m_height = height;

    return true;
}

AtlasAllocation IconAtlas::AllocateRect(uint32_t width, uint32_t height) {
    // 1px border padding to prevent bilinear filtering cross-bleed
    const uint32_t pad = 1;
    const uint32_t totalW = width + pad;
    const uint32_t totalH = height + pad;

    if (m_cursorX + totalW > m_width) {
        // Wrap to next shelf row
        m_cursorX = 1;
        m_cursorY += m_rowHeight + pad;
        m_rowHeight = 0;
    }

    if (m_cursorY + totalH > m_height) {
        // Atlas full - trigger dynamic growth
        GrowAtlas();
        return AllocateRect(width, height);
    }

    AtlasAllocation alloc;
    alloc.x = m_cursorX;
    alloc.y = m_cursorY;
    alloc.width = width;
    alloc.height = height;
    alloc.success = true;

    m_cursorX += totalW;
    m_rowHeight = std::max(m_rowHeight, totalH);

    return alloc;
}

void IconAtlas::GrowAtlas() {
    uint32_t oldW = m_width;
    uint32_t oldH = m_height;
    uint32_t newW = m_width * 2;
    uint32_t newH = m_height * 2;

    if (!m_device || !m_context || !m_texture) return;

    D3D11_TEXTURE2D_DESC desc{};
    desc.Width = newW;
    desc.Height = newH;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    ID3D11Texture2D* newTex = nullptr;
    if (FAILED(m_device->CreateTexture2D(&desc, nullptr, &newTex))) return;

    // Copy old texture contents into top-left of enlarged texture
    D3D11_BOX srcBox{0, 0, 0, oldW, oldH, 1};
    m_context->CopySubresourceRegion(newTex, 0, 0, 0, 0, m_texture, 0, &srcBox);

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
    srvDesc.Format = desc.Format;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = 1;

    ID3D11ShaderResourceView* newSrv = nullptr;
    if (FAILED(m_device->CreateShaderResourceView(newTex, &srvDesc, &newSrv))) {
        newTex->Release();
        return;
    }

    m_texture->Release();
    m_srv->Release();

    m_texture = newTex;
    m_srv = newSrv;
    m_width = newW;
    m_height = newH;

    RecomputeAllUVs(oldW, oldH);
}

void IconAtlas::RecomputeAllUVs(uint32_t oldW, uint32_t oldH) {
    const float scaleX = static_cast<float>(oldW) / static_cast<float>(m_width);
    const float scaleY = static_cast<float>(oldH) / static_cast<float>(m_height);

    for (auto& [key, entry] : m_entries) {
        entry.subTexture.srv = m_srv;
        entry.subTexture.uv0.x *= scaleX;
        entry.subTexture.uv0.y *= scaleY;
        entry.subTexture.uv1.x *= scaleX;
        entry.subTexture.uv1.y *= scaleY;
    }
}

AtlasSubTexture IconAtlas::GetIcon(std::string_view name, float dpiScale, bool preserveColors) {
    if (name.empty()) {
        name = "shield";
    }

    std::string key = std::string(name) + (preserveColors ? ":color" : ":alpha");
    auto it = m_entries.find(key);
    if (it != m_entries.end()) {
        return it->second.subTexture;
    }

    if (!m_device || !m_context || !m_texture) {
        return AtlasSubTexture{};
    }

    // Rasterize at current DPI scale (crisp at 100%, 150%, 200% scaling)
    const uint32_t targetPx = std::clamp(static_cast<uint32_t>(std::round(24.0f * (dpiScale > 0.1f ? dpiScale : 1.0f))), 16u, 128u);

    std::vector<uint8_t> pixels;
    if (!IconLoader::RasterizeSVG(name, targetPx, targetPx, preserveColors, pixels)) {
        return AtlasSubTexture{};
    }

    AtlasAllocation alloc = AllocateRect(targetPx, targetPx);
    if (!alloc.success) {
        return AtlasSubTexture{};
    }

    // Upload rectangle directly into D3D11 texture
    D3D11_BOX box{};
    box.left = alloc.x;
    box.top = alloc.y;
    box.front = 0;
    box.right = alloc.x + alloc.width;
    box.bottom = alloc.y + alloc.height;
    box.back = 1;

    m_context->UpdateSubresource(m_texture, 0, &box, pixels.data(), targetPx * 4, 0);

    AtlasSubTexture subTex;
    subTex.uv0 = ImVec2(static_cast<float>(alloc.x) / m_width, static_cast<float>(alloc.y) / m_height);
    subTex.uv1 = ImVec2(static_cast<float>(alloc.x + alloc.width) / m_width, static_cast<float>(alloc.y + alloc.height) / m_height);
    subTex.size = ImVec2(24.0f, 24.0f);
    subTex.srv = m_srv;
    subTex.valid = true;

    CachedEntry entry;
    entry.x = alloc.x;
    entry.y = alloc.y;
    entry.width = alloc.width;
    entry.height = alloc.height;
    entry.subTexture = subTex;

    m_entries.emplace(key, entry);
    return subTex;
}

void IconAtlas::DrawIcon(const char* name, const ImVec2& size, const ImVec4& tint, bool preserveColors) {
    const float dpi = Context::Get().GetDpiScale();
    AtlasSubTexture sub = GetIcon(name, dpi, preserveColors);

    const ImVec2 renderSize = (size.x > 0.0f && size.y > 0.0f) ? size : ImVec2(sub.size.x * dpi, sub.size.y * dpi);
    const ImVec2 pos = ImGui::GetCursorScreenPos();
    ImGui::Dummy(renderSize);

    if (!sub.valid || !sub.srv) return;

    ImGui::GetWindowDrawList()->AddImage(
        reinterpret_cast<ImTextureID>(sub.srv),
        pos,
        ImVec2(pos.x + renderSize.x, pos.y + renderSize.y),
        sub.uv0, sub.uv1,
        ImColor(tint)
    );
}

void IconAtlas::DrawIcon(ImDrawList* drawList, const char* name, const ImVec2& pos, const ImVec2& size, const ImVec4& tint, bool preserveColors) {
    if (!drawList) return;
    const float dpi = Context::Get().GetDpiScale();
    AtlasSubTexture sub = GetIcon(name, dpi, preserveColors);
    if (!sub.valid || !sub.srv) return;

    const ImVec2 renderSize = (size.x > 0.0f && size.y > 0.0f) ? size : ImVec2(sub.size.x * dpi, sub.size.y * dpi);
    drawList->AddImage(
        reinterpret_cast<ImTextureID>(sub.srv),
        pos,
        ImVec2(pos.x + renderSize.x, pos.y + renderSize.y),
        sub.uv0, sub.uv1,
        ImColor(tint)
    );
}

} // namespace parallax::icons
