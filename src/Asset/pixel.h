#pragma once

#include <common.h>

namespace Asset {

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

}

template <>
struct std::hash<Asset::Pixel> {
	std::size_t operator()(const Asset::Pixel& s) const noexcept
	{
		return std::hash<uint32_t>{ }(std::bit_cast<uint32_t>(s));
	}
};

template <>
struct std::formatter<Asset::Pixel> : std::formatter<std::string> {
	auto format(Asset::Pixel p, format_context& ctx) const
	{
		return formatter<string>::format(std::format("({}, {}, {}, {})", p.r, p.g, p.b, p.a), ctx);
	}
};