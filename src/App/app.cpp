#include "app.h"

#include <Asset/palette.h>
#include <Asset/sprite.h>
#include <Util/log.h>
#include <Util/validation.h>
#include <Util/compute.h>
#include <App/config.h>

using namespace App;

std::optional<ApplicationRunError> App::Run(const std::vector<std::string_view>& arguments)
{
	Util::LogLn("Log: Current working directory {0}", std::filesystem::current_path().string());
	if (arguments.size() < 2) {
		Util::LogLn("Error: Not enough arguments, specify target and output directory");
		return ApplicationRunError::ARGUMENTS_LENGTH_WRONG;
	}
	AppArguments appArgs{
		.configFile = std::filesystem::path(arguments[0]),
		.outputPath = std::filesystem::path(arguments[1]),
	};
	auto parseResult = App::ParseConfig(appArgs);
	if (!parseResult.has_value()) {
		return ApplicationRunError::DATA_INVALID;
	}
	AppConfig config = parseResult.value();
	// Validate input and output directories
	if (!std::filesystem::exists(config.outputPath)) {
		Util::LogLn("Error: Path {0} does not exist", config.outputPath.string());
		return ApplicationRunError::PATH_INVALID;
	}
	if (!std::filesystem::is_directory(config.outputPath)) {
		Util::LogLn("Error: Path {0} is not a directory", config.outputPath.string());
		return ApplicationRunError::PATH_INVALID;
	}
	if (Util::DoPathsOverlap(config.inputPath, config.outputPath)) {
		Util::LogLn("Error: Paths {0} and {1} must not overlap", config.inputPath.string(), config.outputPath.string());
		return ApplicationRunError::PATH_INVALID;
	}
	// (1) Load sprite images
	// (2) For each sprite, break it up into unique tiles + a name table to assemble them
	// (3) For each tileset, put the consistituent tiles into one sheet
	auto& nespal = Asset::Palette::GetInstance();
	for (const auto& [key, tilesetConfig] : config.tilesets) {
		// Load sprite images
		std::unordered_map<std::string, Asset::Image> images;
		for (const std::string& spriteName : tilesetConfig.sprites) {
			auto image = Asset::Image::FromFile(config.inputPath / (spriteName + ".png"));
			if (!image) {
				return ApplicationRunError::DATA_INVALID;
			}
			images.insert({ spriteName, image.value() });
		}
		// Get unique colors in sprites
		std::unordered_map<Asset::Pixel, uint32_t> colorCounts;
		for (const auto& [key, image] : images) {
			for (Asset::Pixel p : image.data.pixels) {
				colorCounts[p] += 1;
			}
		}
		// Calculate scores for colors (color = candidate, pixel = voter)
		Eigen::ArrayXXf scores(nespal.lab.size(), colorCounts.size());
		size_t i = 0;
		for (const auto& [pixel, count] : colorCounts) {
			Asset::Lab c = PixelToLab(pixel);
			for (size_t j = 0; j < nespal.lab.size(); j++) {
				scores(j, i) = (c - nespal.lab[j]).norm() * count;
			}
			i++;
		}
		// Get palette
		auto [winners, ballots] = Util::ScoredElimination(scores, 4);
		Util::LogLn("Palette selected winners:");
		for (auto index : winners) {
			Util::LogLn("0x{:02X}", nespal.mapDuplicates[index]);
		}
		// Create tileset and add sprites
		Asset::Tileset tileset;
		std::copy(winners.begin(), winners.end(), tileset.palette.begin());
		for (const auto& [key, image] : images) {
			tileset.AddSprite(key, image);
		}
		Util::LogLn("{} Unique tiles", tileset.tiles.size());
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

std::string App::ErrorToString(ApplicationRunError error)
{
	switch (error) {
		case_VALUE_return_STRING(ApplicationRunError::ARGUMENTS_LENGTH_WRONG);
		case_VALUE_return_STRING(ApplicationRunError::PATH_INVALID);
		case_VALUE_return_STRING(ApplicationRunError::DATA_INVALID);
		default_return_UNDERLYING(ApplicationRunError, error);
	}
}

std::expected<std::vector<Asset::Image>, ApplicationRunError> App::LoadImages(const std::filesystem::path& path)
{
	// Check that directory exists
	if (!std::filesystem::exists(path)) {
		Util::LogLn("Directory \"{0}\" does not exist", path.string());
		return std::unexpected(ApplicationRunError::PATH_INVALID);
	}
	// Iterate files and load images
	bool encounteredError = false;
	std::vector<Asset::Image> images;
	for (const auto& entry : std::filesystem::directory_iterator(path)) {
		auto result = Asset::Image::FromFile(entry.path());
		if (result.has_value()) {
			images.emplace_back(std::move(*result));
		} else if (result.error() == Asset::LoadImageError::DECODE_FAILED || result.error() == Asset::LoadImageError::LOAD_FAILED) {
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
