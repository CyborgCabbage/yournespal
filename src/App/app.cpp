#include "app.h"

#include <Asset/palette.h>
#include <Asset/sprite.h>
#include <Core/core.h>

std::optional<ApplicationRunError> Application::Run(const std::vector<std::string_view>& arguments)
{
	ynp::LogLn("Log: Current working directory {0}", std::filesystem::current_path().string());
	if (arguments.size() < 2) {
		ynp::LogLn("Error: Not enough arguments, specify target and output directory");
		return ApplicationRunError::ARGUMENTS_LENGTH_WRONG;
	}
	AppArguments appArgs{
		.configFile = std::filesystem::path(arguments[0]),
		.outputPath = std::filesystem::path(arguments[1]),
	};
	auto parseResult = ParseConfig(appArgs);
	if (!parseResult.has_value()) {
		return ApplicationRunError::DATA_INVALID;
	}
	AppConfig config = parseResult.value();
	// Validate input and output directories
	if (!std::filesystem::exists(config.outputPath)) {
		ynp::LogLn("Error: Path {0} does not exist", config.outputPath.string());
		return ApplicationRunError::PATH_INVALID;
	}
	if (!std::filesystem::is_directory(config.outputPath)) {
		ynp::LogLn("Error: Path {0} is not a directory", config.outputPath.string());
		return ApplicationRunError::PATH_INVALID;
	}
	if (DoPathsOverlap(config.inputPath, config.outputPath)) {
		ynp::LogLn("Error: Paths {0} and {1} must not overlap", config.inputPath.string(), config.outputPath.string());
		return ApplicationRunError::PATH_INVALID;
	}
	// (1) Load sprite images
	// (2) For each sprite, break it up into unique tiles + a name table to assemble them
	// (3) For each tileset, put the consistituent tiles into one sheet
	auto& nespal = Palette::GetInstance();
	for (const auto& [key, tilesetConfig] : config.tilesets) {
		// Load sprite images
		std::unordered_map<std::string, Image> images;
		for (const std::string& spriteName : tilesetConfig.sprites) {
			auto image = Image::FromFile(config.inputPath / (spriteName + ".png"));
			if (!image) {
				return ApplicationRunError::DATA_INVALID;
			}
			images.insert({ spriteName, image.value() });
		}
		// Get unique colors in sprites
		std::unordered_map<Pixel, uint32_t> colorCounts;
		for (const auto& [key, image] : images) {
			for (Pixel p : image.data.pixels) {
				colorCounts[p] += 1;
			}
		}
		// Calculate scores for colors (color = candidate, pixel = voter)
		Eigen::ArrayXXf scores(nespal.lab.size(), colorCounts.size());
		size_t i = 0;
		for (const auto& [pixel, count] : colorCounts) {
			Lab c = PixelToLab(pixel);
			for (size_t j = 0; j < nespal.lab.size(); j++) {
				scores(j, i) = (c - nespal.lab[j]).norm() * count;
			}
			i++;
		}
		auto [winners, ballots] = ynp::ScoredElimination(scores, 4);
		ynp::LogLn("Palette selected winners:");
		for (auto index : winners) {
			ynp::LogLn("0x{:X}", index);
		}
	}
	/*
	// Load image
	auto result = LoadImages(config.inputPath);
	if (!result.has_value()) {
		return result.error();
	}
	// Load NES palette and quantise sprites
	std::vector<Sprite> sprites;
	for (const Image& image : result.value()) {
		sprites.push_back(Sprite::FromImage(image));
	}
	// Export Images
	for (const Sprite& spr : sprites) {
		spr.SaveSprite(config.outputPath);
	}
	*/
	return std::nullopt;
}

#define case_VALUE_return_STRING(e) \
	case e:                         \
		return #e
