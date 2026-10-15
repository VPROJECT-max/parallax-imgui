#include "parallax/core/context.hpp"
#include "parallax/core/style.hpp"
#include "parallax/anim/manager.hpp"
#include "parallax/icons/icon_atlas.hpp"
#include "parallax/input/hotkeys.hpp"

namespace parallax {

Context::Context()
    : m_anim(std::make_unique<anim::AnimationManager>()),
      m_icons(std::make_unique<icons::IconAtlas>()),
      m_input(std::make_unique<input::InputHandler>())
{
    Profile.username = "Alex Mercer";
    Profile.subtitle = "Pro Developer";
    Profile.icon = "user";
    Profile.isOnline = true;
}

Context::~Context() {
    Shutdown();
}

Context& Context::Get() noexcept {
    static Context instance;
    return instance;
}

void Context::Initialize(ID3D11Device* device, ID3D11DeviceContext* context) {
    if (m_initialized) return;

    if (device && context) {
        m_icons->Initialize(device, context);
    }

    StyleManager::Get().ApplyToImGui();
    m_initialized = true;
}

void Context::Shutdown() {
    if (!m_initialized) return;

    if (m_icons) {
        m_icons->Shutdown();
    }
    m_initialized = false;
}

anim::AnimationManager& Context::Anim() noexcept {
    return *m_anim;
}

icons::IconAtlas& Context::Icons() noexcept {
    return *m_icons;
}

input::InputHandler& Context::Input() noexcept {
    return *m_input;
}

} // namespace parallax
