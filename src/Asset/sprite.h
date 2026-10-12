#pragma once

#include <Asset/pixel.h>
#include <common.h>

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
};

struct Sprite {
	static Sprite FromImage(const Image& image);
	void SaveSprite(const std::filesystem::path& path) const;
	PixelData<size_t> data;
	std::string id;
};