#include <Util/validation.h>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

TEST(ValidationTest, ToLowercase)
{
	// Check various kinds of strings
	EXPECT_EQ(Util::ToLowercase("QUACK"), "quack");
	EXPECT_EQ(Util::ToLowercase("QUACK\r\n"), "quack\r\n");
	EXPECT_EQ(Util::ToLowercase("QUACK0129,./';#][}{ "), "quack0129,./';#][}{ ");
	EXPECT_EQ(Util::ToLowercase(""), "");
	EXPECT_EQ(Util::ToLowercase(" "), " ");
	// std::string_view should support null chars
	EXPECT_EQ(Util::ToLowercase("QUACK\0QUACK"), "quack\0quack");
	// We expect a cstring to be split by a nullptr
	const char* cString = "QUACK\0QUACK";
	EXPECT_EQ(Util::ToLowercase(cString), "quack");
	// Confirm that original string is unmodified
	std::string a = "QUACK";
	auto* ptr = a.data();
	std::ignore = Util::ToLowercase(a);
	EXPECT_EQ(a, "QUACK");
	EXPECT_EQ(a.data(), ptr);
	// Nullptr string view is allowed if it empty
	EXPECT_EQ(Util::ToLowercase(std::string_view(nullptr, 0)), "");
}

TEST(ValidationTest, CheckedIntegerCast)
{
	// Check unsigned type
	EXPECT_THAT(Util::CheckedIntegerCast<uint8_t>(-1), std::nullopt);
	EXPECT_THAT(Util::CheckedIntegerCast<uint8_t>(0), testing::Optional(0));
	EXPECT_THAT(Util::CheckedIntegerCast<uint8_t>(255), testing::Optional(255));
	EXPECT_THAT(Util::CheckedIntegerCast<uint8_t>(256), std::nullopt);
	// Check signed type
	EXPECT_THAT(Util::CheckedIntegerCast<int8_t>(-129), std::nullopt);
	EXPECT_THAT(Util::CheckedIntegerCast<int8_t>(-128), testing::Optional(-128));
	EXPECT_THAT(Util::CheckedIntegerCast<int8_t>(127), testing::Optional(127));
	EXPECT_THAT(Util::CheckedIntegerCast<int8_t>(128), std::nullopt);
	// Different range
	EXPECT_THAT(Util::CheckedIntegerCast<int8_t>(-1, 0, 3), std::nullopt);
	EXPECT_THAT(Util::CheckedIntegerCast<int8_t>(0, 0, 3), testing::Optional(0));
	EXPECT_THAT(Util::CheckedIntegerCast<int8_t>(3, 0, 3), testing::Optional(3));
	EXPECT_THAT(Util::CheckedIntegerCast<int8_t>(4, 0, 3), std::nullopt);
}

TEST(ValidationTest, ClampedIntegerCast)
{
	// Check unsigned type
	EXPECT_EQ(Util::ClampedIntegerCast<uint8_t>(-1), 0);
	EXPECT_EQ(Util::ClampedIntegerCast<uint8_t>(0), 0);
	EXPECT_EQ(Util::ClampedIntegerCast<uint8_t>(255), 255);
	EXPECT_EQ(Util::ClampedIntegerCast<uint8_t>(256), 255);
	// Check signed type
	EXPECT_EQ(Util::ClampedIntegerCast<int8_t>(-129), -128);
	EXPECT_EQ(Util::ClampedIntegerCast<int8_t>(-128), -128);
	EXPECT_EQ(Util::ClampedIntegerCast<int8_t>(127), 127);
	EXPECT_EQ(Util::ClampedIntegerCast<int8_t>(128), 127);
	// Different range
	EXPECT_EQ(Util::ClampedIntegerCast<int8_t>(-1, 0, 3), 0);
	EXPECT_EQ(Util::ClampedIntegerCast<int8_t>(0, 0, 3), 0);
	EXPECT_EQ(Util::ClampedIntegerCast<int8_t>(3, 0, 3), 3);
	EXPECT_EQ(Util::ClampedIntegerCast<int8_t>(4, 0, 3), 3);
}

TEST(ValidationTest, DoPathOverlap)
{
	EXPECT_FALSE(Util::DoPathsOverlap("./a/", "./b/"));
	EXPECT_FALSE(Util::DoPathsOverlap("b/", "./a/"));
	EXPECT_FALSE(Util::DoPathsOverlap("./b/", "./a"));
	EXPECT_FALSE(Util::DoPathsOverlap("./b", "./a"));
	EXPECT_TRUE(Util::DoPathsOverlap("./a/", "./a/"));
	EXPECT_TRUE(Util::DoPathsOverlap("./a/b", "./a/"));
	EXPECT_TRUE(Util::DoPathsOverlap("./a/b", "."));
	EXPECT_TRUE(Util::DoPathsOverlap("a/b", "."));
	EXPECT_TRUE(Util::DoPathsOverlap("a/b", "./a/b"));
	EXPECT_TRUE(Util::DoPathsOverlap(".", "."));
	EXPECT_THROW(Util::DoPathsOverlap("", "./a/b"), std::invalid_argument);
}
