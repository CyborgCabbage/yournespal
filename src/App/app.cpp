#include "app.h"

#include <Asset/palette.h>
#include <Asset/sprite.h>
#include <Core/core.h>

std::optional<ApplicationRunError> Application::Run(const std::vector<std::string_view>& arguments)
{
    ynp::LogLn("Log: Current working directory {0}", std::filesystem::current_path().string());
    if (arguments.size() < 2) {
        ynp::LogLn("Error: Not enough arguments, specify target and output directory");
        return ApplicationRunError::NOT_ENOUGH_ARGUMENTS;
    }
    AppArguments appArgs{
        .configFile = std::filesystem::path(arguments[0]),
        .outputPath = std::filesystem::path(arguments[1]),
    };
    auto parseResult = ParseConfig(appArgs);
    if (!parseResult.has_value()) {
        return ApplicationRunError::DATA_ERROR;
    }
    AppConfig config = parseResult.value();
    // Validate input and output directories
    if (!std::filesystem::exists(config.outputPath)) {
        ynp::LogLn("Error: Path {0} does not exist", config.outputPath.string());
        return ApplicationRunError::INVALID_PATH;
    }
    if (!std::filesystem::is_directory(config.outputPath)) {
        ynp::LogLn("Error: Path {0} is not a directory", config.outputPath.string());
        return ApplicationRunError::INVALID_PATH;
    }
    if (DoPathsOverlap(config.inputPath, config.outputPath)) {
        ynp::LogLn("Error: Paths {0} and {1} must not overlap", config.inputPath.string(), config.outputPath.string());
        return ApplicationRunError::INVALID_PATH;
    }
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
        case_VALUE_return_STRING(ApplicationRunError::INVALID_PATH);
        case_VALUE_return_STRING(ApplicationRunError::DATA_ERROR);
        default_return_UNDERLYING(ApplicationRunError, error);
    }
}

std::expected<std::vector<Image>, ApplicationRunError> Application::LoadImages(const std::filesystem::path& path)
{
    // Check that directory exists
    if (!std::filesystem::exists(path)) {
        ynp::LogLn("Directory \"{0}\" does not exist", path.string());
        return std::unexpected(ApplicationRunError::INVALID_PATH);
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

std::expected<AppConfig, ApplicationParseError> Application::ParseConfig(const AppArguments& arguments)
{
    // Load TOML file
    toml::table loaded;
    try {
        loaded = toml::parse_file(arguments.configFile.string());
    } catch (const toml::parse_error& err) {
        ynp::LogLn("Error: Error loading config file '{0}' - {1}", arguments.configFile.string(), err.description());
        return std::unexpected(ApplicationParseError::COULD_NOT_LOAD_FILE);
    }
    // File out config structure
    AppConfig config;
    config.inputPath = arguments.configFile.parent_path();
    config.outputPath = arguments.outputPath;
    // Load palettes
    if (const toml::table* palettes = loaded.get_as<toml::table>("palette")) {
        palettes->for_each([](const toml::key& key, const toml::table& val) {
            ConfigPalette palette{ };
            if (const toml::array* colors = val.get_as<toml::array>("colors")) {
                size_t index = 0;
                colors->for_each([&](int64_t colorId) {
                    auto downcast = ynp::CheckedIntegerCast<uint8_t>(colorId, 0, 63);
                    if (downcast.has_value())
                    {
                        palette.colors[index] = *downcast;
                    }
                    else {
                        ynp::LogLn("Error: Failed to parse color ID");
                    }
                    index++;
                });
            }
            std::cout << key << ": " << val << "\n";
        });
    }
    // Load tilesets
    if (auto* tilesets = loaded.get_as<toml::table>("tileset")) {
        tilesets->for_each([](const toml::key& key, auto&& val) {
            std::cout << key << ": " << val << "\n";
        });
    }
    return config;
}