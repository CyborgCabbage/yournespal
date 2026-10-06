#pragma once

#include <common.h>
#include <Asset/sprite.h>

enum class ApplicationRunError {
	NOT_ENOUGH_ARGUMENTS,
	TARGET_DIR_DOES_NOT_EXIST,
	DATA_ERROR,
};

struct AppConfig {
	std::filesystem::path inputDir;
	std::filesystem::path outputDir;
};

class Application
{
public:
	static std::optional<ApplicationRunError> Run(const std::vector<std::string_view>& arguments);
	static std::vector<Pixel> LoadPalette();
private:
	static std::expected<std::vector<Image>, ApplicationRunError> LoadImages(const std::filesystem::path& path);
	Application();
};