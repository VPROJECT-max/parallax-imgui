#pragma once

#include "parallax/core/types.hpp"
#include <memory>
#include <string>

struct ID3D11Device;
struct ID3D11DeviceContext;

namespace parallax {

namespace anim { class AnimationManager; }
namespace icons { class IconAtlas; }
namespace input { class InputHandler; }

class Context {
public:
    static Context& Get() noexcept;

    void Initialize(ID3D11Device* device, ID3D11DeviceContext* context);
    void Shutdown();

    bool IsInitialized() const noexcept { return m_initialized; }

    anim::AnimationManager& Anim() noexcept;
    icons::IconAtlas& Icons() noexcept;
    input::InputHandler& Input() noexcept;

    float GetDpiScale() const noexcept { return m_dpiScale; }
    void SetDpiScale(float scale) noexcept { m_dpiScale = scale > 0.25f ? scale : 1.0f; }

    // Global layout & session states
    NavSide CurrentNavSide{NavSide::Left};
    SubTabPlacement CurrentSubTabPlacement{SubTabPlacement::TopHorizontal};
    WindowSizeTier CurrentSizeTier{WindowSizeTier::Standard};
    SpeedMultiplier CurrentSpeedMultiplier{SpeedMultiplier::Normal};
    int CurrentLayoutPresetIndex{0}; // 0 = ModernSidebar, 1 = CompactTactical, 2 = MinimalTabbed

    // Configurable window size defaults and limits
    ImVec2 DefaultWindowSize{960.0f, 620.0f};
    ImVec2 MinWindowSize{740.0f, 480.0f};
    ImVec2 MaxWindowSize{1920.0f, 1080.0f};

    std::string SearchQuery;
    UserProfile Profile;

    bool MenuOpen{true};
    float MenuVisibilityAlpha{1.0f};
    float MenuScale{1.0f};

private:
    Context();
    ~Context();

    bool m_initialized{false};
    float m_dpiScale{1.0f};

    std::unique_ptr<anim::AnimationManager> m_anim;
    std::unique_ptr<icons::IconAtlas> m_icons;
    std::unique_ptr<input::InputHandler> m_input;
};

} // namespace parallax
