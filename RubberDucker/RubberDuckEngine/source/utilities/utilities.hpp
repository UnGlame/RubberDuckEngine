#pragma once
#include <vector>
#include <array>

#include "utilities/clock/clock.hpp"
#include "utilities/file_parser/file_parser.hpp"

namespace RDE {
	template <typename T>
	size_t arraysizeof(const std::vector<T>& vec)
	{
		return sizeof(T) * vec.size();
	}
	
	template <typename T, size_t Size>
	size_t arraysizeof(const std::array<T, Size>&)
	{
		return sizeof(T) * Size;
	}

	template <typename... T>
	constexpr bool always_false_v = false;
}