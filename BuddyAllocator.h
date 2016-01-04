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

        /// Encapsulates functionality required for navigating the free list table.
        /// This allocates no memory, instead relying on the memory provided in the
        /// constructor. This allows the buddy allocator to store the free list inside
        /// its buffer.
        ///
        /// The given buffer is used to store the first element in the free list for
        /// each level in the free list table. Each element in the free list then stores
        /// the pointers to the previous and next elements.
        ///
        /// This is not thread-safe, so the buddy allocator mutex should always be held
        /// when calling any of the free list table's methods.
        ///
        class FreeListTable final
        {
        public:

            /// Constructs an empty free list.
            ///
            FreeListTable() noexcept {}

            /// Initialises the free list table with the given memory buffer. 
            ///
            /// @param in_numBlockLevels
            ///     The number of levels in the table.
            /// @param in_buffer
            ///     The buffer in which the first element in each list should
            ///     be stored.
            ///
            FreeListTable(std::size_t in_numBlockLevels, void* in_buffer) noexcept;

            /// This is not thread-safe.
            ///
            /// @param in_tableLevel
            ///     The table level requested.
            ///
            /// @return The pointer to the start of the free list.
            ///
            void* getStart(std::size_t in_tableLevel) const noexcept;

            /// This is not thread-safe.
            ///
            /// @param in_listElement
            ///     The pointer to an element in the free list.
            ///
            /// @return The pointer to the next element in the free list.
            ///
            void* getNext(void* in_listElement) const noexcept;

            /// This is not thread-safe.
            ///
            /// @param in_listElement
            ///     The pointer to an element in the free list.
            ///
            /// @return The pointer to the previous element in the free list.
            ///
            void* getPrevious(void* in_listElement) const noexcept;

            /// Adds the given pointer to the start of the free list.
            ///
            /// This is not thread-safe.
            ///
            /// @param in_tableLevel
            ///     The table level requested.
            /// @param in_start
            ///     The pointer to add of the list.
            ///
            void add(std::size_t in_tableLevel, void* in_listElement) noexcept;

            /// Removes the given pointer from the free list.
            ///
            /// This is not thread-safe.
            ///
            /// @param in_start
            ///     The pointer to remove.
            ///
            void remove(std::size_t in_tableLevel, void* in_listElement) noexcept;

        private:
            /// A container for the next and previous elements in a list.
            ///
            struct ListNode final
            {
                ListNode* m_previous = nullptr;
                ListNode* m_next = nullptr;
            };

            std::size_t m_numBlockLevels = 0;
            ListNode** m_freeListTable = nullptr;
        };

        /// Encapsulates functionality for accessing the allocated table. This requires no
        /// memory, instead using the buffer provided in the constructor. This allows the
        /// Buddy Allocator to use memory inside its buffer.
        ///
        /// For each block pair in the table a single bit is stored. This is toggled each
        /// time either or them is allocated or deallocated. This means that a false value
        /// indicates that neither or both of them are allocated and a true value indicates
        /// that one or the other is allocated. As this is only used when deallocating a
        /// block, we already know the state of one of them meaning we can determine the
        /// state of the other.
        ///
        /// Note that this means that the block at level 0 does not have an entry as it 
        /// does not have a buddy. This block can be assumed to always be allocated.
        ///
        /// This is not thread-safe, so the buddy allocator mutex should always be held
        /// when calling any of the free list table's methods.
        ///
        class AllocatedTable final
        {
        public:
            /// Constructs an empty allocated table.
            ///
            AllocatedTable() noexcept {}

            /// Initialises the allocated table with the given memory buffer. 
            ///
            /// @param in_numBlockLevels
            ///     The number of levels in the table.
            /// @param in_buffer
            ///     The buffer in which the allocated table should be stored.
            ///
            AllocatedTable(std::size_t in_numBlockLevels, void* in_buffer) noexcept;

            /// This is not thread-safe.
            ///
            /// @param in_blockLevel
            ///     The level of the block.
            /// @param in_blockIndex
            ///     The index of the block.
            ///
            /// @return The allocated flag for the described block and its buddy.
            bool getAllocatedFlag(std::size_t in_blockLevel, std::size_t in_blockIndex) const noexcept;

            /// Toggles the allocated flag for the given block and its buddy.
            ///
            /// This is not thread-safe.
            ///
            /// @param in_blockLevel
            ///     The level of the block.
            /// @param in_blockIndex
            ///     The index of the block.
            ///
            void toggleAllocatedFlag(std::size_t in_blockLevel, std::size_t in_blockIndex) noexcept;

        private:
            std::size_t m_numBlockLevels = 0;
            void* m_allocatedTable;
        };

        /// Encapsulates functionality for accessing the split table. This requires no
        /// memory, instead using the buffer provided in the constructor. This allows the
        /// Buddy Allocator to use memory inside its buffer.
        ///
        /// Describes, for each possible parent, whether or not it is currently split
        /// into child buffers.
        ///
        /// This is not thread-safe, so the buddy allocator mutex should always be held
        /// when calling any of the free list table's methods.
        ///
        class SplitTable final
        {
        public:
            /// Constructs an empty split table.
            ///
            SplitTable() noexcept {}

            /// Initialises the split table with the given memory buffer. 
            ///
            /// @param in_numBlockLevels
            ///     The number of levels in the table. Note this that is the number of
            ///     levels which can have children, so one less than the typical number
            ///     of block levels.
            /// @param in_buffer
            ///     The buffer in which the split table should be stored.
            ///
            SplitTable(std::size_t in_numBlockLevels, void* in_buffer) noexcept;

            /// This is not thread-safe.
            ///
            /// @param in_blockLevel
            ///     The level of the block.
            /// @param in_blockIndex
            ///     The index of the block.
            ///
            /// @return Whether or not the request
            bool isSplit(std::size_t in_blockLevel, std::size_t in_blockIndex) const noexcept;

            /// Toggles the allocated flag for the given block and its buddy.
            ///
            /// This is not thread-safe.
            ///
            /// @param in_blockLevel
            ///     The level of the block.
            /// @param in_blockIndex
            ///     The index of the block.
            /// @param in_isSplit
            ///     Wheter or not the block should be split.
            ///
            void setSplit(std::size_t in_blockLevel, std::size_t in_blockIndex, bool in_isSplit) noexcept;

        private:
            std::size_t m_numBlockLevels = 0;
            void* m_splitTable;
        };

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
        /// buffer is allocated or not. 
        ///
        /// For each block pair in the table a single bit is stored. This is toggled each
        /// time either or them is allocated or deallocated. This means that a false value
        /// indicates that neither or both of them are allocated and a true value indicates
        /// that one or the other is allocated. As this is only used when deallocating a
        /// block, we already know the state of one of them meaning we can determine the
        /// state of the other.
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
        /// @param in_blockPointer
        ///     The memory which is to be freed.
        ///
        void deallocate(void* in_blockPointer) noexcept;

        /// This is thread-safe.
        ///
        /// @param in_blockLevel
        ///     The level to get the block size for.
        ///
        /// @return The block size for the requested block level.
        ///
        std::size_t getBlockSize(std::size_t in_blockLevel) const noexcept;

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
        /// @param in_blockLevel
        ///     The requested block level.
        /// @param in_blockPointer
        ///     The pointer to the block.
        ///
        /// @return The index in the requested level of the given block pointer.
        ///
        std::size_t getBlockIndex(std::size_t in_blockLevel, void* in_blockPointer) const noexcept;

        /// This is thread-safe.
        ///
        /// @param in_blockLevel
        ///     The level of the child block.
        /// @param in_blockIndex
        ///     The index of the child block.
        ///
        /// @return The index of the parent block.
        ///
        std::size_t getParentBlockIndex(std::size_t in_blockLevel, std::size_t in_blockIndex) const noexcept;

        /// Calculates the indices of the children of the given block.
        /// 
        /// This is thread-safe.
        ///
        /// @param in_parentBlockLevel
        ///     The level of the parent block.
        /// @param in_parentBlockIndex
        ///     The index of the parent block.
        /// @param out_childBlockIndexA
        ///     (Out) The first child index.
        /// @param out_childBlockIndexB
        ///     (Out) The second child index.
        ///
        void getChildBlockIndices(std::size_t in_parentBlockLevel, std::size_t in_parentBlockIndex, std::size_t& out_childBlockIndexA, std::size_t& out_childBlockIndexB) const noexcept;

        /// This is thread-safe.
        ///
        /// @param in_blockLevel
        ///     The requested block level.
        /// @param in_blockIndex
        ///     The index of the block.
        ///
        /// @return The block pointer described by the given block level and index.
        ///
        void* getBlockPointer(std::size_t in_blockLevel, std::size_t in_blockIndex) const noexcept;

        /// Calculates both the level and index of the given allocated block pointer. 
        /// If the block is not allocated, or the pointer is not to a valid block, this
        /// will assert.
        /// 
        /// This is not thread-safe and should only be called while the mutex is held.
        ///
        /// @param in_blockPointer
        ///     The pointer to the block.
        /// @param out_level
        ///     (Out) The output level.
        /// @param out_index
        ///     (Out) The output index.
        ///
        void getAllocatedBlockInfo(void* in_blockPointer, std::size_t& out_level, std::size_t& out_index) const noexcept;

        /// Splits a block in the requested level into two blocks one level higher. If
        /// There are no available blocks at the requested level blocks will be split
        /// recursively at lower levels. If no block can be split after recursion then
        /// the memory pool has run out of memory and will assert.
        ///
        /// This is not thread-safe and should only be called while the mutex is held.
        ///
        /// @param in_blockLevel
        ///     The block level which should be split. Cannot be the lowest or highest
        ///     block levels.
        ///
        void splitBlock(std::size_t in_blockLevel) noexcept;

        /// Tries to merge the given block. If successful, this will recurse and try to
        /// merge its parent. This must only be called immediately after one of the
        /// blocks children have been returned to the free list.
        ///
        /// This is not thread-safe and should only be called while the mutex is held.
        ///
        /// @param in_blockLevel
        ///     The level of the block which should attempt to merge.
        /// @param in_blockIndex
        ///     The index of the block which should attempt to merge.
        ///
        void tryMergeBlock(std::size_t in_blockLevel, std::size_t in_blockIndex) noexcept;

        const std::size_t m_bufferSize;
        const std::size_t m_minBlockSize;
        const std::size_t m_numBlockLevels;
        const std::size_t m_headerSize;

        std::unique_ptr<std::uint8_t[]> m_buffer;
        FreeListTable m_freeListTable;
        AllocatedTable m_allocatedTable;
        SplitTable m_splitTable;

        std::mutex m_mutex;
    };

    //-----------------------------------------------------------------------------
    template <typename TType, typename... TConstructorArgs> UniquePtr<TType> BuddyAllocator::allocate(TConstructorArgs&&... in_constructorArgs) noexcept
    {
        void* memory = allocate(sizeof(TType));
        TType* object = new (memory) TType(std::forward<TConstructorArgs>(in_constructorArgs)...);
        return UniquePtr<TType>(object, [=](TType* in_object) -> void
        {
            in_object->~TType();

            deallocate(reinterpret_cast<void*>(in_object));
        });
    }
}

#endif