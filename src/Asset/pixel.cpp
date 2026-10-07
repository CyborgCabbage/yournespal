#include "pixel.h"

template <>
struct std::hash<Pixel> {
    std::size_t operator()(const Pixel& s) const noexcept
    {
        return std::hash<uint32_t>{ }(std::bit_cast<uint32_t>(s));
    }
};

template <>
struct std::formatter<Pixel> : std::formatter<std::string> {
    auto format(Pixel p, format_context& ctx) const
    {
        return formatter<string>::format(std::format("({}, {}, {}, {})", p.r, p.g, p.b, p.a), ctx);
    }
};

// Applies transfer function (gamma) 
// https://en.wikipedia.org/wiki/SRGB#Transfer_function_(%22gamma%22)
static float ColorByteToLinear(uint8_t inputColor) {
    float value = inputColor / 255.0f;
    if (value <= 0.04045f) {
        value /= 12.92f;
    }
    else {
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
    }
    else {
        value = 1.055f * std ::pow(value, 1.0f / 2.4f) - 0.055f;
    }
    return std::clamp(static_cast<int>(value * 255), 0, 255);
}

Lab PixelToLab(Pixel p)
{
    return RgbToLab(PixelToRgb(p));
}

Pixel LabToPixel(Lab p)
{
    return RgbToPixel(LabToRgb(p));
}

RGB PixelToRgb(Pixel p)
{
    return RGB(ColorByteToLinear(p.r), ColorByteToLinear(p.g), ColorByteToLinear(p.b));
}

Pixel RgbToPixel(RGB p)
{
    return Pixel(ColorLinearToByte(p.r), ColorLinearToByte(p.g), ColorLinearToByte(p.b), 255);
}

// Oklab conversions from https://bottosson.github.io/posts/oklab/#converting-from-linear-srgb-to-oklab
Lab RgbToLab(RGB c)
{
    float l = 0.4122214708f * c.r + 0.5363325363f * c.g + 0.0514459929f * c.b;
    float m = 0.2119034982f * c.r + 0.6806995451f * c.g + 0.1073969566f * c.b;
    float s = 0.0883024619f * c.r + 0.2817188376f * c.g + 0.6299787005f * c.b;

    float l_ = cbrtf(l);
    float m_ = cbrtf(m);
    float s_ = cbrtf(s);

    return {
        0.2104542553f * l_ + 0.7936177850f * m_ - 0.0040720468f * s_,
        1.9779984951f * l_ - 2.4285922050f * m_ + 0.4505937099f * s_,
        0.0259040371f * l_ + 0.7827717662f * m_ - 0.8086757660f * s_,
    };
}

RGB LabToRgb(Lab c)
{
    float l_ = c.L + 0.3963377774f * c.a + 0.2158037573f * c.b;
    float m_ = c.L - 0.1055613458f * c.a - 0.0638541728f * c.b;
    float s_ = c.L - 0.0894841775f * c.a - 1.2914855480f * c.b;

    float l = l_ * l_ * l_;
    float m = m_ * m_ * m_;
    float s = s_ * s_ * s_;

    return {
        +4.0767416621f * l - 3.3077115913f * m + 0.2309699292f * s,
        -1.2684380046f * l + 2.6097574011f * m - 0.3413193965f * s,
        -0.0041960863f * l - 0.7034186147f * m + 1.7076147010f * s,
    };
}

float ColorDiff(Lab a, Lab b)
{
    float LD = a.L - b.L;
    float aD = a.a - b.a;
    float bD = a.b - b.b;
    return std::sqrt(LD * LD + aD * aD + bD * bD);
}
