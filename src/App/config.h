#pragma once

#include <common.h>

namespace App {

struct AppArguments;

enum class ApplicationParseError {
	FILE_LOAD_FAILED,
	ARRAY_LENGTH_WRONG,
	VALUE_PREDICATE_FAILED,
	VALUE_TYPE_WRONG,
	FIELD_MISSING,
};

struct PaletteConfig {
	// REQUIRED
	std::array<std::optional<uint8_t>, 4> colors = { };
};

struct TilesetConfig {
	// Contained sprites
	// REQUIRED
	std::vector<std::string> sprites;
	// A predefined palette (will be derived from sprites if not given)
	// CAN BE EMPTY
	std::string palette;
};

struct AppConfig {
	std::string AddAnonymousPalette(std::string_view tilesetName);
	std::filesystem::path inputPath;
	std::filesystem::path outputPath;
	// Groups have an ID and each group contains a
	std::unordered_map<std::string, PaletteConfig> palettes;
	std::unordered_map<std::string, TilesetConfig> tilesets;
};

std::expected<AppConfig, ApplicationParseError> ParseConfig(const AppArguments& arguments);
std::expected<PaletteConfig, ApplicationParseError> ParsePalette(const toml::table& tbl);
std::expected<TilesetConfig, ApplicationParseError> ParseTileset(const toml::table& tbl);

}
