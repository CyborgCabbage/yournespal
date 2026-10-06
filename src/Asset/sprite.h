#pragma once

#include <common.h>

struct SpriteData {
	// Contains pixels RGBARGBA...
	std::vector<unsigned char> data;
	lodepng::State state;
	unsigned width = 0;
	unsigned height = 0;
};

enum class LoadSpriteError {
	NOT_IMAGE,
	UNSUPPORTED_FORMAT,
	LOAD_FAILED,
	DECODE_FAILED
};

struct alignas(4) Pixel {
	auto operator<=>(const Pixel&) const = default;
	static float diff(const Pixel& a, const Pixel& b);
	uint8_t r = 0;
	uint8_t g = 0;
	uint8_t b = 0;
	uint8_t a = 0;
};

class Sprite {
public:
	Sprite(SpriteData data, std::filesystem::path path);
	// Prevent copying
	Sprite(const Sprite&) = delete;
	Sprite& operator= (const Sprite&) = delete;
	// Access
	std::span<Pixel> GetPixelSpan();
	std::span<Pixel> GetPaletteSpan();
	
	SpriteData data;
	std::filesystem::path path;

	std::vector<uint32_t> quantizedImage;

	static std::expected<std::unique_ptr<Sprite>, LoadSpriteError> TryLoadSprite(const std::filesystem::path& path);
	static std::vector<uint32_t> QuantizeImage(std::span<Pixel> pixels, std::span<Pixel> palette);
	static std::vector<Pixel> GeneratePalette(std::span<Pixel> pixels);
};