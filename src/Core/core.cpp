#include "core.h"

std::ofstream* ynp::logFile = nullptr;

std::string ynp::ToLowercase(std::string_view str)
{
	std::string result{ str };
	std::transform(str.begin(), str.end(), result.begin(), [](unsigned char c) {
		return std::tolower(c);
	});
	return result;
}