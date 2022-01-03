#pragma once
#include <algorithm>
#include <memory>
#include <vector>
#include <optional>

#include "core/core.hpp"

namespace RDE {
	
	template <typename TIntegral, TIntegral DefaultIndex = std::numeric_limits<TIntegral>::max(), size_t PageSizeBytes = 4096>
	class SparseSet
	{
		static_assert(std::is_integral<TIntegral>::value, "Sparse set needs to be an integral type!");
	public:
		using ValueType		= TIntegral;
		using IndexType		= ValueType;
		using SizeType		= size_t;
		using PageIndexType = SizeType;

		void emplace(ValueType value) {
			if (exists(value)) {
				RDE_ASSERT_2(false, "Value {} already exists in this container!", value);
				return;
			}
			auto& index = getIndexRef(value);
			index = static_cast<IndexType>(m_dense.size());

			m_dense.emplace_back(value);
		}

		void remove(ValueType value) {
			if (!exists(value)) {
				RDE_ASSERT_2(false, "Removing value {} failed; Value doesn't exist!", value);
				return;
			}

			ValueType lastValue = m_dense.back();
			IndexType& lastIndex = getIndexRefFast(lastValue);
			IndexType& valueIndex = getIndexRefFast(value);

			m_dense[valueIndex] = lastValue;				// Set value as the last value
			m_dense.pop_back();								// Remove value at the back
			lastIndex = valueIndex;							// Set the index of the last value as the new index
			valueIndex = DefaultIndex;						// Reset the index of the removed value
		}

		void clear() {
			m_sparse.clear();
			m_dense.clear();
		}

		template <typename TFunction>
		void forEach(TFunction function) {
			static_assert(std::is_invocable_v<TFunction, ValueType>, "Function must have ValueType as parameter!");

			for (size_t index = m_dense.size(); index > 0; --index) {
				function(m_dense[index - 1]);
			}
		}

		void swap(ValueType left, ValueType right) {
			if (!exists(left) || !exists(right)) {
				RDE_ASSERT_2(false, "Swap failed; Left value {0} and/or right value {1} do not exist!", lhs, rhs);
				return;
			}

			IndexType& leftIndex = getIndexRefFast(left);
			IndexType& rightIndex = getIndexRefFast(right);

			std::swap(leftIndex, rightIndex);					// Swap indices
			std::swap(m_dense[leftIndex], m_dense[rightIndex]);	// Swap values
		}

		[[nodiscard]] constexpr bool exists(ValueType value) const {
			SizeType pageIndex = getPageIndex(value);
			SizeType pageOffset = getPageOffset(value);

			return
				pageIndex < m_sparse.size()							&&	// Page exists
				m_sparse[pageIndex]									&&	// Page pointer initialized
				m_sparse[pageIndex][pageOffset] != DefaultIndex;		// Index is not default value
		}

		[[nodiscard]] inline constexpr bool				empty() const							{ return m_dense.empty(); }
		[[nodiscard]] inline constexpr SizeType			size() const							{ return m_dense.size(); }
		[[nodiscard]] inline constexpr SizeType			getIndex(ValueType value) const			{ return static_cast<SizeType>(m_sparse[getPageIndex(value)][getPageOffset(value)]); }
		[[nodiscard]] inline constexpr ValueType		getValue(SizeType index) const			{ return m_dense[index]; }

	private:
		[[nodiscard]] inline constexpr PageIndexType	getPageIndex(ValueType value) const		{ return static_cast<SizeType>(value / k_pageSize); }
		[[nodiscard]] inline constexpr IndexType		getPageOffset(ValueType value) const	{ return static_cast<SizeType>(value % k_pageSize); }
		[[nodiscard]] inline constexpr SizeType			getNumberOfPages() const				{ return static_cast<SizeType>(m_sparse.size()); }
		[[nodiscard]] inline constexpr IndexType&		getIndexRef(ValueType value)			{ return getPage(getPageIndex(value))[getPageOffset(value)]; }
		[[nodiscard]] inline constexpr IndexType&		getIndexRefFast(ValueType value)		{ return m_sparse[getPageIndex(value)][getPageOffset(value)]; }

		static_assert(PageSizeBytes % sizeof(IndexType) == 0, "Page size in bytes needs to be divisible by IndexType!");
		static constexpr size_t k_pageSize = PageSizeBytes / sizeof(IndexType);

		void createPage(PageIndexType pageIndex) {
			m_sparse[pageIndex] = std::make_unique<ValueType[]>(k_pageSize);
			std::fill_n(m_sparse[pageIndex].get(), k_pageSize, DefaultIndex);
		}

		[[nodiscard]] constexpr ValueType* getPage(PageIndexType pageIndex) {
			// If page index exceeds number of current pages, resize
			if (pageIndex >= getNumberOfPages()) {
				m_sparse.resize(pageIndex + 1);
			}

			// Check if page has been created, initialize if haven't
			if (m_sparse[pageIndex] == nullptr) {
				createPage(pageIndex);
			}

			return m_sparse[pageIndex].get();
		}

		class Iterator final
		{
			using InstanceType = std::vector<ValueType>;
			using IndexType = int32_t;

			friend class SparseSet;

			// Private constructor to be used by SparseSet only
			Iterator(const InstanceType& ref, const IndexType index) : m_instance(ref), m_index(index) {}

		public:
			using DiffType = IndexType;
			using ValueType = SparseSet::ValueType;
			using Pointer = const ValueType*;
			using Reference = const ValueType&;
			using IteratorCategory = std::random_access_iterator_tag;

			Iterator&	operator++()								{ --m_index;							return *this; }
			Iterator	operator++(int)								{ Iterator copy(*this); operator++();	return copy; }

			Iterator&	operator--()								{ ++m_index;							return *this; }
			Iterator	operator--(int)								{ Iterator copy(*this); operator--();	return copy; }
			
			Iterator&	operator+=(const DiffType value)			{ m_index -= value;						return *this; }
			Iterator	operator+(const DiffType value)		const	{ Iterator copy(*this);					return copy += value; }
			
			Iterator&	operator-=(const DiffType value)			{ return *this += -value; }
			Iterator	operator-(const DiffType value)		const	{ return *this + -value; }
			DiffType	operator-(const Iterator& rhs)		const	{ return rhs.m_index - m_index; }

			Reference	operator[](const DiffType value)	const	{ return m_instance[m_index - value - 1]; }
			Pointer		operator->()						const	{ return &(m_instance[m_index - 1]); }
			Reference	operator*()							const	{ return *operator->(); }

			bool		operator==(const Iterator& rhs)		const	{ return m_index == rhs.m_index; }
			bool		operator!=(const Iterator& rhs)		const	{ return !(*this == rhs); }
			
			bool		operator<(const Iterator& rhs)		const	{ return m_index > rhs.m_index; }
			bool		operator<=(const Iterator& rhs)		const	{ return !(*this > rhs); }
			
			bool		operator>(const Iterator& rhs)		const	{ return m_index < rhs.m_index; }
			bool		operator>=(const Iterator& rhs)		const	{ return !(*this < rhs); }

		private:
			const InstanceType& m_instance;
			IndexType m_index;
		};

	public:
		[[nodiscard]] inline constexpr Iterator	begin()		const	{ return Iterator(m_dense, static_cast<Iterator:IndexType>(m_dense.size())); }
		[[nodiscard]] inline constexpr Iterator	end()		const	{ return Iterator(m_dense, 0); }

	private:
		std::vector<std::unique_ptr<IndexType[]>> m_sparse; // Pages of indices to values
		std::vector<ValueType> m_dense;						// Values
	};
}