#define default_return_UNDERLYING(e, value) \
	default:                                \
		return std::format(#e "::UNKNOWN({0})", std::to_underlying(value));

std::string Application::ErrorToString(ApplicationRunError error)
{
	switch (error) {
		case_VALUE_return_STRING(ApplicationRunError::ARGUMENTS_LENGTH_WRONG);
		case_VALUE_return_STRING(ApplicationRunError::PATH_INVALID);
		case_VALUE_return_STRING(ApplicationRunError::DATA_INVALID);
		default_return_UNDERLYING(ApplicationRunError, error);
	}
}

std::expected<std::vector<Image>, ApplicationRunError> Application::LoadImages(const std::filesystem::path& path)
{
	// Check that directory exists
	if (!std::filesystem::exists(path)) {
		ynp::LogLn("Directory \"{0}\" does not exist", path.string());
		return std::unexpected(ApplicationRunError::PATH_INVALID);
	}
	// Iterate files and load images
	bool encounteredError = false;
	std::vector<Image> images;
	for (const auto& entry : std::filesystem::directory_iterator(path)) {
		auto result = Image::FromFile(entry.path());
		if (result.has_value()) {
			images.emplace_back(std::move(*result));
		} else if (result.error() == LoadImageError::DECODE_FAILED || result.error() == LoadImageError::LOAD_FAILED) {
			// Use flag instead of returning here becuase we want to provide the user with
			// all the log messages for every image tried to load, not just stopping at the first
			encounteredError = true;
		}
	}
	if (encounteredError) {
		return std::unexpected(ApplicationRunError::DATA_INVALID);
	}
	return images;
}

bool Application::DoPathsOverlap(std::filesystem::path a, std::filesystem::path b)
{
	if (a.empty() || b.empty()) {
		// Whether the paths overlap cannot be determined if one is empty
		throw std::invalid_argument("Cannot determine whether an empty path overlaps");
	}
	// Normalise paths
	a = std::filesystem::absolute(a.lexically_normal());
	b = std::filesystem::absolute(b.lexically_normal());
	const auto [aPart, bPart] = std::mismatch(a.begin(), a.end(), b.begin(), b.end());
	bool aEnded = (aPart == a.end() || aPart->empty());
	bool bEnded = (bPart == b.end() || bPart->empty());
	return aEnded || bEnded;
}

void LogConfigLocation(const toml::source_region& region)
{
	ynp::LogLn("[Line, Column] = [{0}, {1}] to [{2}, {3}]:", region.begin.line, region.begin.column, region.end.line, region.end.column);
}

std::expected<AppConfig, ApplicationParseError> Application::ParseConfig(const AppArguments& arguments)
{
	// Load TOML file
	toml::table loaded;
	try {
		loaded = toml::parse_file(arguments.configFile.string());
	} catch (const toml::parse_error& err) {
		ynp::LogLn("Error: Error loading config file '{0}' - {1}", arguments.configFile.string(), err.description());
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
				ynp::LogLn("Error: Expected palette to be table at location:");
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
				ynp::LogLn("Error: Expected tileset to be table at location:");
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
			ynp::LogLn("Error: Wrong type, expected 'TODO', at location:");
			LogConfigLocation(arr.get(i)->source());
			return std::unexpected(ApplicationParseError::VALUE_TYPE_WRONG);
		}
		if (!predicate(found->get())) {
			ynp::LogLn("Error: Predicate failed with value '{0}', at location:", found->get());
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

std::expected<PaletteConfig, ApplicationParseError> Application::ParsePalette(const toml::table& tbl)
{
	PaletteConfig palette{ };
	const toml::array* colors = tbl.get_as<toml::array>("colors");
	if (!colors) {
		ynp::LogLn("Error: Missing or wrong type colors field for palette, should by array of 4 ints, at location");
		LogConfigLocation(tbl.source());
		return std::unexpected(ApplicationParseError::FIELD_MISSING);
	}
	auto result = ParseArray<int64_t>(*colors);
	if (!result.has_value()) {
		return std::unexpected(result.error());
	}
	if (result->size() != 4) {
		ynp::LogLn("Error: Wrong number of colors provided, found {0} (with errors skipped), expected 4 at location:", result->size());
		LogConfigLocation(colors->source());
		return std::unexpected(ApplicationParseError::ARRAY_LENGTH_WRONG);
	}
	std::copy_n(result->begin(), 4, palette.colors.begin());
	return palette;
}

std::expected<TilesetConfig, ApplicationParseError> Application::ParseTileset(const toml::table& tbl)
{
	TilesetConfig tileset{ };
	const toml::array* sprites = tbl.get_as<toml::array>("sprites");
	if (!sprites) {
		ynp::LogLn("Error: Missing or wrong type 'sprites' field for tileset, should by array of strings, at location");
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