#include <App/app.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace App;

TEST(AppTest, AppRun)
{
	// Valid config file and project, works fine
	EXPECT_THAT(::Run({ "./testdata/projects/test0/config.toml", "./testoutput/test0/" }), std::nullopt);
	// One of the pngs is unreadable
	EXPECT_THAT(::Run({ "./testdata/projects/test1/config.toml", "./testoutput/test1/" }), testing::Optional(ApplicationRunError::DATA_INVALID));
	// Insufficient number of arguments
	EXPECT_THAT(::Run({ "./testdata/projects/test1/config.toml" }), testing::Optional(ApplicationRunError::ARGUMENTS_LENGTH_WRONG));
	EXPECT_THAT(::Run({ }), testing::Optional(ApplicationRunError::ARGUMENTS_LENGTH_WRONG));
	// Overlapping paths
	EXPECT_THAT(::Run({ "./testdata/projects/test0/config.toml", "./testdata/projects/test0/" }), testing::Optional(ApplicationRunError::PATH_INVALID));
}