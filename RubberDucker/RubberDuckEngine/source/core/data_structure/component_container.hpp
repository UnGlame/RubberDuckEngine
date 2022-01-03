#pragma once
#include "core/data_structure/sparse_set.hpp"
#include "core/data_structure/entity.hpp"

namespace RDE {
	
	template <typename TComponent, typename = std::void_t<>>
	class ComponentContainer : public SparseSet<Entity>
	{
		using BaseClassType = SparseSet<Entity>;
		using SizeType = size_t;
	public:

		template <typename... TArgs>
		TComponent& emplace(Entity entity, TArgs&&... args) noexcept {
			BaseClassType::emplace(entity);
		
			if constexpr (sizeof...(TArgs) == 0) {
				return m_components.emplace_back();
			}
			else if constexpr (std::is_aggregate_v<TComponent>) {
				return m_components.emplace_back(TComponent{ std::forward<TArgs>(args)... });
			}
			else {
				return m_components.emplace_back(std::forward<TArgs>(args)...);
			}
		}

		template <typename... TArgs>
		TComponent& tryEmplace(Entity entity, TArgs&&... args) {
			if (exists(entity)) {
				remove(entity);
			}
			
			BaseClassType::emplace(entity);
		
			if constexpr (std::is_aggregate_v<Component>) {
				return m_components.emplace_back(TComponent{ std::forward<TArgs>(args)... });
			}
			else {
				return m_component.emplace_back(std::forward<TArgs>(args)...);
			}
		}

		template <typename TFunction>
		void update(Entity entity, TFunction&& function) {
			RDE_ASSERT_2(exists(entity), "Entity does not exist!");
			function(getComponent(entity));
		}

		void swap(Entity left, Entity right) {
			std::swap(getComponent(left), getComponent(right));
			BaseClassType::swap(left, right);
		}

		void remove(Entity entity) {
			getComponent(entity) = std::move(m_components.back());
			m_components.pop_back();
			BaseClassType::remove(entity);
		}

		void clear() {
			m_components.clear();
			BaseClassType::clear();
		}

		template <typename TFunction>
		void forEach(TFunction&& function) {
			if constexpr (std::is_invocable_v<TFunction, TComponent>) {
				for (auto index = m_components.size(); index > 0; --index) {
					function(m_components[index - 1]);
				}
			}
			else if constexpr (std::is_invocable_v<TFunction, Entity>) {
				BaseClassType::forEach(std::move(function));
			}
			else if constexpr (std::is_invocable_v<TFunction, Entity, TComponent>) {
				for (auto index = m_components.size(); index > 0; --index) {
					function(BaseClassType::getValue(index - 1), m_components[index - 1]);
				}
			}
			else if constexpr (std::is_invocable_v<TFunction, TComponent, Entity>) {
				for (auto index = m_components.size(); index > 0; --index) {
					function(m_components[index - 1], BaseClassType::getValue(index - 1));
				}
			}
			else {
				for (auto index = m_components.size(); index > 0; --index) {
					function();
				}
			}
		}

		template <typename TFunction>
		Entity find(TFunction&& function) {
			if constexpr (std::is_invocable_v<TFunction, TComponent>) {
				for (auto index = m_components.size(); index > 0; --index) {
					if (function(m_components[index - 1])) {
						return BaseClassType::getValue(index - 1);
					}
				}
			}
			else {
				for (auto index = m_components.size(); index > 0; --index) {
					if (function()) {
						return BaseClassType::getValue(index - 1);
					}
				}
			}
			return k_nullEntity;
		}

		// TODO: serialize, deserialize, clone, updateClone

