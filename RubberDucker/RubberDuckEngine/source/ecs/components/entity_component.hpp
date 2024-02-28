#pragma once
#include <string>

namespace RDE {

struct EntityComponent
{
    EntityComponent() = default;
    std::string name{};
};

} // namespace RDE
