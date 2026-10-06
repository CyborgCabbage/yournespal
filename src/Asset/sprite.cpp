#include "sprite.h"
#include <Core/log.h>

template<>
struct std::hash<Pixel>
{
	std::size_t operator()(const Pixel& s) const noexcept
	{
		return std::hash<uint32_t>{}(std::bit_cast<uint32_t>(s));
	}
};

template <>
struct std::formatter<Pixel> : std::formatter<std::string> {
	auto format(Pixel p, format_context& ctx) const {
		return formatter<string>::format(std::format("({}, {}, {}, {})", p.r, p.g, p.b, p.a), ctx);
	}
};

float Pixel::diff(const Pixel& a, const Pixel& b) {
	// Implements redmean
	float rBar = (a.r + b.r) / 2;
	float rD = a.r - b.r;
	float gD = a.g - b.g;
	float bD = a.b - b.b;
	return std::sqrt((2+rBar/256)*rD*rD+4*gD*gD+(2+(255-rBar)/256)*bD*bD);
}

// List of common image formats that are not supported
std::vector<std::string> unsupportedImageFormats{
	".bmp",
	".gif",
	".jpeg",
	".jpg",
	".webp",
	".pcx",
};

std::expected<std::unique_ptr<Sprite>, LoadSpriteError> Sprite::TryLoadSprite(const std::filesystem::path& path)
{
	// Ignore directories and extensionless files
	if (!path.has_extension()) {
		return std::unexpected(LoadSpriteError::NOT_IMAGE);
	}
	// Check if a file is not a png
	// (TryLoadSprite is ran on every file in the directory so this will trigger often)
	auto ext = path.extension().string();
	// Case normalisation
	std::ranges::transform(ext, ext.begin(), [](unsigned char c) { return std::tolower(c); });
	if (ext != ".png") {
		// Warn user if they are using an unknown image format
		if (auto result = std::ranges::find(unsupportedImageFormats, ext); result != unsupportedImageFormats.end()) {
			ynp::LogLn("Warning: {0} Image format is not supported, use .png", path.string());
			return std::unexpected(LoadSpriteError::UNSUPPORTED_FORMAT);
		}
		return std::unexpected(LoadSpriteError::NOT_IMAGE);
	}
	// Load file into memory
	std::vector<unsigned char> png;
	if (unsigned error = lodepng::load_file(png, path.string())) {
		ynp::LogLn("Error: {0} {1}", path.string(), lodepng_error_text(error));
		return std::unexpected(LoadSpriteError::LOAD_FAILED);
	}
	// Decode PNG data (doesn't impose any format restriction or conversions)
	SpriteData sd;
	if (unsigned error = lodepng::decode(sd.data, sd.width, sd.height, sd.state, png)) {
		ynp::LogLn("Error: {0} {1}", path.string(), lodepng_error_text(error));
		return std::unexpected(LoadSpriteError::DECODE_FAILED);
	}

	return std::make_unique<Sprite>(sd, path);
}

std::vector<uint32_t> Sprite::QuantizeImage(std::span<Pixel> pixels, std::span<Pixel> palette) {
	std::vector<uint32_t> result;
	result.reserve(pixels.size());
	for (Pixel p : pixels) {
		float minError = std::numeric_limits<uint32_t>::max();
		size_t minIndex = -1;
		for (size_t i = 0; i < palette.size() && minError > 0; i++) {
			float newError = Pixel::diff(p, palette[i]);
			if (newError < minError) {
				minError = newError;
				minIndex = i;
			}
		}
		assert(minIndex != -1);
		result.push_back(minIndex);
	}
	return result;
}

std::span<Pixel> Sprite::GetPixelSpan()
{
	return std::span<Pixel>(reinterpret_cast<Pixel*>(data.data.data()), data.data.size() / 4);
}

std::span<Pixel> Sprite::GetPaletteSpan()
{
	auto& color = data.state.info_png.color;
	return std::span<Pixel>(reinterpret_cast<Pixel*>(color.palette), color.palettesize);
}

std::vector<Pixel> Sprite::GeneratePalette(std::span<Pixel> pixels)
{
	// No palette provided
	std::unordered_map<Pixel, unsigned> colors;
	for (Pixel p : pixels) {
		colors[p] += 1;
	}
	// Get list of unique colours
	std::vector<Pixel> palette;
	palette.reserve(colors.size());
	for (const auto& [pixel, count] : colors) {
		palette.push_back(pixel);
	}
	return palette;
}

Sprite::Sprite(SpriteData data, std::filesystem::path path) :
	data(data),
	path(path)
{
	auto& color = data.state.info_png.color;
	if (color.palettesize == 0) {
		quantizedPalette = GeneratePalette(GetPixelSpan());
	}
	else {
		quantizedPalette.assign_range(GetPaletteSpan());
	}
	quantizedImage = Sprite::QuantizeImage(GetPixelSpan(), std::span(quantizedPalette));
	ynp::LogLn("Palette: {0} colors", quantizedPalette.size());
}