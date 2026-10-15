#include "parallax/icons/icon_loader.hpp"
#include <lunasvg.h>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <filesystem>
#include <windows.h>

namespace fs = std::filesystem;

namespace parallax::icons {

static std::string GetExecutableDir() {
    char path[MAX_PATH];
    GetModuleFileNameA(nullptr, path, MAX_PATH);
    std::string str(path);
    size_t pos = str.find_last_of("\\/");
    return (pos != std::string::npos) ? str.substr(0, pos) : "";
}

std::string IconLoader::ResolveIconPath(std::string_view name) {
    std::string filename(name);
    if (!filename.ends_with(".svg")) {
        filename += ".svg";
    }

    std::string exeDir = GetExecutableDir();
    const std::vector<std::string> searchPaths = {
        "./icons/" + filename,
        "../icons/" + filename,
        "../../icons/" + filename,
        "./assets/icons/" + filename,
        exeDir + "/icons/" + filename,
        exeDir + "/../icons/" + filename
    };

    for (const auto& candidate : searchPaths) {
        if (fs::exists(candidate)) {
            return candidate;
        }
    }

    return "";
}

std::string IconLoader::ReadFileContent(const std::string& fullPath) {
    std::ifstream file(fullPath, std::ios::in | std::ios::binary);
    if (!file) return "";
    std::ostringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

std::string IconLoader::NormalizeSvgContent(std::string svg) {
    // Replace "currentColor" with "#FFFFFF" so Lucide stroke-based icons render pure white
    size_t pos = 0;
    while ((pos = svg.find("currentColor", pos)) != std::string::npos) {
        svg.replace(pos, 12, "#FFFFFF");
        pos += 7;
    }
    return svg;
}

const char* IconLoader::GetFallbackSVG() noexcept {
    // Clean, modern geometric shield as universal fallback
    return R"(<svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="#FFFFFF" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M12 22s8-4 8-10V5l-8-3-8 3v7c0 6 8 10 8 10z"/></svg>)";
}

bool IconLoader::RasterizeSVG(
    std::string_view pathOrName,
    uint32_t targetWidth,
    uint32_t targetHeight,
    bool preserveColors,
    std::vector<uint8_t>& outRgbaPixels
) {
    if (targetWidth == 0 || targetHeight == 0) {
        targetWidth = 24;
        targetHeight = 24;
    }

    std::string resolvedPath = ResolveIconPath(pathOrName);
    std::string svgData;

    if (!resolvedPath.empty()) {
        svgData = ReadFileContent(resolvedPath);
    }

    if (svgData.empty()) {
        svgData = GetFallbackSVG();
    } else {
        svgData = NormalizeSvgContent(std::move(svgData));
    }

    auto doc = lunasvg::Document::loadFromData(svgData);
    if (!doc) {
        doc = lunasvg::Document::loadFromData(GetFallbackSVG());
        if (!doc) return false;
    }

    auto bitmap = doc->renderToBitmap(targetWidth, targetHeight);
    if (bitmap.isNull()) return false;

    const uint32_t bmpWidth = bitmap.width();
    const uint32_t bmpHeight = bitmap.height();
    const uint32_t bmpStride = bitmap.stride();
    const uint8_t* srcPixels = bitmap.data();

    outRgbaPixels.resize(static_cast<size_t>(bmpWidth) * bmpHeight * 4);

    for (uint32_t y = 0; y < bmpHeight; ++y) {
        const uint8_t* rowSrc = srcPixels + (y * bmpStride);
        uint8_t* rowDst = outRgbaPixels.data() + (y * bmpWidth * 4);

        for (uint32_t x = 0; x < bmpWidth; ++x) {
            // LunaSVG outputs BGRA premultiplied in memory
            const uint8_t b = rowSrc[x * 4 + 0];
            const uint8_t g = rowSrc[x * 4 + 1];
            const uint8_t r = rowSrc[x * 4 + 2];
            const uint8_t a = rowSrc[x * 4 + 3];

            if (preserveColors) {
                // Swapped to standard RGBA for DirectX 11
                rowDst[x * 4 + 0] = r;
                rowDst[x * 4 + 1] = g;
                rowDst[x * 4 + 2] = b;
                rowDst[x * 4 + 3] = a;
            } else {
                // Alpha mask: pure white RGB with opacity in alpha
                // Coverage captures stroke antialiasing flawlessly
                const uint8_t coverage = std::max(a, std::max(r, std::max(g, b)));
                rowDst[x * 4 + 0] = 255;
                rowDst[x * 4 + 1] = 255;
                rowDst[x * 4 + 2] = 255;
                rowDst[x * 4 + 3] = coverage;
            }
        }
    }

    return true;
}

} // namespace parallax::icons
