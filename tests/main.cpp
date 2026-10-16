#include <common.h>
#include <gtest/gtest.h>
#include <Util/log.h>

int main(int argc, char** argv)
{
	// Redirect logging to file so that it doesn't interrupt test output
	std::ofstream logFile("log.txt");
	Util::logFile = &logFile;
	std::filesystem::remove_all("./testoutput/");
	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}