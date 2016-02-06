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

#ifndef _ICMEMORY_FRAMEALLOCATOR_H_
#define _ICMEMORY_FRAMEALLOCATOR_H_

#include "ForwardDeclarations.h"

#include <cstdint>
#include <functional>
#include <vector>

namespace IC
{
    /// A paged frame allocator. Within a single page all allocations are allocated
    /// linearly. If a requested allocation will not fit in the current page, a new
    /// page is allocated. It is not possible to allocate a buffer large than the
    /// size of a single page. Allocated memory is not available for reuse until the
    /// start of the next "frame" - i.e after reset() has been called.
    ///
    /// A FrameAllocator is backed by a BuddyAllocator, from which pages will be
    /// allocated.
    ///
    /// Note that this is not thread-safe and should not be accessed from multiple
    /// threads at the same time.
    ///
    class FrameAllocator final
    {
    public:
        /// Initialises a new Frame Allocator with the given page size and backed by the given Buddy
        /// Allocator.
        ///
        /// @param in_buddyAllocator
        ///     The buddy allocator from which pages will be allocated.
        /// @param in_pageSize
        ///     The size of each page. Although not required, ideally pages should be powers of two.
        /// 
        FrameAllocator(BuddyAllocator& in_buddyAllocator, std::size_t in_pageSize) noexcept;

        /// This thread-safe.
        ///
        /// @return The size of the buffer. 
        ///
        std::size_t getPageSize() const noexcept { return m_pageSize; }

        /// This thread-safe.
        ///
        /// @return The maximum allocation size from this allocator. This will always be 
        /// the size of a single page.
        ///
        std::size_t getMaxAllocationSize() const noexcept { return getPageSize(); }

        /// Allocates a new block of memory of the requested size.
        ///
        /// @param in_allocationSize
        ///     The size of the allocation.
        ///
        /// @return The allocated memory.
        ///
        void* allocate(std::size_t in_allocationSize) noexcept;

        /// Decriments the allocation count. This is checked when resetting to ensure that all previously
        /// allocated memory has been deallocated.
        ///
        /// @param in_pointer
        ///     The pointer to deallocate.
        ///
        void deallocate(void* in_pointer) noexcept;

        /// Ends the current "frame", resetting the buffer and allowing all previously allocated
        /// memory to be reused. Before reset() is called, all previously allocated UniquePtrs
        /// should have been reset().
        /// 
        void reset() noexcept;

    private:
        /// Creates a new page to allocate from. If there is a current page prior to this being called
        /// it will be added to the previous pages list.
        ///
        void createPage() noexcept;

        const std::size_t m_pageSize;

        BuddyAllocator& m_buddyAllocator;
        UniquePtr<std::uint8_t[]> m_currentPage;
        std::vector<UniquePtr<std::uint8_t[]>> m_previousPages;
        std::uint8_t* m_nextPointer = nullptr;
        std::size_t m_activeAllocationCount = 0;
    };
}

#endif