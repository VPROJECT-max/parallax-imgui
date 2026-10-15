#pragma once

#include <windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include <cstdint>
#include <functional>

namespace parallax::dx11 {

class D3D11Host {
public:
    D3D11Host();
    ~D3D11Host();

    // Standalone host window
    bool InitStandalone(HINSTANCE hInstance, int width, int height, const wchar_t* title);

    // Existing host hook mode (e.g. game or CAD Present hook)
    bool InitExisting(ID3D11Device* device, ID3D11DeviceContext* context, IDXGISwapChain* swapChain);

    void Resize(UINT width, UINT height);
    void BeginFrame();
    void EndFrame(bool vsync = true);
    void Shutdown();

    bool ProcessMessages();

    HWND GetHwnd() const noexcept { return m_hwnd; }
    ID3D11Device* GetDevice() const noexcept { return m_device; }
    ID3D11DeviceContext* GetContext() const noexcept { return m_context; }
    IDXGISwapChain* GetSwapChain() const noexcept { return m_swapChain; }
    ID3D11RenderTargetView* GetRenderTargetView() const noexcept { return m_renderTargetView; }

    uint32_t GetWidth() const noexcept { return m_width; }
    uint32_t GetHeight() const noexcept { return m_height; }

    using RenderCallback = std::function<void()>;
    void RunMainLoop(RenderCallback callback);

private:
    bool CreateDeviceAndSwapChain(HWND hwnd, int width, int height);
    void CreateRenderTarget();
    void CleanupRenderTarget();

    static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

    HWND m_hwnd{nullptr};
    HINSTANCE m_hInstance{nullptr};
    bool m_ownsWindow{false};

    ID3D11Device* m_device{nullptr};
    ID3D11DeviceContext* m_context{nullptr};
    IDXGISwapChain* m_swapChain{nullptr};
    ID3D11RenderTargetView* m_renderTargetView{nullptr};

    uint32_t m_width{1280};
    uint32_t m_height{720};
    bool m_running{false};
};

} // namespace parallax::dx11
