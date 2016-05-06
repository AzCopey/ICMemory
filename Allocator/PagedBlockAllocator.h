// Created by Ian Copland on 2016-05-06
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

#ifndef _ICMEMORY_ALLOCATOR_PAGEDBLOCKALLOCATOR_H_
#define _ICMEMORY_ALLOCATOR_PAGEDBLOCKALLOCATOR_H_

#include "BlockAllocator.h"
#include "../Container/UniquePtr.h"
#include "../Container/Vector.h"

namespace IC
{
	/// A paged version of the PagedBlockAllocator. This allocates fixed size blocks of memory
	/// from pages. If an allocation is requested when no pages have any free blocks then
	/// a new page is allocated. Pages are not deallocated until the allocator is destroyed.
	///
	/// A PagedBlockAllocator can be backed by other allocator types, from which pages will 
	/// be allocated, otherwise they are allocated from the free store.
	///
	/// Note that this is not thread-safe and should not be accessed from multiple
	/// threads at the same time.
	///
	class PagedBlockAllocator final : public IAllocator
	{
	public:
		/// Creates a new PagedBlockAllocator with pages allocated from the free store.
		///
		/// @param blockSize
		///		The size of each block in the allocator. The block size must be multiple
		///		of the size of a pointer, at at least twice the size of a pointer.
		/// @param numBlocksPerPage
		///		The number of blocks available in each page.
		/// 
		PagedBlockAllocator(std::size_t blockSize, std::size_t numBlocksPerPage) noexcept;

		/// Creates a new PagedBlockAllocator with pages allocated from the given allocator.
		///
		/// @param allocator
		///		The allocator from which to allocate pages.
		/// @param blockSize
		///		The size of each block in the allocator. The block size must be multiple
		///		of the size of a pointer, at at least twice the size of a pointer.
		/// @param numBlocksPerPage
		///		The number of blocks available in each page.
		/// 
		PagedBlockAllocator(IAllocator& parentAllocator, std::size_t blockSize, std::size_t numBlocksPerPage) noexcept;

		/// This is thread safe.
		///
		/// @return The maximum allocation size from this allocator. Will be the size of a 
		/// single block.
		///
		std::size_t GetMaxAllocationSize() const noexcept override { return GetBlockSize(); }

		/// This is thread-safe.
		///
		/// @return The size of each block in the allocator.
		///
		std::size_t GetBlockSize() const noexcept { return m_blockSize; }

		/// This is thread-safe.
		///
		/// @return The total number of blocks available in each page.
		///
		std::size_t GetNumBlocksPerPage() const noexcept { return m_numBlocksPerPage; }

		/// Allocates a block from the allocator. The allocation size must be smaller than 
		/// that of a block, otherwise this will assert. If there are no free blocks in any
		/// available pages, then a new page will be allocated.
		///
		/// @param allocationSize
		///		The size of allocation required.
		///
		/// @return The block of memory.
		///
		void* Allocate(std::size_t allocationSize) noexcept override;

		/// Deallocates the given block, freeing it for reuse.
		///
		/// @param pointer
		///		The pointer which should be deallocated.
		///
		void Deallocate(void* pointer) noexcept override;

		~PagedBlockAllocator() noexcept;

	private:
		PagedBlockAllocator(PagedBlockAllocator&) = delete;
		PagedBlockAllocator& operator=(PagedBlockAllocator&) = delete;
		PagedBlockAllocator(PagedBlockAllocator&&) = delete;
		PagedBlockAllocator& operator=(PagedBlockAllocator&&) = delete;

		const std::size_t m_blockSize;
		const std::size_t m_numBlocksPerPage;
		const std::size_t m_pageSize;

		IAllocator* m_parentAllocator = nullptr;

		union
		{
			std::vector<std::unique_ptr<BlockAllocator>> m_freeStoreBlockAllocators;
			Vector<UniquePtr<BlockAllocator>> m_parentAllocatorBlockAllocators;
		};
	};
}

#endif