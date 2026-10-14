#include "image.h"

#include <Util/log.h>
#include <Util/validation.h>

using namespace Asset;


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
	auto ext = Util::ToLowercase(path.extension().string());
	// Case normalisation
	if (ext != ".png") {
		// Warn user if they are using an unknown image format
		if (auto result = std::ranges::find(unsupportedImageFormats, ext); result != unsupportedImageFormats.end()) {
			Util::LogLn("Warning: {0} Image format is not supported, use .png", path.string());
			return std::unexpected(LoadImageError::UNSUPPORTED_FORMAT);
		}
		return std::unexpected(LoadImageError::NOT_IMAGE);
	}
	// Load file into memory
	std::vector<unsigned char> png;
	if (unsigned error = lodepng::load_file(png, path.string())) {
		Util::LogLn("Error: {0} {1}", path.string(), lodepng_error_text(error));
		return std::unexpected(LoadImageError::LOAD_FAILED);
	}
	// Decode PNG data (doesn't impose any format restriction or conversions)
	std::vector<unsigned char> pixelData;
	unsigned width, height;
	lodepng::State state;
	if (unsigned error = lodepng::decode(pixelData, width, height, state, png)) {
		Util::LogLn("Error: {0} {1}", path.string(), lodepng_error_text(error));
		return std::unexpected(LoadImageError::DECODE_FAILED);
	}
	// Get pixels in vector
	std::vector<Pixel> pixels;
	pixels.assign_range(std::span<Pixel>(reinterpret_cast<Pixel*>(pixelData.data()), pixelData.size() / 4));
	// Get palette in vector
	return Image{
		.data = PixelData<Pixel>{
			.pixels = pixels,
			.width = width,
			.height = height,
		},
	};
}