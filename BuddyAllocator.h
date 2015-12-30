// Created by Ian Copland on 2015-09-19
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

#ifndef _IC_BUDDYALLOCATOR_H_
#define _IC_BUDDYALLOCATOR_H_

#include "UniquePtr.h"

#include <cstdint>
#include <memory>
#include <mutex>

namespace IC
{
    /// An efficient memory allocator which partitions memory into blocks. The root block
    /// encapsulates the entire buffer. If a smaller allocation is needed a block is
    /// split into two "buddies", each half the size of the parent buffer. This is repeated
    /// until the smallest block required for the requested allocation is found. When
    /// deallocating blocks will be re-merged if their buddy is also free.
    ///
    /// This "buddy" system results in a very efficient method of allocation and block 
    /// coalescing. However, fragmentation can be an issue if one of each of the buddy pairs
    /// is allocated, resulting in plenty of free space, but no consecutive blocks.
    ///
    /// The biggest allocation which can be made with a buddy allocator is half the size of
    /// the buffer. There is no limit to how small a requested allocation can be, though the
    /// literal memory used will be the size of the minimum block size.
    ///
    /// For a more detailed explaination of how a buddy allocator works, please see the 
    /// Bitsquid blog at: 
    /// 
    /// http://bitsquid.blogspot.co.uk/2015/08/allocation-adventures-3-buddy-allocator.html
    ///
    /// The buddy allocator is thread-safe, however it requires locking to acheive this.
    ///
    class BuddyAllocator final
    {
    public:
        /// Constructs a new allocator of the given size.
        ///
        /// @param in_bufferSize
        ///     The size of the buffer. This must be a power of two.
        /// @param in_minBlockSize
        ///	    The minimum block size. This must be a power of two.
        ///
        BuddyAllocator(std::size_t in_bufferSize, std::size_t in_minBlockSize = 64) noexcept;

        /// This thread-safe.
        ///
        /// @return The size of the buffer. 
        ///
        std::size_t getBufferSize() const noexcept { return m_bufferSize; }

        /// This thread-safe.
        ///
        /// @return The minimum block size of the buffer.
        ///
        std::size_t getMinBlockSize() const noexcept { return m_minBlockSize; }

        /// Allocates a new block of memory for the requested type. 
        /// 
        /// This is thread-safe, though it will require locking.
        ///
        /// @params The arguments for the constructor if appropriate.
        ///
        /// @return A unique pointer to the allocated memory.
        /// 
        template <typename TType, typename... TConstructorArgs> UniquePtr<TType> allocate(TConstructorArgs&&... in_constructorArgs) noexcept;

    private:
        /// Initialises the 'free' list table, which describes the first free block in any
        /// given level of the memory pool. Note that the pointers to the rest of the list
        /// are stored in the free block itself.
        ///
        /// For the sake of efficiency, this data is stored directly within the memory
        /// buffer.
        ///
        /// This is not thread-safe and should only be called during construction.
        ///
        void initFreeListTable() noexcept;

        /// Initialises the allocated table. This describes whether or not each block in the
        /// buffer is allocated or not. Note that only one value is stored for each pair of
        /// "buddies" for efficiency. When either buddy is allocated or deallocated the flag
        /// is toggled meaning it is false if neither or both are allocated. The only time
        /// this is read is during the deallocation of a block and since it already knows its
        /// own state, this is enough to determine the state of the other.
        ///        
        /// For the sake of efficiency, this data is stored directly within the memory
        /// buffer.
        ///
        /// This is not thread-safe and should only be called during construction.
        ///
        void initAllocatedTable() noexcept;

        /// Initialises the split table. This describes whether or not each possible parent
        /// block in the buffer has been split into two "buddies".
        ///
        /// For the sake of efficiency, this data is stored directly within the memory
        /// buffer.
        ///
        /// This is not thread-safe and should only be called during construction.
        ///
        void initSplitTable() noexcept;

        /// Allocates a new block of memory of the requested size.
        /// 
        /// This is thread-safe, though it will require locking.
        ///
        /// @param in_allocationSize
        ///     The size of the allocation.
        ///
        /// @return The allocated memory.
        ///
        void* allocate(std::size_t in_allocationSize) noexcept;

