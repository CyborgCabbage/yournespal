#include "sprite.h"
#include <App/app.h>
#include <Asset/palette.h>
#include <Core/core.h>

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
	auto ext = ynp::ToLowercase(path.extension().string());
	// Case normalisation
	if (ext != ".png") {
		// Warn user if they are using an unknown image format
		if (auto result = std::ranges::find(unsupportedImageFormats, ext); result != unsupportedImageFormats.end()) {
			ynp::LogLn("Warning: {0} Image format is not supported, use .png", path.string());
			return std::unexpected(LoadImageError::UNSUPPORTED_FORMAT);
		}
		return std::unexpected(LoadImageError::NOT_IMAGE);
	}
	// Load file into memory
	std::vector<unsigned char> png;
	if (unsigned error = lodepng::load_file(png, path.string())) {
		ynp::LogLn("Error: {0} {1}", path.string(), lodepng_error_text(error));
		return std::unexpected(LoadImageError::LOAD_FAILED);
	}
	// Decode PNG data (doesn't impose any format restriction or conversions)
	std::vector<unsigned char> pixelData;
	unsigned width, height;
	lodepng::State state;
	if (unsigned error = lodepng::decode(pixelData, width, height, state, png)) {
		ynp::LogLn("Error: {0} {1}", path.string(), lodepng_error_text(error));
		return std::unexpected(LoadImageError::DECODE_FAILED);
	}
	// Get pixels in vector
	std::vector<Pixel> pixels;
	pixels.assign_range(std::span<Pixel>(reinterpret_cast<Pixel*>(pixelData.data()), pixelData.size() / 4));
	// Get palette in vector
	std::vector<Pixel> palette;
	auto& color = state.info_png.color;
	palette.assign_range(std::span<Pixel>(reinterpret_cast<Pixel*>(color.palette), color.palettesize));
	return Image{
		.data = PixelData<Pixel>{
			.pixels = pixels,
			.width = width,
			.height = height,
		},
		.sourcePalette = palette,
		.id = path.stem().string()
	};
}

Sprite Sprite::FromImage(const Image& image)
{
	auto& pal = Palette::GetInstance();
	std::vector<size_t> result;
	result.reserve(image.data.pixels.size());
	for (Pixel p : image.data.pixels) {
		result.push_back(pal.GetClosestDither(PixelToLab(p)).first);
	}
	return Sprite{
		.data = PixelData<size_t>{
			.pixels = result,
			.width = image.data.width,
			.height = image.data.height,
		},
		.id = image.id
	};
}

void Sprite::SaveSprite(const std::filesystem::path& directory) const
{
	auto& pal = Palette::GetInstance();
	std::vector<unsigned char> result;
	result.reserve(data.pixels.size() * 4);
	for (size_t y = 0; y < data.height; y++) {
		for (size_t x = 0; x < data.width; x++) {
			auto& entry = pal.dither[data.pixels[y * data.width + x]];
			bool dither = (x + y) % 2 == 0;
			auto color = pal.raw[dither ? entry.a : entry.b];
			result.push_back(color.r);
			result.push_back(color.g);
			result.push_back(color.b);
			result.push_back(color.a);
		}
	}
	auto filepath = directory / (id + ".png");
	lodepng::encode(filepath.string(), result, data.width, data.height);
}