#pragma once
#include <common.h>

namespace ynp {

// Exists so that logs can be pushed into a file during testing instead of interrupting the test output
extern std::ofstream* logFile;

template <class... Types>
void LogLn(const std::format_string<Types...> formatString, Types&&... args)
{
    (logFile ? *logFile : std::cout) << std::format(formatString, std::forward<Types>(args)...) << std::endl;
}

}