#include <gtest/gtest.h>
#include <Asset/sprite.h>

TEST(SpriteTest, Load) {
	EXPECT_TRUE(Image::FromFile("./testdata/assets/valid.png").has_value());
	EXPECT_TRUE(Image::FromFile(".\\testdata\\assets\\valid.png").has_value());
	EXPECT_TRUE(Image::FromFile("./testdata/assets/capitalised.PNG").has_value());
	EXPECT_EQ(Image::FromFile("./testdata/assets/nonexistent.png"), std::unexpected(LoadImageError::LOAD_FAILED));
	EXPECT_EQ(Image::FromFile("./testdata/assets/invalid.png"), std::unexpected(LoadImageError::DECODE_FAILED));
	EXPECT_EQ(Image::FromFile("./testdata/assets/wrong_format.bmp"), std::unexpected(LoadImageError::UNSUPPORTED_FORMAT));
	EXPECT_EQ(Image::FromFile("./testdata/assets/text.txt"), std::unexpected(LoadImageError::NOT_IMAGE));
	EXPECT_EQ(Image::FromFile("./testdata/assets/extensionless"), std::unexpected(LoadImageError::NOT_IMAGE));
}