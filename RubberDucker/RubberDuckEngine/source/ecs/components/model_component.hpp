#pragma once
#include <string>

namespace RDE {

struct ModelComponent {
    ModelComponent() = default;

    // TODO: Use GUID to represent and preload assets
    uint32_t modelGUID;
    uint32_t textureGUID;
};
} // namespace RDE