		[[nodiscard]] inline constexpr SizeType size() const						{ return m_components.size(); }
		[[nodiscard]] inline constexpr bool	exists(Entity entity) const				{ return BaseClassType::exists(value); }
		[[nodiscard]] inline TComponent& getComponent(Entity entity)				{ return m_components[BaseClassType::getIndex(entity)]; }
		[[nodiscard]] inline const TComponent& getComponent(Entity entity) const	{ return m_components[BaseClassType::getIndex(entity)]; }
		[[nodiscard]] inline auto tryGetComponent(Entity entity)					{ return exists(entity) ? std::optional<std::reference_wrapper<TComponent>>(getComponent(entity)) : std::nullopt; }
		[[nodiscard]] inline auto tryGetComponent(Entity entity)const				{ return exists(entity) ? std::optional<std::reference_wrapper<const TComponent>>(getComponent(entity)) : std::nullopt; }

	private:
		template <bool Const>
		class Iterator final
		{
			using InstanceType = std::conditional_t<Const, const std::vector<TComponent>, std::vector<TComponent>>;
			using IndexType = int32_t;

			friend class ComponentContainer<typename TComponent>;

			// Private constructor to be used by ComponentContainer only
			Iterator(const InstanceType& ref, const IndexType index) : m_instance(ref), m_index(index) {}

		public:
			using DiffType = IndexType;
			using ValueType = TComponent;
			using Pointer = std::conditional_t<Const, const ValueType*, ValueType*>;
			using Reference = std::conditional_t<Const, const ValueType&, ValueType&>;
			using IteratorCategory = std::random_access_iterator_tag;

			Iterator&	operator++()											{ --m_index;							return *this; }
			Iterator	operator++(int)											{ Iterator copy(*this); operator++();	return copy; }

			Iterator&	operator--()											{ ++m_index;							return *this; }
			Iterator	operator--(int)											{ Iterator copy(*this); operator--();	return copy; }

			Iterator&	operator+=(const DiffType value)						{ m_index -= value;						return *this; }
			Iterator	operator+(const DiffType value)		const				{ Iterator copy(*this);					return copy += value; }

			Iterator&	operator-=(const DiffType value)						{ return *this += -value; }
			Iterator	operator-(const DiffType value)		const				{ return *this + -value; }
			DiffType	operator-(const Iterator& rhs)		const				{ return rhs.m_index - m_index; }

			Reference	operator[](const DiffType value)	const				{ return m_instance[m_index - value - 1]; }
			Pointer		operator->()						const				{ return &(m_instance[m_index - 1]); }
			Reference	operator*()							const				{ return *operator->(); }

			bool		operator==(const Iterator& rhs)		const				{ return m_index == rhs.m_index; }
			bool		operator!=(const Iterator& rhs)		const				{ return !(*this == rhs); }

			bool		operator<(const Iterator& rhs)		const				{ return m_index > rhs.m_index; }
			bool		operator<=(const Iterator& rhs)		const				{ return !(*this > rhs); }

			bool		operator>(const Iterator& rhs)		const				{ return m_index < rhs.m_index; }
			bool		operator>=(const Iterator& rhs)		const				{ return !(*this < rhs); }

		private:
			InstanceType& m_instance;
			IndexType m_index;
		};

	public:
		using IteratorType = Iterator<false>;
		using ConstIteratorType = Iterator<true>;

		[[nodiscard]] inline constexpr IteratorType begin()				{ return IteratorType(m_components, static_cast<IteratorType::IndexType>(m_components.size())); }
		[[nodiscard]] inline constexpr IteratorType begin()		const	{ return ConstIteratorType(m_components, static_cast<IteratorType::IndexType>(m_components.size())); }
		[[nodiscard]] inline constexpr IteratorType cbegin()	const	{ return begin(); }

		[[nodiscard]] inline constexpr IteratorType end()				{ return IteratorType(m_components, 0); }
		[[nodiscard]] inline constexpr IteratorType end()		const	{ return ConstIteratorType(m_components, 0); }
		[[nodiscard]] inline constexpr IteratorType cend()		const	{ return end(); }

	private:
		std::vector<TComponent> m_components;
	};

}