#pragma once

#include <Asset/tileset.h>
#include <Asset/image.h>
#include <common.h>

namespace App {

enum class ApplicationRunError {
	ARGUMENTS_LENGTH_WRONG,
	PATH_INVALID,
	DATA_INVALID,
};

struct AppArguments {
	std::filesystem::path configFile;
	std::filesystem::path outputPath;
};

std::optional<ApplicationRunError> Run(const std::vector<std::string_view>& arguments);
std::string ErrorToString(ApplicationRunError error);
std::expected<std::vector<Asset::Image>, ApplicationRunError> LoadImages(const std::filesystem::path& path);

}