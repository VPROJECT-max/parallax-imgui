#pragma once

namespace parallax {

// Renders a sleek floating status watermark pill with app name, user info, and FPS stats
void Watermark(
    const char* appName = "PARALLAX",
    const char* user = "Alex Mercer",
    bool showFps = true,
    bool showPing = true
);

} // namespace parallax
