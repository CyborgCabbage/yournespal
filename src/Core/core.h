#pragma once
#include <common.h>

namespace ynp {

// Exists so that logs can be pushed into a file during testing instead of interrupting the test output
extern std::ofstream* logFile;

// Like std::println but can be redirected to a file
template <class... Types>
void LogLn(const std::format_string<Types...> formatString, Types&&... args)
{
    (logFile ? *logFile : std::cout) << std::format(formatString, std::forward<Types>(args)...) << std::endl;
}

// Transforms to lowercase, returns new string object and does not modify str
[[nodiscard]] std::string ToLowercase(std::string_view str);

// Tries to cast int64_t (toml int type) to the target type
// Returns nullopt if the input is out of range [min, max]
template <typename A>
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

// Parses toml array to a vector of a single type, logs and skips invalid values
template <typename T, typename F = std::nullptr_t>
    requires std::predicate<F, T> || std::same_as<F, std::nullptr_t>
static std::vector<T> ParseArray(const toml::array& arr, F&& predicate = nullptr) {
    std::vector<T> result;
    for (int i = 0; i < arr.size(); i++) {
        const T* found = arr.get_as<T>(i);
        if ()
        if (found && ) {
            result.push_back(*found);
        } else {
            
        }
    }
    return result;
}


}