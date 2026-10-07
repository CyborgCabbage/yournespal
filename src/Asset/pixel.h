#pragma once

#include <common.h>

struct alignas(4) Pixel {
    auto operator<=>(const Pixel&) const = default;
    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;
    uint8_t a = 0;
};

struct Lab {
    float L;
    float a;
    float b;
};

// Linear RGB
struct RGB {
    float r;
    float g;
    float b;
};

Lab PixelToLab(Pixel p);
Pixel LabToPixel(Lab p);
RGB PixelToRgb(Pixel p);
Pixel RgbToPixel(RGB p);
Lab RgbToLab(RGB c);
RGB LabToRgb(Lab c);

float ColorDiff(Lab a, Lab b);