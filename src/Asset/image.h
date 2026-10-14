#pragma once

#include <Asset/pixel.h>
#include <common.h>

namespace Asset {

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

}