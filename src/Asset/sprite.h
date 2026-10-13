#pragma once

#include <Asset/pixel.h>
#include <common.h>

enum class LoadImageError {
	NOT_IMAGE,
	UNSUPPORTED_FORMAT,
	LOAD_FAILED,
	DECODE_FAILED
};

// Could pull out a base class here
template <typename Type>
struct PixelData {
	std::vector<Type> pixels;
	unsigned width, height;
};

struct Image {
	static std::expected<Image, LoadImageError> FromFile(const std::filesystem::path& path);
	PixelData<Pixel> data;
};

// 8 by 8 array of 2 bit uints
// (Aligned to 8 bytes so it can be bit cast to uint64_t for hashing)
struct alignas(8) Tile {
	auto operator<=>(const Tile&) const = default;
	uint8_t Get(uint8_t x, uint8_t y) const;
	void Set(uint8_t x, uint8_t y, uint8_t val);
	// 16 bytes for 
	std::array<uint16_t, 8> data = { };
};


template <>
struct std::hash<Tile> {
	std::size_t operator()(const Tile& s) const noexcept
	{
		using HashPair = std::pair<uint64_t, uint64_t>;
		size_t seed = std::hash<uint64_t>{ }(std::bit_cast<HashPair>(s).first);
		size_t b = std::hash<uint64_t>{ }(std::bit_cast<HashPair>(s).second);
		// Source - https://stackoverflow.com/a/2595226
		return seed ^ b + 0x9e3779b9 + (seed << 6) + (seed >> 2);
	}
};

struct Tileset {
	void AddSprite(const std::string& name, const Image& image);
	void Save(const std::filesystem::path& path) const;
	std::unordered_map<Tile, uint16_t> tiles;
	std::array<uint8_t, 4> palette = { };
	std::unordered_map<std::string, Eigen::ArrayXX<uint16_t>> nametables;
};