        /// Deallocates the given memory, returning the memory block to the free list. If
        /// appropriate the block will be re-merged with its buddy.
        /// 
        /// This is thread-safe, though it will require locking.
        ///
        /// @param in_buffer
        ///     The memory which is to be freed.
        ///
        void deallocate(void* in_buffer) noexcept;

        /// This is thread-safe.
        ///
        /// @param in_level
        ///     The level to get the block size for.
        ///
        /// @return The block size for the requested block level.
        ///
        std::size_t getBlockSize(std::size_t in_level) const noexcept;

        /// This is thread-safe.
        ///
        /// @param in_blockSize
        ///     The block size to find a block level for.
        ///
        /// @return The level which has the given block size.
        ///
        std::size_t getLevel(std::size_t in_blockSize) const noexcept;

        /// This is thread-safe.
        ///
        /// @param in_level
        ///     The requested block level.
        /// @param in_pointer
        ///     The pointer to the block.
        ///
        /// @return The index in the requested level of the given block pointer.
        ///
        std::size_t getBlockIndex(std::size_t in_level, void* in_pointer) const noexcept;

        /// This is thread-safe.
        ///
        /// @param in_level
        ///     The level of the child block.
        /// @param in_blockIndex
        ///     The index of the child block.
        ///
        /// @return The index of the parent block.
        ///
        std::size_t getParentBlockIndex(std::size_t in_level, std::size_t in_blockIndex) const noexcept;

        /// This is thread-safe
        ///
        /// @param in_level
        ///     The level of the block.
        /// @param in_blockIndex
        ///     The index of the block.
        ///
        /// @return The pointer to the block in memory.
        ///
        void* getBlockPointer(std::size_t in_level, std::size_t in_blockIndex) const noexcept;

        /// This is not thread-safe and should only be called while the mutex is held.
        ///
        /// @param in_level
        ///     The block level requested.
        ///
        /// @return The pointer to the start of the free list.
        ///
        void* getFreeListStart(std::size_t in_level) noexcept;

        /// Sets the pointer to the start of the free list for the given level.
        ///
        /// This is not thread-safe and should only be called while the mutex is held.
        ///
        /// @param in_level
        ///     The block level requested.
        /// @param in_start
        ///     The pointer to the start of the list.
        ///
        void setFreeListStart(std::size_t in_level, void* in_start) noexcept;

        /// Toggles the allocated flag for the given block index and level. Note that
        /// this flag is shared with the buddy, and that the root level block cannot 
        /// be toggled as it does not have a buddy.
        ///
        /// This is not thread-safe and should only be called while the mutex is held.
        ///
        /// @param in_level
        ///     The level of the parent.
        /// @param in_blockIndex
        ///     The block index of the parent.
        ///
        void toggleChildrenAllocated(std::size_t in_level, std::size_t in_blockIndex) noexcept;

        /// Sets wehther or not the block at the given level and index is split. Note
        /// that only blocks which can be split can be set - i.e the bottom level cannot
        /// be supplied.
        ///
        /// This is not thread-safe and should only be called while the mutex is held.
        ///
        /// @param in_level
        ///     The level of of the block which was split.
        /// @param in_blockIndex
        ///     The index of the block within the level.
        /// @param in_isSplit
        ///     Whether or not the block is split.
        ///
        void setSplit(std::size_t in_level, std::size_t in_blockIndex, bool in_isSplit) noexcept;

        const std::size_t m_bufferSize;
        const std::size_t m_minBlockSize;
        const std::size_t m_numLevels;
        const std::size_t m_freeListTableSize;
        const std::size_t m_blockDataTableSize;
        const std::size_t m_blockDataTableSizeAligned;
        const std::size_t m_blockDataTableSizeBits;
        const std::size_t m_headerSize;

        std::unique_ptr<std::uint8_t[]> m_buffer;
        void** m_freeListTable;
        void* m_allocatedTable;
        void* m_splitTable;

        std::mutex m_mutex;
    };

    //-----------------------------------------------------------------------------
    template <typename TType, typename... TConstructorArgs> UniquePtr<TType> BuddyAllocator::allocate(TConstructorArgs&&... in_constructorArgs) noexcept
    {
        void* memory = allocate(sizeof(TType));
        TType* object = new (memory) TType(std::forward<TConstructorArgs>(in_constructorArgs)...);
        return UniquePtr<TType>(object, [](TType* in_object) -> void
        {
            in_object->~TType();

            //TODO: Deallocate
        });
    }
}

#endif