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
	return Image{
		.data = PixelData<Pixel>{
			.pixels = pixels,
			.width = width,
			.height = height,
		},
	};
}

uint8_t Tile::Get(uint8_t x, uint8_t y) const
{
	assert(0 <= x && x < 8);
	assert(0 <= y && y < 8);
	uint16_t mask = 0b11 << x;
	return (data[y] & mask) >> x;
}

void Tile::Set(uint8_t x, uint8_t y, uint8_t val)
{
	assert(0 <= x && x < 8);
	assert(0 <= y && y < 8);
	assert(0 <= val && val < 4);
	// Get line
	uint16_t line = data[y];
	// Clear target bits by and-ing inverted mask
	line &= ~(0b11 << x);
	// Set target bits by or-ing, masking val to stop out of range effects
	line |= (val & 0b11) << x;
	// Set line
	data[y] = line;
}

void Tileset::AddSprite(const std::string& name, const Image& image)
{
	assert(image.data.height % 8 == 0);
	assert(image.data.width % 8 == 0);
	assert(!name.empty());
	// Prepare palette
	auto& pal = Palette::GetInstance();
	// Number of tiles in sprite
	size_t tHeight = image.data.height / 8;
	size_t tWidth = image.data.width / 8;
	Eigen::ArrayXX<uint16_t> nametable(tHeight, tWidth);
	// Iterate tiles
	for (size_t ty = 0; ty < tHeight; ty++) {
		for (size_t tx = 0; tx < tWidth; tx++) {
			// Iterate pixels
			Tile tile;
			for (size_t y = 0; y < 8; y++) {
				for (size_t x = 0; x < 8; x++) {
					size_t gy = ty * 8 + y;
					size_t gx = tx * 8 + x;
					auto [index, error] = pal.GetClosestColor(PixelToLab(image.data.pixels.at(gy * image.data.width + gx)), palette);
					tile.Set(x, y, index);
				}
			}
			// Tiles 
			// (tiles.size() is - conveniently - the next available tile index)
			auto [it, success] = tiles.insert({ tile, tiles.size() });
			nametable(ty, tx) = it->second;
		}
	}
	// Add nametable to map
	nametables[name] = nametable;
}

void Tileset::Save(const std::filesystem::path& path) const
{
}
