#pragma once

#include <Asset/sprite.h>
#include <common.h>

enum class ApplicationRunError {
    NOT_ENOUGH_ARGUMENTS,
    TARGET_DIRECTORY_IS_INVALID,
    DATA_ERROR,
};

struct AppConfig {
    std::filesystem::path inputDir;
    std::filesystem::path outputDir;
};

class Application {
public:
    static std::optional<ApplicationRunError> Run(const std::vector<std::string_view>& arguments);
    static std::string ErrorToString(ApplicationRunError error);
    static bool DoPathsOverlap(std::filesystem::path a, std::filesystem::path b);

private:
    static std::expected<std::vector<Image>, ApplicationRunError> LoadImages(const std::filesystem::path& path);
    Application();
};