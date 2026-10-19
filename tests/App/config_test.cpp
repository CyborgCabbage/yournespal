#include <App/config.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <ranges>

using testing::UnorderedElementsAre;

using namespace App;

TEST(ConfigTest, AppConfigAddPalette)
{
	AppConfig config;
	config.palettes["apple"] = { };
	config.palettes["orange"] = { };
	config.palettes["banana"] = { };
	config.palettes["peach"] = { };
	config.AddAnonymousPalette("tomato");
	config.AddAnonymousPalette("peach");
	config.AddAnonymousPalette("peach");
	EXPECT_THAT(std::views::keys(config.palettes), UnorderedElementsAre("apple", "orange", "banana", "tomato", "peach", "peach_0", "peach_1"));
}