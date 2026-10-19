#include "app.h"

#include <App/config.h>
#include <Asset/palette.h>
#include <Asset/tileset.h>
#include <Util/compute.h>
#include <Util/log.h>
#include <Util/validation.h>

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
	if (Util::DoPathsOverlap(config.inputPath, config.outputPath)) {
		Util::LogLn("Error: Paths {0} and {1} must not overlap", config.inputPath.string(), config.outputPath.string());
		return ApplicationRunError::PATH_INVALID;
	}
	// LOAD IMAGES
	std::unordered_map<std::string, Asset::Image> images;
	std::unordered_map<std::string, std::vector<std::string>> paletteToImage;
	for (auto& [tilesetName, tilesetConfig] : config.tilesets) {
		// Get Palette (we say that empty string should be the same as no value)
		if (!tilesetConfig.palette.empty()) {
			// Check palette exists
			if (!config.palettes.contains(tilesetConfig.palette)) {
				Util::LogLn("Error: Could not find palette {} for tileset {}", tilesetConfig.palette, tilesetName);
				return ApplicationRunError::DATA_INVALID;
			}
		} else {
			tilesetConfig.palette = config.AddAnonymousPalette(tilesetName);
		}
		// Load images from tileset sprite list
		for (const std::string& spriteName : tilesetConfig.sprites) {
			auto image = Asset::Image::FromFile(config.inputPath / (spriteName + ".png"));
			if (!image) {
				return ApplicationRunError::DATA_INVALID;
			}
			images.try_emplace(spriteName, std::move(image.value()));
			paletteToImage[tilesetConfig.palette].push_back(spriteName);
		}
	}
	auto& nespal = Asset::Palette::GetInstance();
	std::unordered_map<std::string, std::array<uint8_t, 4>> resultPalette;
	for (const auto& [paletteName, imageNames] : paletteToImage) {
		const auto& colors = config.palettes.at(paletteName).colors;
		// Check if any of the colors are unspecified
		std::vector<uint16_t> winners;
		if (std::find(colors.begin(), colors.end(), std::nullopt) != colors.end()) {
			// Get unique colors in sprites
			std::unordered_map<Asset::Pixel, uint32_t> colorCounts;
			for (const auto& imageName : imageNames) {
				const auto& image = images.at(imageName);
				for (Asset::Pixel p : image.data.data) {
					colorCounts[p] += 1;
				}
			}
			// Calculate scores for colors (color = candidate = width, pixel = voter = height)
			MDVector<float> scores(nespal.lab.size(), colorCounts.size());
			size_t i = 0;
			for (const auto& [pixel, count] : colorCounts) {
				Asset::Lab c = PixelToLab(pixel);
				for (size_t j = 0; j < nespal.lab.size(); j++) {
					scores.span[j, i] = (c - nespal.lab[j]).length() * count;
				}
				i++;
			}
			// Get predetermined colors
			std::vector<uint16_t> fixed;
			for (auto c : colors) {
				if (c.has_value()) {
					fixed.push_back(*c);
				}
			}
			// Get palette
			auto [_winners, ballots] = Util::ScoredElimination(scores, 4 - fixed.size(), fixed);
			winners = std::move(_winners);
		}
		// Combine fixed and winners
		auto& newPal = resultPalette[paletteName];
		for (size_t i = 0; i < 4; i++) {
			// Black as a placeholder for if we don't need all palette slots
			uint8_t val = 0x0F;
			if (colors[i].has_value()) {
				val = *colors[i];
			} else if (winners.size() > 0) {
				val = winners.back();
				winners.pop_back();
			}
			newPal[i] = nespal.mapDuplicates[val];
		}
		Util::LogLn("Palette '{}'", paletteName);
		for (auto index : newPal) {
			Util::LogLn("0x{:02X}", index);
		}
	}
	// CREATE TILESET
	std::filesystem::create_directories(config.outputPath);
	for (const auto& [tilesetName, tilesetConfig] : config.tilesets) {
		Util::LogLn("Tileset '{}'", tilesetName);
		// Create tileset and add sprites
		Asset::Tileset tileset;
		tileset.palette = resultPalette.at(tilesetConfig.palette);
		for (const auto& spriteName : tilesetConfig.sprites) {
			tileset.AddSprite(spriteName, images.at(spriteName));
		}
		Util::LogLn("{} Unique tiles", tileset.tiles.size());
		// Output tileset and nametables
		tileset.ToPaletteImage().ToFile(config.outputPath / (tilesetName + ".png"));
		for (const auto& [nametableName, nametable] : tileset.nametables) {
			std::ofstream nametableFile(config.outputPath / (nametableName + ".nam.bin"), std::ios::out | std::ios::trunc | std::ios::binary);
			nametableFile << static_cast<uint8_t>(nametable.width);
			nametableFile << static_cast<uint8_t>(nametable.height);
			for (const auto val : nametable.data) {
				nametableFile << static_cast<uint8_t>(val);
			}
		}
	}
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