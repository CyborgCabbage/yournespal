#include <gtest/gtest.h>
#include <Asset/sprite.h>

TEST(SpriteTest, Load) {
	EXPECT_TRUE(Sprite::TryLoadSprite("./testdata/assets/valid.png").has_value());
	EXPECT_TRUE(Sprite::TryLoadSprite(".\\testdata\\assets\\valid.png").has_value());
	EXPECT_TRUE(Sprite::TryLoadSprite("./testdata/assets/capitalised.PNG").has_value());
	EXPECT_EQ(Sprite::TryLoadSprite("./testdata/assets/nonexistent.png"), std::unexpected(LoadSpriteError::LOAD_FAILED));
	EXPECT_EQ(Sprite::TryLoadSprite("./testdata/assets/invalid.png"), std::unexpected(LoadSpriteError::DECODE_FAILED));
	EXPECT_EQ(Sprite::TryLoadSprite("./testdata/assets/wrong_format.bmp"), std::unexpected(LoadSpriteError::UNSUPPORTED_FORMAT));
	EXPECT_EQ(Sprite::TryLoadSprite("./testdata/assets/text.txt"), std::unexpected(LoadSpriteError::NOT_IMAGE));
	EXPECT_EQ(Sprite::TryLoadSprite("./testdata/assets/extensionless"), std::unexpected(LoadSpriteError::NOT_IMAGE));
}