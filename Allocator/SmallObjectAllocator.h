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

#ifndef _ICMEMORY_ALLOCATOR_SMALLOBJECTPOOL_H_
#define _ICMEMORY_ALLOCATOR_SMALLOBJECTPOOL_H_

#include "BlockAllocator.h"

#include <array>

namespace IC
{
    /// An allocator for allocating small objects. This is built out of muliple Block Allocators
    /// each of increasing allocation size. Allocating an object will use the smallest Block
    /// Allocator possible to reduce wasted memory. The maximum allocation size is 64 bytes for
    /// 32-bit architecture and 128 bytes for 64-bit.
    ///
    /// A SmallObjectAllocator can be backed by other allocator types, from which pages 
    /// will be allocated, otherwise they are allocated from the free store.
    ///
    /// Note that this is not thread-safe and should not be accessed from multiple
    /// threads at the same time.
    ///
    class SmallObjectAllocator final : public IAllocator
    {
    public:
        /// Creates a new small object allocator with the given buffer size; each block allocator
        /// will be this size. All allocators will be from the free store.
        ///
        /// @param bufferSize
        ///     The size of each of the internal block allocators.
        /// 
        SmallObjectAllocator(std::size_t bufferSize) noexcept;
        
        /// Creates a new small object allocator with the given buffer size; each block allocator
        /// will be this size. The given parent allocator will be used for all allocations.
        ///
        /// @param parentAllocator
        ///     The parent allocator.
        /// @param bufferSize
        ///     The size of each of the internal block allocators.
        /// 
        SmallObjectAllocator(IAllocator& parentAllocator, std::size_t bufferSize) noexcept;

        /// @return The maximum allocator size that this can allocate. This will be 64 bytes for
        /// 32-bit architecture and 128 bytes for 64-bit.
        ///
        std::size_t GetMaxAllocationSize() const noexcept override { return k_level4BlockSize; }

        /// Allocates a new block of memory of the requested size. If there is no space left in the
        /// buffer for the alloaction then this will assert.
        ///
        /// @param allocationSize
        ///     The size of the allocation.
        ///
        /// @return The allocated memory.
        ///
        void* Allocate(std::size_t allocationSize) noexcept override;

        /// Deallocates the given object, freeing it for reuse.
        ///
        /// @param pointer
        ///        The pointer which should be deallocated.
        ///
        void Deallocate(void* pointer) noexcept override;

    private:
        static constexpr std::size_t k_level1BlockSize = sizeof(std::intptr_t) * 2;
        static constexpr std::size_t k_level2BlockSize = sizeof(std::intptr_t) * 4;
        static constexpr std::size_t k_level3BlockSize = sizeof(std::intptr_t) * 8;
        static constexpr std::size_t k_level4BlockSize = sizeof(std::intptr_t) * 16;

        SmallObjectAllocator(SmallObjectAllocator&) = delete;
        SmallObjectAllocator& operator=(SmallObjectAllocator&) = delete;
        SmallObjectAllocator(SmallObjectAllocator&&) = delete;
        SmallObjectAllocator& operator=(SmallObjectAllocator&&) = delete;

        BlockAllocator m_level1Allocator;
        BlockAllocator m_level2Allocator;
        BlockAllocator m_level3Allocator;
        BlockAllocator m_level4Allocator;
    };
}

#endif