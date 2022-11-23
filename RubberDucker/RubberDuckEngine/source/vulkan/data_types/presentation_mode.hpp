#pragma once
#include <type_traits>

namespace RDE
{
namespace Vulkan
{

enum class PresentationMode : uint32_t {
    Immediate = 0,
    AdaptiveVsync,
    Vsync,
    TripleBuffered,

    PresentationModeCount
};

}
} // namespace RDE