#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <App/app.h>

TEST(AppTest, Run) {
	EXPECT_THAT(Application::Run({ "./testdata/projects/test0/", "./testoutput/"  }), std::nullopt);
	EXPECT_THAT(Application::Run({ "./testdata/projects/test0", "./testoutput/" }), std::nullopt);
	EXPECT_THAT(Application::Run({ "./testdata/projects/test1/", "./testoutput/" }), testing::Optional(ApplicationRunError::DATA_ERROR));
	EXPECT_THAT(Application::Run({ }), testing::Optional(ApplicationRunError::NOT_ENOUGH_ARGUMENTS));
	EXPECT_THAT(Application::Run({"./doesntexist", "./testoutput/" }), testing::Optional(ApplicationRunError::TARGET_DIR_DOES_NOT_EXIST));
}