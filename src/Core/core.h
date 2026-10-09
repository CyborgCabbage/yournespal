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

[[nodiscard]] std::string ToLowercase(std::string_view str);

template <typename A>
    requires std::integral<A>
std::optional<A> CheckedIntegerCast(int64_t value, int64_t min = std::numeric_limits<A>::min(), int64_t max = std::numeric_limits<A>::max())
{
    if (max >= value && min <= value) {
        return static_cast<A>(value);
    }
    return std::nullopt;
}

template <typename A>
    requires std::integral<A>
A ClampedIntegerCast(int64_t value, int64_t min = std::numeric_limits<A>::min(), int64_t max = std::numeric_limits<A>::max())
{
    return static_cast<A>(std::clamp(value, min, max));
}

}