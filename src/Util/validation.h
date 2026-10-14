#pragma once

#include <common.h>

namespace Util {
	
// Transforms to lowercase, returns new string object and does not modify str
[[nodiscard]] std::string ToLowercase(std::string_view str);

// Tries to cast int64_t (toml int type) to the target type
// Returns nullopt if the input is out of range [min, max]
template <typename T>
	requires std::integral<T>
std::optional<T> CheckedIntegerCast(int64_t value, int64_t min = std::numeric_limits<T>::min(), int64_t max = std::numeric_limits<T>::max())
{
	if (max >= value && min <= value) {
		return static_cast<T>(value);
	}
	return std::nullopt;
}

// Like above, but clamps when out of range instead of returning nullopt
template <typename T>
	requires std::integral<T>
T ClampedIntegerCast(int64_t value, int64_t min = std::numeric_limits<T>::min(), int64_t max = std::numeric_limits<T>::max())
{
	return static_cast<T>(std::clamp(value, min, max));
}

bool DoPathsOverlap(std::filesystem::path a, std::filesystem::path b);

}