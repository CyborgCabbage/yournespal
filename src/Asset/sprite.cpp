#include "sprite.h"
#include <Core/log.h>

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

struct alignas(4) Pixel {
	auto operator<=>(const Pixel&) const = default;
	uint8_t r = 0;
	uint8_t g = 0;
	uint8_t b = 0;
	uint8_t a = 0;
};

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

Sprite::Sprite(SpriteData data, std::filesystem::path path) :
	data(data),
	path(path)
{
	ynp::LogLn("Sprite Created {0}", path.string());
	// Check for palette
	auto& color = data.state.info_png.color;
	if (color.palettesize > 0) {
		ynp::LogLn("Palette: {0} colors", color.palettesize);
		auto span = std::span<Pixel>(reinterpret_cast<Pixel*>(color.palette), color.palettesize);
		for (Pixel p : span) {
			ynp::LogLn("{0}", p);
		}
	}
}