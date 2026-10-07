#pragma once

#include <common.h>

#include <Asset/pixel.h>

class Palette {
public:
    static const Palette& GetInstance();
    std::vector<Pixel> raw;
    std::vector<Lab> lab;

    // Prevent accidental copying
    Palette(const Palette&) = delete;
    Palette& operator=(const Palette&) = delete;
private:
    // Make constructor private to prevent construction outside of GetInstance()
    Palette();
};