#include <App/app.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

TEST(AppTest, Run)
{
	EXPECT_THAT(Application::Run({ "./testdata/projects/test0/", "./testoutput/" }), std::nullopt);
	EXPECT_THAT(Application::Run({ "./testdata/projects/test0", "./testoutput/" }), std::nullopt);
	EXPECT_THAT(Application::Run({ "./testdata/projects/test1/", "./testoutput/" }), testing::Optional(ApplicationRunError::DATA_INVALID));
	EXPECT_THAT(Application::Run({ }), testing::Optional(ApplicationRunError::ARGUMENTS_LENGTH_WRONG));
	EXPECT_THAT(Application::Run({ "./doesntexist", "./testoutput/" }), testing::Optional(ApplicationRunError::PATH_INVALID));
	EXPECT_THAT(Application::Run({ "./testdata/projects/test0/", "./testdata/projects/test0/" }), testing::Optional(ApplicationRunError::PATH_INVALID));
}

TEST(AppTest, PathOverlap)
{
	EXPECT_FALSE(Application::DoPathsOverlap("./a/", "./b/"));
	EXPECT_FALSE(Application::DoPathsOverlap("b/", "./a/"));
	EXPECT_FALSE(Application::DoPathsOverlap("./b/", "./a"));
	EXPECT_FALSE(Application::DoPathsOverlap("./b", "./a"));
	EXPECT_TRUE(Application::DoPathsOverlap("./a/", "./a/"));
	EXPECT_TRUE(Application::DoPathsOverlap("./a/b", "./a/"));
	EXPECT_TRUE(Application::DoPathsOverlap("./a/b", "."));
	EXPECT_TRUE(Application::DoPathsOverlap("a/b", "."));
	EXPECT_TRUE(Application::DoPathsOverlap("a/b", "./a/b"));
	EXPECT_TRUE(Application::DoPathsOverlap(".", "."));
	EXPECT_THROW(Application::DoPathsOverlap("", "./a/b"), std::invalid_argument);
}