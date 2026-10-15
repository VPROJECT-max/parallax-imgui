#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <cstdint>

namespace parallax::icons {

class IconLoader {
public:
    static std::string ResolveIconPath(std::string_view name);

    static bool RasterizeSVG(
        std::string_view pathOrName,
        uint32_t targetWidth,
        uint32_t targetHeight,
        bool preserveColors,
        std::vector<uint8_t>& outRgbaPixels
    );

    static const char* GetFallbackSVG() noexcept;

private:
    static std::string ReadFileContent(const std::string& fullPath);
    static std::string NormalizeSvgContent(std::string svg);
};

} // namespace parallax::icons
