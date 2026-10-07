#include "sprite.h"
#include <App/app.h>
#include <Core/log.h>
#include <Asset/palette.h>

// List of common image formats that are not supported
std::vector<std::string> unsupportedImageFormats{
    ".bmp",
    ".gif",
    ".jpeg",
    ".jpg",
    ".webp",
    ".pcx",
};

std::expected<Image, LoadImageError> Image::FromFile(const std::filesystem::path& path)
{
    // Ignore directories and extensionless files
    if (!path.has_extension() || std::filesystem::is_directory(path)) {
        return std::unexpected(LoadImageError::NOT_IMAGE);
    }
    // Check if a file is not a png
    // (TryLoadSprite is ran on every file in the directory so this will trigger often)
    auto ext = path.extension().string();
    // Case normalisation
    std::ranges::transform(ext, ext.begin(), [](unsigned char c) { return std::tolower(c); });
    if (ext != ".png") {
        // Warn user if they are using an unknown image format
        if (auto result = std::ranges::find(unsupportedImageFormats, ext); result != unsupportedImageFormats.end()) {
            ynp::LogLn("Warning: {0} Image format is not supported, use .png", path.string());
            return std::unexpected(LoadImageError::UNSUPPORTED_FORMAT);
        }
        return std::unexpected(LoadImageError::NOT_IMAGE);
    }
    // Load file into memory
    std::vector<unsigned char> png;
    if (unsigned error = lodepng::load_file(png, path.string())) {
        ynp::LogLn("Error: {0} {1}", path.string(), lodepng_error_text(error));
        return std::unexpected(LoadImageError::LOAD_FAILED);
    }
    // Decode PNG data (doesn't impose any format restriction or conversions)
    std::vector<unsigned char> pixelData;
    unsigned width, height;
    lodepng::State state;
    if (unsigned error = lodepng::decode(pixelData, width, height, state, png)) {
        ynp::LogLn("Error: {0} {1}", path.string(), lodepng_error_text(error));
        return std::unexpected(LoadImageError::DECODE_FAILED);
    }
    // Get pixels in vector
    std::vector<Pixel> pixels;
    pixels.assign_range(std::span<Pixel>(reinterpret_cast<Pixel*>(pixelData.data()), pixelData.size() / 4));
    // Get palette in vector
    std::vector<Pixel> palette;
    auto& color = state.info_png.color;
    palette.assign_range(std::span<Pixel>(reinterpret_cast<Pixel*>(color.palette), color.palettesize));
    return Image{
        .data = PixelData<Pixel>{
            .pixels = pixels,
            .width = width,
            .height = height,
        },
        .sourcePalette = palette,
        .id = path.stem().string()
    };
}

Sprite Sprite::FromImage(const Image& image)
{
    auto& pal = Palette::GetInstance();
    std::vector<std::pair<uint8_t, uint8_t>> result;
    result.reserve(image.data.pixels.size());
    for (Pixel p : image.data.pixels) {
        Lab c = PixelToLab(p);
        float minError = std::numeric_limits<float>::max();
        float minError2 = std::numeric_limits<float>::max();
        size_t minIndex = -1;
        size_t minIndex2 = -1;
        for (size_t i = 0; i < pal.lab.size() && minError > 0; i++) {
            float newError = ColorDiff(c, pal.lab[i]);
            // Patch to skip duplicates
            if (newError == minError) {
                continue;
            }
            if (newError < minError) {
                // Move min error down
                minError2 = minError;
                minIndex2 = minIndex;
                // Replace min error
                minError = newError;
                minIndex = i;
            } else if (newError < minError2) {
                minError2 = newError;
                minIndex2 = i;
            }
        }
        // Whether to include secondary color or not for dithering
        // Checks that minError2 isn't more than %150 minError
        if (minError2 / minError > 1.2f) {
            minIndex2 = minIndex;
        }
        assert(minIndex != -1);
        result.push_back({ minIndex, minIndex2 });
    }
    return Sprite{
        .data = PixelData<std::pair<uint8_t, uint8_t>>{
            .pixels = result,
            .width = image.data.width,
            .height = image.data.height,
        },
        .id = image.id
    };
}

void Sprite::SaveSprite(const std::filesystem::path& directory) const
{
    auto& pal = Palette::GetInstance();
    std::vector<unsigned char> result;
    result.reserve(data.pixels.size() * 4);
    bool toggle = false;
    for (auto [first, second] : data.pixels) {
        auto& color = pal.raw[toggle ? first : second];
        toggle = !toggle;
        result.push_back(color.r);
        result.push_back(color.g);
        result.push_back(color.b);
        result.push_back(color.a);
    }
    auto filepath = directory / (id + ".png");
    lodepng::encode(filepath.string(), result, data.width, data.height);
}