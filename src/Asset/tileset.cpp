#include "tileset.h"
#include <App/app.h>
#include <Asset/palette.h>

using namespace Asset;

uint8_t Tile::Get(uint8_t x, uint8_t y) const
{
	assert(0 <= x && x < 8);
	assert(0 <= y && y < 8);
	uint16_t mask = 0b11 << (x * 2);
	return (data[y] & mask) >> (x * 2);
}

void Tile::Set(uint8_t x, uint8_t y, uint8_t val)
{
	assert(0 <= x && x < 8);
	assert(0 <= y && y < 8);
	assert(0 <= val && val < 4);
	// Get line
	uint16_t line = data[y];
	// Clear target bits by and-ing inverted mask
	line &= ~(0b11 << (x * 2));
	// Set target bits by or-ing, masking val to stop out of range effects
	line |= (val & 0b11) << (x * 2);
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
	size_t tWidth = image.data.width / 8;
	size_t tHeight = image.data.height / 8;
	MDVector<uint16_t> nametable(tWidth, tHeight);
	// Iterate tiles
	for (size_t ty = 0; ty < tHeight; ty++) {
		for (size_t tx = 0; tx < tWidth; tx++) {
			// Iterate pixels
			Tile tile;
			for (size_t y = 0; y < 8; y++) {
				for (size_t x = 0; x < 8; x++) {
					size_t gy = ty * 8 + y;
					size_t gx = tx * 8 + x;
					auto [index, error] = pal.GetClosestColor(PixelToLab(image.data.span[gx, gy]), palette);
					tile.Set(x, y, index);
				}
			}
			// Tiles
			// (tiles.size() is - conveniently - the next available tile index)
			auto [it, success] = tiles.insert({ tile, tiles.size() });
			nametable.span[tx, ty] = it->second;
		}
	}
	// Add nametable to map
	nametables.try_emplace(name, std::move(nametable));
}

PaletteImage Tileset::ToPaletteImage() const
{
	assert(tiles.size() <= 256);// TODO: Handle case where there are too many tiles
	PaletteImage image{
		.data = MDVector<uint8_t>(128, 128),
		.palette = std::vector(palette.size(), Pixel{ })
	};
	auto& pal = Palette::GetInstance();
	for (size_t i = 0; i < palette.size(); i++) {
		image.palette[i] = pal.raw[palette[i]];
	}
	for (const auto& [tile, id] : tiles) {
		int xt = id % 16;
		int yt = id / 16;
		for (int y = 0; y < 8; y++) {
			for (int x = 0; x < 8; x++) {
				image.data.span[xt * 8 + x, yt * 8 + y] = tile.Get(x, y);
			}
		}
	}
	return image;
}
