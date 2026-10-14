#include "sprite.h"
#include <App/app.h>
#include <Asset/palette.h>

using namespace Asset;

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
