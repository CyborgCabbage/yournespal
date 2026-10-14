#include "config.h"

#include <App/app.h>
#include <Util/log.h>

using namespace App;

void LogConfigLocation(const toml::source_region& region)
{
	Util::LogLn("[Line, Column] = [{0}, {1}] to [{2}, {3}]:", region.begin.line, region.begin.column, region.end.line, region.end.column);
}

std::expected<AppConfig, ApplicationParseError> App::ParseConfig(const AppArguments& arguments)
{
	// Load TOML file
	toml::table loaded;
	try {
		loaded = toml::parse_file(arguments.configFile.string());
	} catch (const toml::parse_error& err) {
		Util::LogLn("Error: Error loading config file '{0}' - {1}", arguments.configFile.string(), err.description());
		return std::unexpected(ApplicationParseError::FILE_LOAD_FAILED);
	}
	// File out config structure
	AppConfig config;
	config.inputPath = arguments.configFile.parent_path();
	config.outputPath = arguments.outputPath;
	// Load palettes
	if (const toml::table* palettes = loaded.get_as<toml::table>("palette")) {
		for (const auto& pair : *palettes) {
			const toml::key& key = pair.first;
			const toml::table* val = pair.second.as_table();
			if (!val) {
				Util::LogLn("Error: Expected palette to be table at location:");
				LogConfigLocation(pair.second.source());
				return std::unexpected(ApplicationParseError::VALUE_TYPE_WRONG);
			}
			auto palette = ParsePalette(*val);
			if (!palette.has_value()) {
				return std::unexpected(palette.error());
			}
			config.palettes.insert(std::pair<std::string, PaletteConfig>(key.str(), palette.value()));
		}
	}
	// Load tilesets
	if (auto* tilesets = loaded.get_as<toml::table>("tileset")) {
		for (const auto& pair : *tilesets) {
			const toml::key& key = pair.first;
			const toml::table* val = pair.second.as_table();
			if (!val) {
				Util::LogLn("Error: Expected tileset to be table at location:");
				LogConfigLocation(pair.second.source());
				return std::unexpected(ApplicationParseError::VALUE_TYPE_WRONG);
			}
			auto tileset = ParseTileset(*val);
			if (!tileset.has_value()) {
				return std::unexpected(tileset.error());
			}
			config.tilesets.insert(std::pair<std::string, TilesetConfig>(key.str(), tileset.value()));
		}
	}
	return config;
}

// Parses toml array to a vector of a single type, logs and skips invalid values
template <typename T, typename F>
	requires std::predicate<F, T>
static std::expected<std::vector<T>, ApplicationParseError> ParseArray(const toml::array& arr, F&& predicate)
{
	std::vector<T> result;
	for (int i = 0; i < arr.size(); i++) {
		const toml::value<T>* found = arr.get_as<T>(i);
		if (!found) {
			Util::LogLn("Error: Wrong type, expected 'TODO', at location:");
			LogConfigLocation(arr.get(i)->source());
			return std::unexpected(ApplicationParseError::VALUE_TYPE_WRONG);
		}
		if (!predicate(found->get())) {
			Util::LogLn("Error: Predicate failed with value '{0}', at location:", found->get());
			LogConfigLocation(arr.get(i)->source());
			return std::unexpected(ApplicationParseError::VALUE_PREDICATE_FAILED);
		}
		result.push_back(found->get());
	}
	return result;
}

template <typename T>
static std::expected<std::vector<T>, ApplicationParseError> ParseArray(const toml::array& arr)
{
	// Default predicate should compile to nothing ?
	return ParseArray<T>(arr, [](auto) -> bool { return true; });
}

std::expected<PaletteConfig, ApplicationParseError> App::ParsePalette(const toml::table& tbl)
{
	PaletteConfig palette{ };
	const toml::array* colors = tbl.get_as<toml::array>("colors");
	if (!colors) {
		Util::LogLn("Error: Missing or wrong type colors field for palette, should by array of 4 ints, at location");
		LogConfigLocation(tbl.source());
		return std::unexpected(ApplicationParseError::FIELD_MISSING);
	}
	auto result = ParseArray<int64_t>(*colors);
	if (!result.has_value()) {
		return std::unexpected(result.error());
	}
	if (result->size() != 4) {
		Util::LogLn("Error: Wrong number of colors provided, found {0} (with errors skipped), expected 4 at location:", result->size());
		LogConfigLocation(colors->source());
		return std::unexpected(ApplicationParseError::ARRAY_LENGTH_WRONG);
	}
	std::copy_n(result->begin(), 4, palette.colors.begin());
	return palette;
}

std::expected<TilesetConfig, ApplicationParseError> App::ParseTileset(const toml::table& tbl)
{
	TilesetConfig tileset{ };
	const toml::array* sprites = tbl.get_as<toml::array>("sprites");
	if (!sprites) {
		Util::LogLn("Error: Missing or wrong type 'sprites' field for tileset, should by array of strings, at location");
		LogConfigLocation(tbl.source());
		return std::unexpected(ApplicationParseError::FIELD_MISSING);
	}
	auto result = ParseArray<std::string>(*sprites);
	if (!result.has_value()) {
		return std::unexpected(result.error());
	}
	tileset.sprites = *result;
	tileset.palette = tbl["palette"].value<std::string>();
	return tileset;
}