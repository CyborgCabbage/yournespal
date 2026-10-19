#include "validation.h"

using namespace Util;

std::string Util::ToLowercase(std::string_view str)
{
	std::string result(str.size(), '*'); // '*' is placeholder
	std::transform(str.begin(), str.end(), result.begin(), [](unsigned char c) {
		return std::tolower(c);
	});
	return result;
}

bool Util::DoPathsOverlap(std::filesystem::path a, std::filesystem::path b)
{
	if (a.empty() || b.empty()) {
		throw std::invalid_argument("Either path a or path b had no value");
	}
	// Normalise paths
	a = std::filesystem::absolute(a.lexically_normal());
	b = std::filesystem::absolute(b.lexically_normal());
	const auto [aPart, bPart] = std::mismatch(a.begin(), a.end(), b.begin(), b.end());
	bool aEnded = (aPart == a.end() || aPart->empty());
	bool bEnded = (bPart == b.end() || bPart->empty());
	return aEnded || bEnded;
}
