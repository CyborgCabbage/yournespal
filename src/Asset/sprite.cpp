#include "sprite.h"

// List of common image formats that are not supported
std::vector<std::string> unsupportedImageFormats{
	".bmp",
	".gif",
	".jpeg",
	".jpg",
	".webp",
	".pcx",
};

std::expected<Sprite, LoadSpriteError> Sprite::TryLoadSprite(const std::filesystem::path& path)
{
	// TEST: Directory or file with no extension
	if (!path.has_extension()) {
		return std::unexpected(LoadSpriteError::UNSUPPORTED_FORMAT);
	}
	auto ext = path.extension().string();
	// TEST: Capitalised extension name?
	if (ext != ".png") {
		// Warn user if they are using an unknown image format
		if (auto result = std::ranges::find(unsupportedImageFormats, ext); result != unsupportedImageFormats.end()) {
			std::println("Image format of {0} is not supported, use .png", path.string());
		}
		return std::unexpected(LoadSpriteError::UNSUPPORTED_FORMAT);
	}
	std::vector<unsigned char> png;
	if (unsigned error = lodepng::load_file(png, path.string())) {
		std::println("lodepng: Failed to load {0}, error {1} - {2}", path.string(), error, lodepng_error_text(error));
		return std::unexpected(LoadSpriteError::LOAD_FAILED);
	}
	SpriteData sd;
	if (unsigned error = lodepng::decode(sd.data, sd.width, sd.height, sd.state, png)) {
		std::println("lodepng: Failed to decode {0}, error {1} - {2}", path.string(), error, lodepng_error_text(error));
		return std::unexpected(LoadSpriteError::DECODE_FAILED);
	}
	return Sprite(sd, path);
}

Sprite::Sprite(SpriteData data, std::filesystem::path path) :
	data_(data),
	path_(path)
{
}
