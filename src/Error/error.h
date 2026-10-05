#pragma once
#include <common.h>

// Error with string view
// TODO: Would be nice if there was a way to assert these string views are always literals
namespace ynp {
	template <typename ErrorCode>
	struct Error {
		Error(ErrorCode code) : code(code) {}
		Error(ErrorCode code, std::string_view str) : code(code), str(str) {}
		// The string is optional and user facing only, so we only compare the code
		bool operator==(const Error<ErrorCode>& other) const { return other.code == this->code; }
		const ErrorCode code;
		const std::string_view str{ nullptr, 0 };
	};

	template <typename Value, typename ErrorCode>
	using Expected = std::expected<Value, ynp::Error<ErrorCode>>;

	template <typename ErrorCode>
	std::unexpected<ynp::Error<ErrorCode>> Unexpected(ErrorCode code) {
		return std::unexpected(ynp::Error<ErrorCode>(code));
	}

	template <typename ErrorCode>
	std::unexpected<ynp::Error<ErrorCode>> Unexpected(ErrorCode code, std::string_view str) {
		return std::unexpected(ynp::Error<ErrorCode>(code, str));
	}
}
