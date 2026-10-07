#pragma once

#include <common.h>

struct alignas(4) Pixel {
    auto operator<=>(const Pixel&) const = default;
    static float diff(const Pixel& a, const Pixel& b);
    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;
    uint8_t a = 0;
};

enum class LoadImageError {
    NOT_IMAGE,
    UNSUPPORTED_FORMAT,
    LOAD_FAILED,
    DECODE_FAILED
};

// Could pull out a base class here
template <typename Type>
struct PixelData {
    std::vector<Type> pixels;
    unsigned width, height;
};

struct Image {
    static std::expected<Image, LoadImageError> FromFile(const std::filesystem::path& path);
    PixelData<Pixel> data;
    // If it was a paletted image file, source palette contains the palette from their
    std::vector<Pixel> sourcePalette;
};

struct Sprite {
    static Sprite FromImage(const Image& image, const std::span<Pixel> palette);
    PixelData<uint32_t> data;
};