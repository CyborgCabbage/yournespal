#include "app.h"

#include <Asset/sprite.h>
#include <Core/log.h>

struct Pixel {
	auto operator<=>(const Pixel&) const = default;
	uint8_t R = 0;
	uint8_t G = 0;
	uint8_t B = 0;
	uint8_t A = 0;
};

template<>
struct std::hash<Pixel>
{
	std::size_t operator()(const Pixel& s) const noexcept
	{
		return std::hash<uint32_t>{}(std::bit_cast<uint32_t>(s));
	}
};

std::optional<ApplicationRunError> Application::Run(const std::vector<std::string_view>& arguments)
{
	ynp::LogLn("Log: Current working directory {0}", std::filesystem::current_path().string());
	if (arguments.size() == 0) {
		ynp::LogLn("Error: Not enough arguments, specify target directory (use . for current directory)");
		return ApplicationRunError::NOT_ENOUGH_ARGUMENTS;
	}
	auto target = std::filesystem::path(arguments[0]);
	auto result = LoadSprites(target);
	if (!result.has_value()) {
		return result.error();
	}
	std::vector<Sprite> sprites = result.value();
	for (const auto& spr : sprites) {
		std::unordered_set<Pixel> colours;
		auto& d = spr.data.data;
		for (int i = 0; i < d.size(); i += 4) {
			colours.insert(Pixel(d[i], d[i + 1], d[i + 2], d[i + 3]));
		}
		ynp::LogLn("Log: There were {0} unique colours", colours.size());
	}
	return std::nullopt;
}

std::expected<std::vector<Sprite>, ApplicationRunError> Application::LoadSprites(const std::filesystem::path& path)
{
	// Check that directory exists
	if (!std::filesystem::exists(path)) {
		ynp::LogLn("Directory \"{0}\" does not exist", path.string());
		return std::unexpected(ApplicationRunError::TARGET_DIR_DOES_NOT_EXIST);
	}
	// Iterate files and load sprites
	std::vector<Sprite> sprites;
	for (const auto& entry : std::filesystem::directory_iterator(path)) {
		auto result = Sprite::TryLoadSprite(entry.path());
		if (result.has_value()) {
			sprites.emplace_back(*result);
		}
		else if (result.error() == LoadSpriteError::DECODE_FAILED || result.error() == LoadSpriteError::LOAD_FAILED) {
			return std::unexpected(ApplicationRunError::DATA_ERROR);
		}
	}
	return sprites;
}
