#pragma once
#include <common.h>

namespace ynp {
	extern std::ofstream* logFile;

	template <class... Types>
	void LogLn(const std::format_string<Types...> formatString, Types&&... args) {
		(logFile ? *logFile : std::cout) << std::format(formatString, std::forward<Types>(args)...) << std::endl;
	}
}