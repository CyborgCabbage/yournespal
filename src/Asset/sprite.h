#pragma once

#include <common.h>
#include <Asset/pixel.h>

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
    std::string id;
};

struct Sprite {
    static Sprite FromImage(const Image& image);
    void SaveSprite(const std::filesystem::path& path) const;
    PixelData<uint32_t> data;
    std::string id;
};