#include <common.h>
#include <gtest/gtest.h>
#include <Util/log.h>

int main(int argc, char** argv)
{
	std::ofstream logFile("log.txt");
	Util::logFile = &logFile;
	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}