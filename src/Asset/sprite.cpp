#include "sprite.h"
#include <App/app.h>
#include <Core/log.h>

template <>
struct std::hash<Pixel> {
    std::size_t operator()(const Pixel& s) const noexcept
    {
        return std::hash<uint32_t>{ }(std::bit_cast<uint32_t>(s));
    }
};

template <>
struct std::formatter<Pixel> : std::formatter<std::string> {
    auto format(Pixel p, format_context& ctx) const
    {
        return formatter<string>::format(std::format("({}, {}, {}, {})", p.r, p.g, p.b, p.a), ctx);
    }
};

float Pixel::diff(const Pixel& a, const Pixel& b)
{
    // Implements redmean
    float rBar = (a.r + b.r) / 2;
    float rD = a.r - b.r;
    float gD = a.g - b.g;
    float bD = a.b - b.b;
    return std::sqrt((2 + rBar / 256) * rD * rD + 4 * gD * gD + (2 + (255 - rBar) / 256) * bD * bD);
}

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

Sprite Sprite::FromImage(const Image& image, std::span<Pixel> palette)
{
    std::vector<uint32_t> result;
    result.reserve(image.data.pixels.size());
    for (Pixel p : image.data.pixels) {
        float minError = std::numeric_limits<float>::max();
        size_t minIndex = -1;
        for (size_t i = 0; i < palette.size() && minError > 0; i++) {
            float newError = Pixel::diff(p, palette[i]);
            if (newError < minError) {
                minError = newError;
                minIndex = i;
            }
        }
        assert(minIndex != -1);
        result.push_back(minIndex);
    }
    return Sprite{
        .data = PixelData<uint32_t>{
            .pixels = result,
            .width = image.data.width,
            .height = image.data.height,
        },
        .id = image.id
    };
}

void Sprite::SaveSprite(const std::filesystem::path& directory, std::span<Pixel> palette) const
{
    std::vector<unsigned char> result;
    result.reserve(data.pixels.size() * 4);
    for (auto index : data.pixels) {
        auto& color = palette[index];
        result.push_back(color.r);
        result.push_back(color.g);
        result.push_back(color.b);
        result.push_back(color.a);
    }
    auto filepath = directory / (id + ".png");
    lodepng::encode(filepath.string(), result, data.width, data.height);
}