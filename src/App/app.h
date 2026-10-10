#pragma once

#include <Asset/sprite.h>
#include <common.h>

enum class ApplicationRunError {
	ARGUMENTS_LENGTH_WRONG,
	PATH_INVALID,
	DATA_INVALID,
};

enum class ApplicationParseError {
	FILE_LOAD_FAILED,
	ARRAY_LENGTH_WRONG,
	VALUE_PREDICATE_FAILED,
	VALUE_TYPE_WRONG,
	FIELD_MISSING,
};

struct AppArguments {
	std::filesystem::path configFile;
	std::filesystem::path outputPath;
};



struct PaletteConfig {
	// REQUIRED
	std::array<uint8_t, 4> colors;
};

struct TilesetConfig {
	// Contained sprites
	// REQUIRED
	std::vector<std::string> sprites;
	// A predefined palette (will be derived from sprites if not given)
	// OPTIONAL
	std::optional<std::string> palette;
};

struct AppConfig {
	std::filesystem::path inputPath;
	std::filesystem::path outputPath;
	// Groups have an ID and each group contains a
	std::unordered_map<std::string, PaletteConfig> palettes;
	std::unordered_map<std::string, TilesetConfig> tilesets;
};

class Application {
public:
	static std::optional<ApplicationRunError> Run(const std::vector<std::string_view>& arguments);
	static std::string ErrorToString(ApplicationRunError error);
	static bool DoPathsOverlap(std::filesystem::path a, std::filesystem::path b);
	static std::expected<AppConfig, ApplicationParseError> ParseConfig(const AppArguments& arguments);
	static std::expected<PaletteConfig, ApplicationParseError> ParsePalette(const toml::table& tbl);
	static std::expected<TilesetConfig, ApplicationParseError> ParseTileset(const toml::table& tbl);

private:
	static std::expected<std::vector<Image>, ApplicationRunError> LoadImages(const std::filesystem::path& path);
	Application();
};