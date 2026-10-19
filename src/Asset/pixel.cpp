#include "pixel.h"

using namespace Asset;

Vector3f Vector3f::operator+(const Vector3f& other)
{
	return Vector3f{
		.x = x + other.x,
		.y = y + other.y,
		.z = z + other.z
	};
}

Vector3f Vector3f::operator-(const Vector3f& other)
{
	return Vector3f{
		.x = x - other.x,
		.y = y - other.y,
		.z = z - other.z
	};
}

Vector3f Vector3f::operator*(float scalar)
{
	return Vector3f{
		.x = x * scalar,
		.y = y * scalar,
		.z = z * scalar
	};
}

Vector3f Vector3f::operator/(float scalar)
{
	return Vector3f{
		.x = x / scalar,
		.y = y / scalar,
		.z = z / scalar
	};
}

float Vector3f::length() const
{
	return std::sqrt(x * x + y * y + z * z);
}

// Applies transfer function (gamma)
// https://en.wikipedia.org/wiki/SRGB#Transfer_function_(%22gamma%22)
static float ColorByteToLinear(uint8_t inputColor)
{
	float value = inputColor / 255.0f;
	if (value <= 0.04045f) {
		value /= 12.92f;
	} else {
		value = std::pow((value + 0.055f) / 1.055f, 2.4f);
	}
	return std::clamp(value, 0.0f, 1.0f);
}

// Inverse transfer function
static uint8_t ColorLinearToByte(float inputColor)
{
	float value = inputColor;
	if (value <= 0.0031308f) {
		value *= 12.92f;
	} else {
		value = 1.055f * std ::pow(value, 1.0f / 2.4f) - 0.055f;
	}
	return std::clamp(static_cast<int>(value * 255), 0, 255);
}

Lab Asset::PixelToLab(Pixel p)
{
	return RgbToLab(PixelToRgb(p));
}

Pixel Asset::LabToPixel(Lab p)
{
	return RgbToPixel(LabToRgb(p));
}

RGB Asset::PixelToRgb(Pixel p)
{
	return RGB(ColorByteToLinear(p.r), ColorByteToLinear(p.g), ColorByteToLinear(p.b));
}

Pixel Asset::RgbToPixel(RGB p)
{
	return Pixel(ColorLinearToByte(p.x), ColorLinearToByte(p.y), ColorLinearToByte(p.z), 255);
}

// Oklab conversions from https://bottosson.github.io/posts/oklab/#converting-from-linear-srgb-to-oklab
Lab Asset::RgbToLab(RGB c)
{
	float l = 0.4122214708f * c.x + 0.5363325363f * c.y + 0.0514459929f * c.z;
	float m = 0.2119034982f * c.x + 0.6806995451f * c.y + 0.1073969566f * c.z;
	float s = 0.0883024619f * c.x + 0.2817188376f * c.y + 0.6299787005f * c.z;

	float l_ = cbrtf(l);
	float m_ = cbrtf(m);
	float s_ = cbrtf(s);

	return {
		0.2104542553f * l_ + 0.7936177850f * m_ - 0.0040720468f * s_,
		1.9779984951f * l_ - 2.4285922050f * m_ + 0.4505937099f * s_,
		0.0259040371f * l_ + 0.7827717662f * m_ - 0.8086757660f * s_,
	};
}

RGB Asset::LabToRgb(Lab c)
{
	float l_ = c.x + 0.3963377774f * c.y + 0.2158037573f * c.z;
	float m_ = c.x - 0.1055613458f * c.y - 0.0638541728f * c.z;
	float s_ = c.x - 0.0894841775f * c.y - 1.2914855480f * c.z;

	float l = l_ * l_ * l_;
	float m = m_ * m_ * m_;
	float s = s_ * s_ * s_;

	return {
		+4.0767416621f * l - 3.3077115913f * m + 0.2309699292f * s,
		-1.2684380046f * l + 2.6097574011f * m - 0.3413193965f * s,
		-0.0041960863f * l - 0.7034186147f * m + 1.7076147010f * s,
	};
}