// Created by Ian Copland on 2016-05-04
//
// The MIT License(MIT)
// 
// Copyright(c) 2016 Ian Copland
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files(the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#ifndef _ICMEMORY_POOL_SMALLOBJECTPOOL_H_
#define _ICMEMORY_POOL_SMALLOBJECTPOOL_H_

#include "PagedObjectPool.h"

namespace IC
{
	/// A pool for efficent creation of small objects such as fundamental types and
	/// small classes. The pool is paged such that if the existing pages have no free
	/// objects another will be allocated. Once a page has been allocated it will not
	/// be deallocated until the SmallObjectPool is deleted.
	///
	/// The maximum size of object which can be allocated from the pool is 16x the 
	/// size of a pointer. This means larger objects can be allocated on 64-bit systems
	/// to account for larger memory requirements.
	///
	/// The object pool can be backed by any of the allocators.
	///
	/// This is not thread-safe and should not be accessed from multiple threads at
	/// the same time.
	///
	class SmallObjectPool final
	{
	public:
		static constexpr std::size_t k_defaultPageSize = 4 * 1024;

		/// This is thread-safe.
		/// 
		/// @return The maximum object size from the pool. This will be 16x the size
		/// of std::intptr_t.
		///
		static constexpr std::size_t GetMaxObjectSize() noexcept { return k_level4DataSize; }

		/// Creates a new SmallObjectPool which allocates buffers from the free store.
		/// All internal buffers will be the given page size.
		///
		/// @param pageSize
		///		Optional. The size of each page in the pool in bytes. Defaults to 
		///		k_defaultPageSize.
		///
		SmallObjectPool(std::size_t pageSize = k_defaultPageSize) noexcept;

		/// Creates a new SmallObjectPool which allocates buffers from then given 
		/// allocator. All internal buffers will be the given page size.
		///
		/// @param allocator
		///		The allocator which should be used for all allocations.
		/// @param pageSize
		///		Optional. The size of each page in the pool in bytes. Defaults to 
		///		k_defaultPageSize.
		///
		SmallObjectPool(IAllocator& allocator, std::size_t pageSize = k_defaultPageSize) noexcept;

		/// This is thread safe. 
		///
		/// @return The size of each page in bytes.
		///
		std::size_t GetPageSize() const noexcept { return m_pageSize; }

		/// Creates a new object from the pool. If there are currently no free objects
		/// of the correct size in the pool then a new page will be allocated.
		///
		///  @param constructorArgs
		///		The arguments for the constructor if appropriate.
		///
		/// @return The newly constructed object.
		///
		template <typename TObjectType, typename... TConstructorArgs> UniquePtr<TObjectType> Create(TConstructorArgs&&... constructorArgs) noexcept;

	private:
		static constexpr std::size_t k_level1DataSize = sizeof(std::uintptr_t) * 2;
		static constexpr std::size_t k_level2DataSize = sizeof(std::uintptr_t) * 4;
		static constexpr std::size_t k_level3DataSize = sizeof(std::uintptr_t) * 8;
		static constexpr std::size_t k_level4DataSize = sizeof(std::uintptr_t) * 16;

		/// A basic structure for representing arbitrarily sized blocks of data. This
		/// is used as the underlying data type for the differently sized data pools.
		///
		template <std::size_t TDataSize> struct DataBlock final
		{
			std::int8_t m_data[TDataSize];
		};

		SmallObjectPool(SmallObjectPool&) = delete;
		SmallObjectPool& operator=(SmallObjectPool&) = delete;
		SmallObjectPool(SmallObjectPool&&) = delete;
		SmallObjectPool& operator=(SmallObjectPool&&) = delete;

		/// Creates a new object of the specified type and size from the given object
		/// pool.
		///
		/// @param pool
		///		The object pool which should be used to create the object.
		///  @param constructorArgs
		///		The arguments for the constructor if appropriate.
		///
		/// @return The newly constructed object.
		///
		template <typename TObjectType, int TBlockSize, typename... TConstructorArgs>  
		UniquePtr<TObjectType> CreateFromPool(PagedObjectPool<DataBlock<TBlockSize>>& pool, TConstructorArgs&&... constructorArgs) const noexcept;

		const std::size_t m_pageSize;

		PagedObjectPool<DataBlock<k_level1DataSize>> m_level1Pool;
		PagedObjectPool<DataBlock<k_level2DataSize>> m_level2Pool;
		PagedObjectPool<DataBlock<k_level3DataSize>> m_level3Pool;
		PagedObjectPool<DataBlock<k_level4DataSize>> m_level4Pool;
	};
}

#include "SmallObjectPoolImpl.h"

#endif