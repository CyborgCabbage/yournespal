#include <Core/core.h>
#include <common.h>
#include <gtest/gtest.h>

int main(int argc, char** argv)
{
    std::ofstream logFile("log.txt");
    ynp::logFile = &logFile;
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}