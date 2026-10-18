#include <Asset/image.h>
#include <Asset/tileset.h>
#include <gtest/gtest.h>

using namespace Asset;

TEST(TilesetTest, LoadImageFromFile)
{
	EXPECT_TRUE(Image::FromFile("./testdata/assets/valid.png").has_value());
	EXPECT_TRUE(Image::FromFile(".\\testdata\\assets\\valid.png").has_value());
	EXPECT_TRUE(Image::FromFile("./testdata/assets/capitalised.PNG").has_value());
	EXPECT_EQ(Image::FromFile("./testdata/assets/nonexistent.png"), std::unexpected(LoadImageError::LOAD_FAILED));
	EXPECT_EQ(Image::FromFile("./testdata/assets/invalid.png"), std::unexpected(LoadImageError::DECODE_FAILED));
	EXPECT_EQ(Image::FromFile("./testdata/assets/wrong_format.bmp"), std::unexpected(LoadImageError::UNSUPPORTED_FORMAT));
	EXPECT_EQ(Image::FromFile("./testdata/assets/text.txt"), std::unexpected(LoadImageError::NOT_IMAGE));
	EXPECT_EQ(Image::FromFile("./testdata/assets/extensionless"), std::unexpected(LoadImageError::NOT_IMAGE));
}

static std::string PrintTile(const Tile& tile)
{
	std::string result;
	for (int y = 0; y < 8; y++) {
		result += std::format("{:016b}\n", tile.data[y]);
	}
	return result;
}

TEST(TilesetTest, TileReadWrite)
{
	// Write row
	{
		std::array<uint8_t, 8> testValues = { 3, 2, 1, 0, 3, 2, 1, 0 };
		Tile tile;
		for (int x = 0; x < 8; x++) {
			tile.Set(1, x, testValues[x]);
		}
		bool equal = true;
		for (int x = 0; x < 8; x++) {
			equal &= (tile.Get(1, x) == testValues[x]);
		}
		EXPECT_TRUE(equal) << PrintTile(tile);
	}
	// Write whole thing
	{
		Tile tile;
		for (int x = 0; x < 8; x++) {
			for (int y = 0; y < 8; y++) {
				tile.Set(x, y, (x + y) % 4);
			}
		}
		bool equal = true;
		for (int x = 0; x < 8; x++) {
			for (int y = 0; y < 8; y++) {
				equal &= (tile.Get(x, y) == (x + y) % 4);
			}
		}
		EXPECT_TRUE(equal) << PrintTile(tile);
	}
}
