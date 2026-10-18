#pragma once

#include <Asset/pixel.h>
#include <Util/mdvector.h>
#include <common.h>

namespace Asset {

enum class LoadImageError {
	NOT_IMAGE,
	UNSUPPORTED_FORMAT,
	LOAD_FAILED,
	DECODE_FAILED
};

enum class SaveImageError {
	ENCODE_FAILED,
	SAVE_FAILED
};

struct Image {
	static std::expected<Image, LoadImageError> FromFile(const std::filesystem::path& path);
	MDVector<Pixel> data;
};

struct PaletteImage {
	std::optional<SaveImageError> ToFile(const std::filesystem::path& path);
	MDVector<uint8_t> data;
	std::vector<Pixel> palette;
};

}