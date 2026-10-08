#include <App/app.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

TEST(AppTest, Run)
{
    EXPECT_THAT(Application::Run({ "./testdata/projects/test0/", "./testoutput/" }), std::nullopt);
    EXPECT_THAT(Application::Run({ "./testdata/projects/test0", "./testoutput/" }), std::nullopt);
    EXPECT_THAT(Application::Run({ "./testdata/projects/test1/", "./testoutput/" }), testing::Optional(ApplicationRunError::DATA_ERROR));
    EXPECT_THAT(Application::Run({ }), testing::Optional(ApplicationRunError::NOT_ENOUGH_ARGUMENTS));
    EXPECT_THAT(Application::Run({ "./doesntexist", "./testoutput/" }), testing::Optional(ApplicationRunError::INVALID_PATH));
    EXPECT_THAT(Application::Run({ "./testdata/projects/test0/", "./testdata/projects/test0/" }), testing::Optional(ApplicationRunError::INVALID_PATH));
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