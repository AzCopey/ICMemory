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

#ifndef _ICMEMORY_POOL_PAGEDOBJECTPOOL_H_
#define _ICMEMORY_POOL_PAGEDOBJECTPOOL_H_

#include "ObjectPool.h"
#include "../Container/Vector.h"

namespace IC
{
	/// An object pool which consists of a series of pages. Each page is a memory
	/// block which represents a contiquous list of objects. The pool initially
	/// contains a single page; an additional page is allocated every time there
	/// are no free objects in any of the existing pages. Once a page has been
	/// created it will not be deallocated until the PagedObjectPool is destroyed.
	///
	/// The object pool can be backed by any of the allocators.
	///
	/// This is not thread-safe and should not be accessed from multiple threads at
	/// the same time.
	///
	template <typename TObjectType> class PagedObjectPool final
	{
	public:
		static constexpr std::size_t k_defaultNumObjectsPerPage = 128;

		/// Creates a new object pool containing the given number of objects in each
		/// page. Memory used for the pool is allocated from the free store.
		///
		/// @param numObjectsPerPage
		///		Optional. The number of objects in each page. Defaults to 
		///		k_defaultNumObjectsPerPage.
		///
		PagedObjectPool(std::size_t numObjectsPerPage = k_defaultNumObjectsPerPage) noexcept;

		/// Creates a new object pool containing the given number of objects in each
		/// page. Memory used for the pool is allocated from the given allocator.
		///
		/// @param allocator
		///		The allocator from which to allocate the memory block.
		/// @param numObjectsPerPage
		///		Optional. The number of objects in each page. Defaults to 
		///		k_defaultNumObjectsPerPage.
		///
		PagedObjectPool(IAllocator& allocator, std::size_t numObjectsPerPage = k_defaultNumObjectsPerPage) noexcept;

		/// This is thread safe. 
		///
		/// @return The number of objects in each page.
		///
		std::size_t GetNumObjectsPerPage() const noexcept { return m_numObjectsPerPage; }

		/// Creates a new object from the pool. If there are no free objects in any
		/// of the current pages, a new page will be allocated.
		///
		///  @param constructorArgs
		///		The arguments for the constructor if appropriate.
		///
		/// @return The newly constructed object.
		///
		template <typename... TConstructorArgs> UniquePtr<TObjectType> Create(TConstructorArgs&&... constructorArgs) noexcept;

		~PagedObjectPool() noexcept;

	private:
		PagedObjectPool(PagedObjectPool&) = delete;
		PagedObjectPool& operator=(PagedObjectPool&) = delete;
		PagedObjectPool(PagedObjectPool&&) = delete;
		PagedObjectPool& operator=(PagedObjectPool&&) = delete;

		const std::size_t m_numObjectsPerPage;

		IAllocator* m_allocator = nullptr;

		union
		{
			std::vector<std::unique_ptr<ObjectPool<TObjectType>>> m_freeStorePools;
			Vector<UniquePtr<ObjectPool<TObjectType>>> m_allocatorPools;
		};
	};
}

#include "PagedObjectPoolImpl.h"

#endif