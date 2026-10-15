#include "parallax/dx11/d3d11_host.hpp"
#include <imgui.h>
#include <backends/imgui_impl_win32.h>
#include <backends/imgui_impl_dx11.h>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace parallax::dx11 {

D3D11Host::D3D11Host() = default;

D3D11Host::~D3D11Host() {
    Shutdown();
}

bool D3D11Host::InitStandalone(HINSTANCE hInstance, int width, int height, const wchar_t* title) {
    m_hInstance = hInstance ? hInstance : GetModuleHandle(nullptr);
    m_width = width > 100 ? width : 1280;
    m_height = height > 100 ? height : 720;
    m_ownsWindow = true;

    WNDCLASSEXW wc{};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.style = CS_CLASSDC;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = m_hInstance;
    wc.lpszClassName = L"ParallaxDX11HostClass";
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);

    RegisterClassExW(&wc);

    RECT wr = {0, 0, static_cast<LONG>(m_width), static_cast<LONG>(m_height)};
    AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);

    m_hwnd = CreateWindowExW(
        0,
        wc.lpszClassName,
        title ? title : L"Parallax UI DX11 Host",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        wr.right - wr.left, wr.bottom - wr.top,
        nullptr, nullptr,
        m_hInstance,
        this
    );

    if (!m_hwnd) return false;

    if (!CreateDeviceAndSwapChain(m_hwnd, m_width, m_height)) {
        DestroyWindow(m_hwnd);
        m_hwnd = nullptr;
        return false;
    }

    CreateRenderTarget();

    ShowWindow(m_hwnd, SW_SHOWDEFAULT);
    UpdateWindow(m_hwnd);

    // Initialize ImGui Backends
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui_ImplWin32_Init(m_hwnd);
    ImGui_ImplDX11_Init(m_device, m_context);

    m_running = true;
    return true;
}

bool D3D11Host::InitExisting(ID3D11Device* device, ID3D11DeviceContext* context, IDXGISwapChain* swapChain) {
    if (!device || !context || !swapChain) return false;

    m_device = device;
    m_device->AddRef();
    m_context = context;
    m_context->AddRef();
    m_swapChain = swapChain;
    m_swapChain->AddRef();
    m_ownsWindow = false;

    DXGI_SWAP_CHAIN_DESC sd;
    m_swapChain->GetDesc(&sd);
    m_hwnd = sd.OutputWindow;
    m_width = sd.BufferDesc.Width;
    m_height = sd.BufferDesc.Height;

    CreateRenderTarget();

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui_ImplWin32_Init(m_hwnd);
    ImGui_ImplDX11_Init(m_device, m_context);

    m_running = true;
    return true;
}

bool D3D11Host::CreateDeviceAndSwapChain(HWND hwnd, int width, int height) {
    DXGI_SWAP_CHAIN_DESC sd{};
    sd.BufferCount = 2;
    sd.BufferDesc.Width = width;
    sd.BufferDesc.Height = height;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hwnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT createDeviceFlags = 0;
#ifdef _DEBUG
    // createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevels[2] = {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_0
    };

    HRESULT hr = D3D11CreateDeviceAndSwapChain(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        createDeviceFlags,
        featureLevels, 2,
        D3D11_SDK_VERSION,
        &sd,
        &m_swapChain,
        &m_device,
        &featureLevel,
        &m_context
    );

    return SUCCEEDED(hr);
}

void D3D11Host::CreateRenderTarget() {
    if (!m_swapChain || !m_device) return;

    ID3D11Texture2D* pBackBuffer = nullptr;
    if (SUCCEEDED(m_swapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer)))) {
        m_device->CreateRenderTargetView(pBackBuffer, nullptr, &m_renderTargetView);
        pBackBuffer->Release();
    }
}

void D3D11Host::CleanupRenderTarget() {
    if (m_renderTargetView) {
        m_renderTargetView->Release();
        m_renderTargetView = nullptr;
    }
}

void D3D11Host::Resize(UINT width, UINT height) {
    if (!m_swapChain || width == 0 || height == 0) return;

    m_width = width;
    m_height = height;

    CleanupRenderTarget();
    m_swapChain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0);
    CreateRenderTarget();
}

void D3D11Host::BeginFrame() {
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
}

void D3D11Host::EndFrame(bool vsync) {
    ImGui::Render();

    const float clearColor[4] = {0.040f, 0.043f, 0.055f, 1.0f}; // Charcoal slate background
    m_context->OMSetRenderTargets(1, &m_renderTargetView, nullptr);
    m_context->ClearRenderTargetView(m_renderTargetView, clearColor);

    D3D11_VIEWPORT vp{};
    vp.Width = static_cast<float>(m_width);
    vp.Height = static_cast<float>(m_height);
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    m_context->RSSetViewports(1, &vp);

    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
    }

    m_swapChain->Present(vsync ? 1 : 0, 0);
}

bool D3D11Host::ProcessMessages() {
    MSG msg;
    while (PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
        if (msg.message == WM_QUIT) {
            m_running = false;
            return false;
        }
    }
    return m_running;
}

void D3D11Host::RunMainLoop(RenderCallback callback) {
    while (m_running) {
        if (!ProcessMessages()) break;

        BeginFrame();
        if (callback) {
            callback();
        }
        EndFrame(true);
    }
}

void D3D11Host::Shutdown() {
    m_running = false;

    if (ImGui::GetCurrentContext()) {
        ImGui_ImplDX11_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
    }

    CleanupRenderTarget();

    if (m_swapChain) {
        m_swapChain->Release();
        m_swapChain = nullptr;
    }
    if (m_context) {
        m_context->Release();
        m_context = nullptr;
    }
    if (m_device) {
        m_device->Release();
        m_device = nullptr;
    }

    if (m_ownsWindow && m_hwnd) {
        DestroyWindow(m_hwnd);
        UnregisterClassW(L"ParallaxDX11HostClass", m_hInstance);
        m_hwnd = nullptr;
    }
}

LRESULT CALLBACK D3D11Host::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    D3D11Host* host = nullptr;
    if (msg == WM_NCCREATE) {
        auto* cs = reinterpret_cast<CREATESTRUCT*>(lParam);
        host = reinterpret_cast<D3D11Host*>(cs->lpCreateParams);
        SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(host));
    } else {
        host = reinterpret_cast<D3D11Host*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
    }

    switch (msg) {
        case WM_SIZE:
            if (host && host->m_swapChain && wParam != SIZE_MINIMIZED) {
                host->Resize(LOWORD(lParam), HIWORD(lParam));
            }
            return 0;

        case WM_SYSCOMMAND:
            if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
                return 0;
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }

    return DefWindowProcW(hWnd, msg, wParam, lParam);
}

} // namespace parallax::dx11
