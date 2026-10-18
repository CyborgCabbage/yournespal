#pragma once

#include <common.h>

#include <Asset/image.h>
#include <Asset/pixel.h>
#include <Util/mdvector.h>

namespace Asset {

// 8 by 8 array of 2 bit uints
// (Aligned to 8 bytes so it can be bit cast to uint64_t for hashing)
struct alignas(8) Tile {
	auto operator<=>(const Tile&) const = default;
	uint8_t Get(uint8_t x, uint8_t y) const;
	void Set(uint8_t x, uint8_t y, uint8_t val);
	// 16 bytes for
	std::array<uint16_t, 8> data = { };
};

}

template <>
struct std::hash<Asset::Tile> {
	std::size_t operator()(const Asset::Tile& s) const noexcept
	{
		using HashPair = std::pair<uint64_t, uint64_t>;
		size_t seed = std::hash<uint64_t>{ }(std::bit_cast<HashPair>(s).first);
		size_t b = std::hash<uint64_t>{ }(std::bit_cast<HashPair>(s).second);
		// Source - https://stackoverflow.com/a/2595226
		return seed ^ b + 0x9e3779b9 + (seed << 6) + (seed >> 2);
	}
};

namespace Asset {

struct Tileset {
	void AddSprite(const std::string& name, const Image& image);
	MDVector<uint8_t> ToPixels() const;
	std::unordered_map<Tile, uint16_t> tiles;
	std::array<uint8_t, 4> palette = { };
	std::unordered_map<std::string, MDVector<uint16_t>> nametables;
};

}