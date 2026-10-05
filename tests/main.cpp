#include <gtest/gtest.h>
#include <common.h>
#include <Core/log.h>

int main(int argc, char** argv) {
    std::ofstream logFile("log.txt");
    ynp::logFile = &logFile;
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}