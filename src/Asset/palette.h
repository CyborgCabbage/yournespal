#pragma once

#include <common.h>

#include <Asset/pixel.h>

struct DitherEntry {
	uint8_t a;
	uint8_t b;
	Lab average;
};

class Palette {
public:
	static const Palette& GetInstance();
	std::pair<size_t, float> GetClosestColor(Lab color) const;
	std::pair<size_t, float> GetClosestDither(Lab color) const;
	std::vector<Pixel> raw;
	std::vector<Lab> lab;
	std::vector<DitherEntry> dither;

	// Prevent accidental copying
	Palette(const Palette&) = delete;
	Palette& operator=(const Palette&) = delete;

private:
	// Make constructor private to prevent construction outside of GetInstance()
	Palette();
};