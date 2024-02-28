#pragma once
#include <limits>
#include <string>

namespace RDE {

constexpr auto k_undefinedGuid = std::numeric_limits<uint32_t>::max();

struct MeshComponent
{
    MeshComponent() = default;

    // TODO: Use GUID to represent and preload assets
    uint32_t modelGuid{k_undefinedGuid};
    uint32_t textureGuid{k_undefinedGuid};
};

} // namespace RDE
