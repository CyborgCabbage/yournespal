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
	return Image{ .data = MDVector<Pixel>(width, height, std::move(pixels)) };
}

std::optional<SaveImageError> PaletteImage::ToFile(const std::filesystem::path& path)
{
	lodepng::State state;
	// Create palette
	for (Pixel p : palette) {
		// Palette must be created for both the format we provide the pixel data in
		// and the format it will be encoded into
		lodepng_palette_add(&state.info_png.color, p.r, p.g, p.b, p.a);
		lodepng_palette_add(&state.info_raw, p.r, p.g, p.b, p.a);
	}

	// We are encoding 8 bit palette data (MDVector<uint8_t>) into a 2-bit (NES) palette PNG
	state.info_png.color.colortype = LCT_PALETTE;
	state.info_png.color.bitdepth = 2;
	state.info_raw.colortype = LCT_PALETTE;
	state.info_raw.bitdepth = 8;
	state.encoder.auto_convert = 0;

	// encode and save
	std::vector<unsigned char> buffer;
	if (unsigned error = lodepng::encode(buffer, data.data, data.width, data.height, state)) {
		Util::LogLn("Error: {0} {1}", path.string(), lodepng_error_text(error));
		return SaveImageError::ENCODE_FAILED;
	}

	if (unsigned error = lodepng::save_file(buffer, path.string())) {
		Util::LogLn("Error: {0} {1}", path.string(), lodepng_error_text(error));
		return SaveImageError::ENCODE_FAILED;
	}
	return std::nullopt;
}