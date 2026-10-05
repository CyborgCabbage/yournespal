#include "app.h"

#include <Asset/sprite.h>

void Application::Run(int argc, char* argv[])
{
	std::println("Current working directory: {0}", std::filesystem::current_path().string());
	if (argc < 2) {
		std::println("Not enough arguments, specify target directory (use . for current directory)");
		return;
	}
	auto target = std::filesystem::path(argv[1]);
	if (!std::filesystem::exists(target)) {
		std::println("Directory \"{0}\" does not exist", target.string());
		return;
	}
	std::vector<Sprite> sprites;
	for (const auto& entry : std::filesystem::directory_iterator(target)) {
		auto result = Sprite::TryLoadSprite(entry.path());
		if (result.has_value()) {
			sprites.emplace_back(*result);
		}
		else if (result.error() == LoadSpriteError::LOAD_FAILED || result.error() == LoadSpriteError::DECODE_FAILED) {
			std::println("Ending program due to error");
			return;
		}
	}
}