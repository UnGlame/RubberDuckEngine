#pragma once
#include "model_component.hpp"
<<<<<<< HEAD
#include "entity_component.hpp"
=======
#include "transform_component.hpp"
>>>>>>> main
#include "utilities/type_list.hpp"

namespace RDE
{

template <typename... Types> struct ComponentTypeList : TypeList<Types...> {
};

template <typename... Types>
constexpr ComponentTypeList<Types...> componentTypes_v{};

<<<<<<< HEAD
	using ComponentList = ComponentTypeList <
		EntityComponent,
		TransformComponent,
		ModelComponent
	>;
}
=======
using ComponentList = ComponentTypeList<TransformComponent, ModelComponent>;
} // namespace RDE
>>>>>>> main
