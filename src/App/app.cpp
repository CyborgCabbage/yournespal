#include "app.h"

#include <Asset/sprite.h>
#include <Core/log.h>
#include <Asset/palette.h>

std::optional<ApplicationRunError> Application::Run(const std::vector<std::string_view>& arguments)
{
    ynp::LogLn("Log: Current working directory {0}", std::filesystem::current_path().string());
    if (arguments.size() < 2) {
        ynp::LogLn("Error: Not enough arguments, specify target and output directory");
        return ApplicationRunError::NOT_ENOUGH_ARGUMENTS;
    }
    AppConfig config{
        .inputDir = std::filesystem::path(arguments[0]),
        .outputDir = std::filesystem::path(arguments[1]),
    };
    // Validate input and output directories
    bool anyDirInvalid = false;
    for (const auto& dir : { config.inputDir, config.outputDir }) {
        if (!std::filesystem::exists(dir)) {
            ynp::LogLn("Error: Path {0} does not exist", dir.string());
            anyDirInvalid = true;
        } else if (!std::filesystem::is_directory(dir)) {
            ynp::LogLn("Error: Path {0} is not a directory", dir.string());
            anyDirInvalid = true;
        }
    }
    if (anyDirInvalid || DoPathsOverlap(config.inputDir, config.outputDir)) {
        return ApplicationRunError::TARGET_DIRECTORY_IS_INVALID;
    }
    // Load image
    auto result = LoadImages(config.inputDir);
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
        spr.SaveSprite(config.outputDir);
    }
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
        case_VALUE_return_STRING(ApplicationRunError::NOT_ENOUGH_ARGUMENTS);
        case_VALUE_return_STRING(ApplicationRunError::TARGET_DIRECTORY_IS_INVALID);
        case_VALUE_return_STRING(ApplicationRunError::DATA_ERROR);
        default_return_UNDERLYING(ApplicationRunError, error);
    }
}

std::expected<std::vector<Image>, ApplicationRunError> Application::LoadImages(const std::filesystem::path& path)
{
    // Check that directory exists
    if (!std::filesystem::exists(path)) {
        ynp::LogLn("Directory \"{0}\" does not exist", path.string());
        return std::unexpected(ApplicationRunError::TARGET_DIRECTORY_IS_INVALID);
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
        return std::unexpected(ApplicationRunError::DATA_ERROR);
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