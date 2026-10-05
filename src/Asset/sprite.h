#pragma once

#include <common.h>

struct SpriteData {
	// Contains pixels RGBARGBA...
	std::vector<unsigned char> data;
	lodepng::State state;
	unsigned width = 0;
	unsigned height = 0;
};

enum class LoadSpriteError {
	UNSUPPORTED_FORMAT,
	LOAD_FAILED,
	DECODE_FAILED
};

class Sprite {
public:
	static std::expected<Sprite, LoadSpriteError> TryLoadSprite(const std::filesystem::path& path);
private:
	Sprite(SpriteData data, std::filesystem::path path);
	SpriteData data_;
	std::filesystem::path path_;
};