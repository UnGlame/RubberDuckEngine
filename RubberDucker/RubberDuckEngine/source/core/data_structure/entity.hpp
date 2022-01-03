#pragma once
#include <type_traits>
#include <limits>

namespace RDE {
	using Entity = uint32_t;
	static constexpr Entity k_nullEntity = std::numeric_limits<Entity>::max();
}