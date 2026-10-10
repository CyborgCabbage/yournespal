#pragma once

#include <common.h>

struct alignas(4) Pixel {
	auto operator<=>(const Pixel&) const = default;
	uint8_t r = 0;
	uint8_t g = 0;
	uint8_t b = 0;
	uint8_t a = 0;
};

using Lab = Eigen::Vector3f;
using RGB = Eigen::Vector3f;

Lab PixelToLab(Pixel p);
Pixel LabToPixel(Lab p);
RGB PixelToRgb(Pixel p);
Pixel RgbToPixel(RGB p);
Lab RgbToLab(RGB c);
RGB LabToRgb(Lab c);
