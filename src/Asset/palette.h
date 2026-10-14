#pragma once

#include <common.h>

#include <Asset/pixel.h>

namespace Asset {

struct DitherEntry {
	uint8_t a;
	uint8_t b;
	Lab average;
};

constexpr size_t PALETTE_SIZE = 64;

class Palette {
public:
	static const Palette& GetInstance();
	std::pair<size_t, float> GetClosestColor(Lab color) const;
	std::pair<size_t, float> GetClosestColor(Lab color, const std::span<uint8_t>& subPalette) const;
	std::pair<size_t, float> GetClosestDither(Lab color) const;
	std::array<Pixel, PALETTE_SIZE> raw;
	std::array<Lab, PALETTE_SIZE> lab;
	std::array<uint8_t, PALETTE_SIZE> mapDuplicates;
	std::vector<DitherEntry> dither;

	// Prevent accidental copying
	Palette(const Palette&) = delete;
	Palette& operator=(const Palette&) = delete;

private:
	// Make constructor private to prevent construction outside of GetInstance()
	Palette();
};

}