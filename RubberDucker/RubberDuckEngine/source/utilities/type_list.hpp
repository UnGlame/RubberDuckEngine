#pragma once

namespace RDE {

	template <typename... Types>
	struct TypeList
	{};

	template <typename...>
	struct TypeListCat;

	template <>
	struct TypeListCat <>
	{
		using type = TypeList<>;
	};

	template <typename... Types, typename... TRest, typename... List>
	struct TypeListCat <TypeList<Types...>, TypeList <TRest...>, List...>
	{
		using type = typename TypeListCat <TypeList<Types..., TRest...>, List...>::type;
	};

	template <typename... List>
	using TypeListCat_t = typename TypeListCat<List...>::type;

}