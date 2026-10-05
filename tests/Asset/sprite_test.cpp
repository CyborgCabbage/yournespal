#include <gtest/gtest.h>
#include <Asset/sprite.h>
#include <Error/error.h>

TEST(SpriteTest, Load) {
	EXPECT_TRUE(Sprite::TryLoadSprite("./testdata/assets/valid.png").has_value());
	EXPECT_TRUE(Sprite::TryLoadSprite(".\\testdata\\assets\\valid.png").has_value());
	EXPECT_TRUE(Sprite::TryLoadSprite("./testdata/assets/capitalised.PNG").has_value());
	EXPECT_EQ(Sprite::TryLoadSprite("./testdata/assets/nonexistent.png"), ynp::Unexpected(LoadSpriteError::LOAD_FAILED));
	EXPECT_EQ(Sprite::TryLoadSprite("./testdata/assets/invalid.png"), ynp::Unexpected(LoadSpriteError::DECODE_FAILED));
	EXPECT_EQ(Sprite::TryLoadSprite("./testdata/assets/wrong_format.bmp"), ynp::Unexpected(LoadSpriteError::UNSUPPORTED_FORMAT));
	EXPECT_EQ(Sprite::TryLoadSprite("./testdata/assets/text.txt"), ynp::Unexpected(LoadSpriteError::NOT_IMAGE));
	EXPECT_EQ(Sprite::TryLoadSprite("./testdata/assets/extensionless"), ynp::Unexpected(LoadSpriteError::NOT_IMAGE));
}