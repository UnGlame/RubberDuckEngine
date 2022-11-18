#pragma once
#include <type_traits>

namespace RDE {
namespace Vulkan {

	enum BindingIDs : uint32_t
	{
		VertexBufferBindingID = 0,
		InstanceBufferBindingID = 1,

		bufferBindingCount
	};
}
}