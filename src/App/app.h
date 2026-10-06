#pragma once

#include <common.h>
#include <Asset/sprite.h>

enum class ApplicationRunError {
	NOT_ENOUGH_ARGUMENTS,
	TARGET_DIR_DOES_NOT_EXIST,
	DATA_ERROR,
};

class Application
{
public:
	static std::optional<ApplicationRunError> Run(const std::vector<std::string_view>& arguments);
private:
	static std::expected<std::vector<std::unique_ptr<Sprite>>, ApplicationRunError> LoadSprites(const std::filesystem::path& path);
	Application();
};