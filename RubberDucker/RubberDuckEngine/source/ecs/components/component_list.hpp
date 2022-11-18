#pragma once
#include "transform_component.hpp"
#include "model_component.hpp"
#include "entity_component.hpp"
#include "utilities/type_list.hpp"

namespace RDE {

	template <typename... Types>
	struct ComponentTypeList : TypeList <Types...>
	{};

	template <typename... Types>
	constexpr ComponentTypeList <Types...> componentTypes_v{};

	using ComponentList = ComponentTypeList <
		EntityComponent,
		TransformComponent,
		ModelComponent
	>;
}