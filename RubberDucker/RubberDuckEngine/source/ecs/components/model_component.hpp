#pragma once
#include <limits>
#include <string>

namespace RDE
{
constexpr auto k_undefinedGUID = std::numeric_limits<uint32_t>::max();

struct ModelComponent {
    ModelComponent() = default;

    // TODO: Use GUID to represent and preload assets
    uint32_t modelGUID{k_undefinedGUID};
    uint32_t textureGUID{k_undefinedGUID};
};
} // namespace RDE
