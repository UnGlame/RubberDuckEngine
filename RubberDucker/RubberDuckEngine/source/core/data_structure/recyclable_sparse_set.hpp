#pragma once
#include <algorithm>
#include <memory>
#include <vector>

#include "core/core.hpp"

namespace RDE {

	template <typename TIntegral, TIntegral DefaultIndex = std::numeric_limits<TIntegral>::max(), size_t PageSizeBytes = 4096>
	class RecyclableSparseSet
	{
		static_assert(std::is_integral<TIntegral>::value, "Sparse set needs to be an integral type!");
	public:
		using ValueType = TIntegral;
		using IndexType = ValueType;
		using SizeType = size_t;
		using PageIndexType = size_t;

		ValueType emplace() {
			if (m_graveyard.empty()) {
				IndexType& index = getIndexRef(m_largest);
				index = static_cast<IndexType>(m_dense.size()); // Set index to the number of values currently

				return m_dense.emplace_back(m_largest++);		// Add value to dense array
			}
			else {
				ValueType recycled = m_graveyard.back();
				m_graveyard.pop_back();

				IndexType& index = getIndexRef(recycled);
				index = static_cast<IndexType>(m_dense.size());

				return m_dense.emplace_back(recycled);
			}
		}

		void remove(ValueType value) {
			if (!exists(value)) {
				RDE_ASSERT_2(false, "Removing value {} failed; Value doesn't exist!", value);
				return;
			}

			IndexType& index = getIndexRef(value);
			m_dense[index] = m_dense.back();		// Swap last value to use this index
			getIndexRef(m_dense.back()) = index;	// Set the index of the swapped value as the new index
			m_dense.pop_back();						// Remove value
			index = DefaultIndex;					// Reset the index of the removed value 

			m_graveyard.emplace_back(value);		// Add the removed value to recyclable graveyard
		}

		void clear() {
			m_sparse.clear();
			m_dense.clear();
			m_graveyard.clear();
			m_count = 0;
		}

		template <typename TFunction>
		void forEach(TFunction function) {
			static_assert(std::is_invocable_v<TFunction, ValueType>, "Function must have ValueType as parameter!");

			for (size_t index = m_dense.size(); index > 0; --index) {
				function(m_dense[index - 1]);
			}
		}

		[[nodiscard]] constexpr bool exists(ValueType value) const {
			SizeType pageIndex = getPageIndex(value);
			SizeType pageOffset = getPageOffset(value);

			return
				pageIndex < m_sparse.size()							&&	// Page exists
				m_sparse[pageIndex]									&&	// Page pointer initialized
				m_sparse[pageIndex][pageOffset] < m_dense.size()	&&	// Dense index is within dense bounds
				m_dense[m_sparse[pageIndex][pageOffset]] == value;		// Value at dense index matches given value
		}

		[[nodiscard]] inline constexpr SizeType size() const							{ return m_dense.size(); }

	private:
		[[nodiscard]] inline constexpr SizeType getPageIndex(ValueType value) const		{ return static_cast<SizeType>(value / k_pageSize); }
		[[nodiscard]] inline constexpr SizeType getPageOffset(ValueType value) const	{ return static_cast<SizeType>(value % k_pageSize); }
		[[nodiscard]] inline constexpr SizeType getNumberOfPages() const				{ return static_cast<SizeType>(m_sparse.size()); }
		[[nodiscard]] inline constexpr IndexType& getIndexRef(ValueType value)			{ return getPage(getPageIndex(value))[getPageOffset(value)]; }

		static_assert(PageSizeBytes % sizeof(IndexType) == 0, "Page size in bytes needs to be divisible by IndexType!");
		static constexpr size_t k_pageSize = PageSizeBytes / sizeof(IndexType);

		void createPage(PageIndexType pageIndex) {
			m_sparse[pageIndex] = std::make_unique<ValueType[]>(k_pageSize);
			std::fill_n(m_sparse[pageIndex].get(), k_pageSize, DefaultIndex);
		}

		ValueType* getPage(PageIndexType pageIndex) {
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

		std::vector<std::unique_ptr<IndexType[]>> m_sparse; // Vector of pages of indices to values
		std::vector<ValueType> m_dense;						// Values
		std::vector<ValueType> m_graveyard;					// Removed values to be reused when "insert" is called

		ValueType m_largest = 0;
	};
}