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
	// Ignore directories and extensionless files
	if (!path.has_extension()) {
		return std::unexpected(LoadSpriteError::NOT_IMAGE);
	}
	// Check if a file is not a png
	// (TryLoadSprite is ran on every file in the directory so this will trigger often)
	auto ext = path.extension().string();
	// Case normalisation
	std::ranges::transform(ext, ext.begin(), [](unsigned char c) { return std::tolower(c); });
	if (ext != ".png") {
		// Warn user if they are using an unknown image format
		if (auto result = std::ranges::find(unsupportedImageFormats, ext); result != unsupportedImageFormats.end()) {
			std::println("Image format is not supported, use .png");
			return std::unexpected(LoadSpriteError::UNSUPPORTED_FORMAT);
		}
		return std::unexpected(LoadSpriteError::NOT_IMAGE);
	}
	// Load file into memory
	std::vector<unsigned char> png;
	if (unsigned error = lodepng::load_file(png, path.string())) {
		std::println(lodepng_error_text(error));
		return std::unexpected(LoadSpriteError::LOAD_FAILED);
	}
	// Decode PNG data (doesn't impose any format restriction or conversions)
	SpriteData sd;
	if (unsigned error = lodepng::decode(sd.data, sd.width, sd.height, sd.state, png)) {
		std::println(lodepng_error_text(error));
		return std::unexpected(LoadSpriteError::DECODE_FAILED);
	}

	return Sprite(sd, path);
}

Sprite::Sprite(SpriteData data, std::filesystem::path path) :
	data_(data),
	path_(path)
{
}
