#include "validation.h"

using namespace Util;

std::string Util::ToLowercase(std::string_view str)
{
	assert(str.data());
	assert(!str.empty());
	std::string result{ str };
	std::transform(str.begin(), str.end(), result.begin(), [](unsigned char c) {
		return std::tolower(c);
	});
	return result;
}

bool Util::DoPathsOverlap(std::filesystem::path a, std::filesystem::path b)
{
	assert(!a.empty());
	assert(!b.empty());
	// Normalise paths
	a = std::filesystem::absolute(a.lexically_normal());
	b = std::filesystem::absolute(b.lexically_normal());
	const auto [aPart, bPart] = std::mismatch(a.begin(), a.end(), b.begin(), b.end());
	bool aEnded = (aPart == a.end() || aPart->empty());
	bool bEnded = (bPart == b.end() || bPart->empty());
	return aEnded || bEnded;
}
