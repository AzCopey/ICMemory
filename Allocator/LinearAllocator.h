// Created by Ian Copland on 2015-09-15
//
// The MIT License(MIT)
// 
// Copyright(c) 2015 Ian Copland
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

#ifndef _ICMEMORY_ALLOCATOR_LINEARALLOCATOR_H_
#define _ICMEMORY_ALLOCATOR_LINEARALLOCATOR_H_

#include "IAllocator.h"

#include <cstdint>
#include <functional>
#include <vector>

namespace IC
{
    /// A paged Linear Allocator. Within a single page all allocations are allocated
    /// linearly. If a requested allocation will not fit in the current page, a new
    /// page is allocated. It is not possible to allocate a buffer large than the
    /// size of a single page. Allocated memory is not available for reuse until
    /// after reset() has been called.
    ///
    /// A LinearAllocator can be backed by a BuddyAllocator, from which pages will be
    /// allocated, otherwise they are allocated from the free store.
    ///
    /// Note that this is not thread-safe and should not be accessed from multiple
    /// threads at the same time.
    ///
    class LinearAllocator final : public IAllocator
    {
    public:
        static constexpr std::size_t k_defaultPageSize = 4 * 1024;

        /// Initialises a new Linear Allocator with the given page size without a buddy allocator.
        /// Pages will be allocated from the freestore.
        ///
        /// @param buddyAllocator
        ///     The buddy allocator from which pages will be allocated.
        /// @param pageSize
        ///     Optional. The size of each page. Although not required, ideally pages should be powers
        ///     of two.
        /// 
        LinearAllocator(std::size_t pageSize = k_defaultPageSize) noexcept;

		/// Initialises a new Linear Allocator with the given page size and backed by the given Buddy
		/// Allocator.
		///
		/// @param parentAllocator
		///     The allocator from which pages will be allocated.
		/// @param pageSize
		///     Optional. The size of each page. Although not required, ideally pages should be powers 
		///     of two.
		/// 
		LinearAllocator(IAllocator& parentAllocator, std::size_t pageSize = k_defaultPageSize) noexcept;

        /// This thread-safe.
        ///
        /// @return The maximum allocation size from this allocator. This will always be 
        /// the size of a single page.
        ///
        std::size_t GetMaxAllocationSize() const noexcept override { return GetPageSize(); }

        /// This thread-safe.
        ///
        /// @return The size of the buffer. 
        ///
        std::size_t GetPageSize() const noexcept { return m_pageSize; }

        /// Allocates a new block of memory of the requested size.
        ///
        /// @param allocationSize
        ///     The size of the allocation.
        ///
        /// @return The allocated memory.
        ///
        void* Allocate(std::size_t allocationSize) noexcept override;

        /// Decriments the allocation count. This is checked when resetting to ensure that all previously
        /// allocated memory has been deallocated.
        ///
        /// @param pointer
        ///     The pointer to deallocate.
        ///
        void Deallocate(void* pointer) noexcept override;

        /// Resets the buffer, allowing all previously allocated memory to be reused. Deallocate() must
        /// have been called for all allocated blocks prior to reset() being called.
        /// 
        void Reset() noexcept;

		~LinearAllocator() noexcept;

    private:
		LinearAllocator(LinearAllocator&) = delete;
		LinearAllocator& operator=(LinearAllocator&) = delete;
		LinearAllocator(LinearAllocator&& toMove) = delete;
		LinearAllocator& operator=(LinearAllocator&& toMove) = delete;

        /// Creates a new page to allocate from. If there is a current page prior to this being called
        /// it will be added to the previous pages list.
        ///
        /// If the linear allocator was created using a buddy allocator then the page will be allocated
        /// from it, otherwise it will be allocated from the freestore.
        ///
        void CreatePage() noexcept;

        const std::size_t m_pageSize;

        IAllocator* m_parentAllocator;

        std::uint8_t* m_currentPage;
        std::vector<std::uint8_t*> m_previousPages;
        std::uint8_t* m_nextPointer = nullptr;

		std::size_t m_activeAllocationCount = 0;
    };
}

#endif