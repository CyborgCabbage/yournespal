#pragma once

#include <common.h>

template <typename T>
using MDVectorSpan = std::mdspan<T, std::dextents<uint32_t, 2>, std::layout_left>;

template <typename T>
struct MDVector {
	MDVector(size_t width, size_t height)
		: data(width * height, T{ 0 })
		, width(width)
		, height(height)
		, span(data.data(), width, height)
	{
	}
	MDVector(size_t width, size_t height, std::vector<T>&& data)
		: data(data)
		, width(width)
		, height(height)
		, span(this->data.data(), width, height)
	{
		assert(this->data.size() == this->width * this->height);
		assert(this->data.data() == span.data_handle());
	}
	// Allow moving
	MDVector(MDVector&&) = default;
	MDVector& operator=(MDVector&&) = default;
	// Data
	std::vector<T> data;
	MDVectorSpan<T> span;
	uint32_t width;
	uint32_t height;
};