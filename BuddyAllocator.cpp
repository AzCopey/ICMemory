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

#include "BuddyAllocator.h"

#include "MemoryUtils.h"

#include <cassert>

namespace IC
{
    namespace
    {
        /// Calculates the number of levels required for the given buffer size and
        /// min block size.
        ///
        /// @param in_bufferSize
        ///     The buffer size. Must be a power of two.
        /// @param in_minBlockSize
        ///     The minimum block size. Must be a power of two.
        ///
        /// @return The number of levels required.
        ///
        inline std::size_t calcNumLevels(std::size_t in_bufferSize, std::size_t in_minBlockSize) noexcept
        {
            return MemoryUtils::calcShift(in_bufferSize / in_minBlockSize) + 1;
        }

        /// Calculates the free list table size based on the number of levels.
        ///
        /// @param in_numBlockLevels
        ///     The number of levels.
        ///
        /// @return The free list table size.
        ///
        constexpr std::size_t calcFreeListTableSize(std::size_t in_numBlockLevels) noexcept
        {
            return in_numBlockLevels * sizeof(std::uintptr_t);
        }

        /// Calculates the size of the split or allocated tables in bits based on the number
        /// of levels.
        ///
        /// @param in_numBlockLevels
        ///     The number of levels.
        ///
        /// @return The split or allocated table size in bits.
        ///
        constexpr std::size_t calcBlockDataTableSizeBits(std::size_t in_numBlockLevels) noexcept
        {
            return (1 << (in_numBlockLevels - 1)) - 1;
        }

        /// Calculates the size of the split or allocated tables in bytes based on the number
        /// of levels.
        ///
        /// @param in_numBlockLevels
        ///     The number of levels.
        ///
        /// @return The split or allocated table size in bytes.
        ///
        constexpr std::size_t calcBlockDataTableSize(std::size_t in_numBlockLevels) noexcept
        {
            return (calcBlockDataTableSizeBits(in_numBlockLevels) + CHAR_BIT - 1) / CHAR_BIT;
        }

        /// Calculates the size of the split or allocated tables in bytes, aligned to the size
        /// of a pointer, based on the number of levels.
        ///
        /// @param in_numBlockLevels
        ///     The number of levels.
        ///
        /// @return The aligned split or allocated table size in bytes.
        ///
        inline std::size_t calcBlockDataTableSizeAligned(std::size_t in_numBlockLevels) noexcept
        {
            return MemoryUtils::align(calcBlockDataTableSize(in_numBlockLevels), sizeof(std::uintptr_t));
        }

        /// Calculates the total size of the header, based on the number of levels.
        ///
        /// @param in_numBlockLevels
        ///     The number of levels.
        ///
        /// @return The total header size.
        ///
        inline std::size_t calcHeaderSize(std::size_t in_numBlockLevels) noexcept
        {
            return calcFreeListTableSize(in_numBlockLevels) + 2 * calcBlockDataTableSizeAligned(in_numBlockLevels);
        }

        /// @param in_blockLevel
        ///     The level requested.
        ///
        /// @return The number of indices in the requested level.
        ///
        constexpr std::size_t getNumIndicesForLevel(std::size_t in_blockLevel) noexcept
        {
            return (1 << in_blockLevel);
        }
    }

    //------------------------------------------------------------------------------
    BuddyAllocator::BuddyAllocator(std::size_t in_bufferSize, std::size_t in_minBlockSize) noexcept
        : m_bufferSize(in_bufferSize),
        m_minBlockSize(in_minBlockSize),
        m_numBlockLevels(calcNumLevels(m_bufferSize, m_minBlockSize)),
        m_headerSize(calcHeaderSize(m_numBlockLevels))
    {
        assert(MemoryUtils::isPowerOfTwo(m_bufferSize));
        assert(MemoryUtils::isPowerOfTwo(m_minBlockSize));
        assert(m_minBlockSize > sizeof(std::uintptr_t) * 2);
        assert(m_numBlockLevels > 1);
        assert(m_headerSize < m_bufferSize);

        m_buffer = std::unique_ptr<std::uint8_t[]>(new std::uint8_t[m_bufferSize]);

        initFreeListTable();
        initAllocatedTable();
        initSplitTable();
    }

    //------------------------------------------------------------------------------
    void* BuddyAllocator::allocate(std::size_t in_allocationSize) noexcept
    {
        auto blockSize = MemoryUtils::align(MemoryUtils::nextPowerofTwo(in_allocationSize), m_minBlockSize);
        auto level = getLevel(blockSize);
        assert(level != 0);

        std::unique_lock<std::mutex> lock(m_mutex);

        auto block = m_freeListTable.getStart(level);
        if (!block)
        {
            splitBlock(level - 1);

            block = m_freeListTable.getStart(level);
            assert(block);
        }

        m_freeListTable.remove(level, block);

        auto blockIndex = getBlockIndex(level, block);
        m_allocatedTable.toggleAllocatedFlag(level, blockIndex);

        return block;
    }

    //------------------------------------------------------------------------------
    void BuddyAllocator::deallocate(void* in_blockPointer) noexcept
    {
        assert(in_blockPointer >= m_buffer.get());
        assert(MemoryUtils::getPointerOffset(in_blockPointer, m_buffer.get()) < m_bufferSize);

        std::unique_lock<std::mutex> lock(m_mutex);

        std::size_t level, index;
        getAllocatedBlockInfo(in_blockPointer, level, index);
        assert(level > 0 && level < m_numBlockLevels);

        m_allocatedTable.toggleAllocatedFlag(level, index);
        m_freeListTable.add(level, in_blockPointer);

        std::size_t parentLevel = level - 1;
        std::size_t parentIndex = getParentBlockIndex(level, index);
        tryMergeBlock(parentLevel, parentIndex);
    }

    //------------------------------------------------------------------------------
    void BuddyAllocator::initFreeListTable() noexcept
    {
        m_freeListTable = FreeListTable(m_numBlockLevels, m_buffer.get());

        auto relativeBufferBodyStart = static_cast<std::uintptr_t>(MemoryUtils::align(m_headerSize, m_minBlockSize));
        for (std::size_t level = 0; level < m_numBlockLevels; ++level)
        {
            auto relativeFirstFreeBlock = MemoryUtils::align(relativeBufferBodyStart, getBlockSize(level));
            if (relativeFirstFreeBlock < m_bufferSize)
            {
                void* firstFreeBlock = m_buffer.get() + relativeFirstFreeBlock;

                if (getBlockIndex(level, firstFreeBlock) % 2 == 1)
                {
                    m_freeListTable.add(level, firstFreeBlock);
                }
            }
        }
    }

    //------------------------------------------------------------------------------
    void BuddyAllocator::initAllocatedTable() noexcept
    {
        m_allocatedTable = AllocatedTable(m_numBlockLevels, m_buffer.get() + calcFreeListTableSize(m_numBlockLevels));

        for (std::size_t level = 1; level < m_numBlockLevels; ++level)
        {
            auto relativeEndOfAllocated = static_cast<std::uintptr_t>(MemoryUtils::align(m_headerSize, getBlockSize(level)));
            
            auto firstFreeIndex = getNumIndicesForLevel(level);
            if (relativeEndOfAllocated < m_bufferSize)
            {
                auto endOfAllocated = m_buffer.get() + relativeEndOfAllocated;
                firstFreeIndex = getBlockIndex(level, endOfAllocated);
            }

            for (std::size_t index = 0; index < firstFreeIndex; ++index)
            {
                m_allocatedTable.toggleAllocatedFlag(level, index);
            }
        }
    }

    //------------------------------------------------------------------------------
    void BuddyAllocator::initSplitTable() noexcept
    {
        const auto numParentLevels = m_numBlockLevels - 1;

        m_splitTable = SplitTable(numParentLevels, m_buffer.get() + calcFreeListTableSize(m_numBlockLevels) + calcBlockDataTableSizeAligned(m_numBlockLevels));

        auto relativeBufferBodyStart = static_cast<std::uintptr_t>(MemoryUtils::align(m_headerSize, m_minBlockSize));

        for (std::size_t level = 0; level < numParentLevels; ++level)
        {
            auto relativeLastSplitBlock = MemoryUtils::align(relativeBufferBodyStart, getBlockSize(level)) - getBlockSize(level);
            auto lastSplitBlock = m_buffer.get() + relativeLastSplitBlock;
            auto lastSplitBlockIndex = getBlockIndex(level, lastSplitBlock);

            for (std::size_t index = 0; index <= lastSplitBlockIndex; ++index)
            {
                m_splitTable.setSplit(level, index, true);
            }
        }
    }

    //------------------------------------------------------------------------------
    std::size_t BuddyAllocator::getBlockSize(std::size_t in_blockLevel) const noexcept
    {
        assert(in_blockLevel >= 0 && in_blockLevel < m_numBlockLevels);

        auto output = m_bufferSize;
        for (std::size_t level = 0; level < in_blockLevel; ++level)
        {
            output >>= 1;
        }

        return output;
    }

    //------------------------------------------------------------------------------
    std::size_t BuddyAllocator::getLevel(std::size_t in_blockSize) const noexcept
    {
        assert(MemoryUtils::isPowerOfTwo(in_blockSize));
        assert(in_blockSize >= m_minBlockSize);
        assert(in_blockSize <= getBlockSize(0));

        return MemoryUtils::calcShift(m_bufferSize / in_blockSize);
    }

    //------------------------------------------------------------------------------
    std::size_t BuddyAllocator::getBlockIndex(std::size_t in_blockLevel, void* in_blockPointer) const noexcept
    {
        assert(in_blockLevel >= 0 && in_blockLevel < m_numBlockLevels);
        assert(in_blockPointer >= m_buffer.get());
        assert(MemoryUtils::getPointerOffset(in_blockPointer, m_buffer.get()) < m_bufferSize);
        assert(MemoryUtils::isAligned(MemoryUtils::getPointerOffset(in_blockPointer, m_buffer.get()), getBlockSize(in_blockLevel)));

        auto pointerDiff = reinterpret_cast<std::uintptr_t>(in_blockPointer) - reinterpret_cast<std::uintptr_t>(m_buffer.get());
        return static_cast<std::size_t>(pointerDiff) / getBlockSize(in_blockLevel);
    }

    //------------------------------------------------------------------------------
    std::size_t BuddyAllocator::getParentBlockIndex(std::size_t in_blockLevel, std::size_t in_blockIndex) const noexcept
    {
        assert(in_blockLevel > 0 && in_blockLevel < m_numBlockLevels);
        assert(in_blockIndex < getNumIndicesForLevel(in_blockLevel));

        return in_blockIndex >> 1;
    }

    //------------------------------------------------------------------------------
    void BuddyAllocator::getChildBlockIndices(std::size_t in_parentBlockLevel, std::size_t in_parentBlockIndex, std::size_t& out_childBlockIndexA, std::size_t& out_childBlockIndexB) const noexcept
    {
        assert(in_parentBlockLevel >= 0 && in_parentBlockLevel < m_numBlockLevels - 1);
        assert(in_parentBlockIndex < getNumIndicesForLevel(in_parentBlockLevel));

        out_childBlockIndexA = in_parentBlockIndex << 1;
        out_childBlockIndexB = out_childBlockIndexA + 1;
    }

    //------------------------------------------------------------------------------
    void* BuddyAllocator::getBlockPointer(std::size_t in_blockLevel, std::size_t in_blockIndex) const noexcept
    {
        assert(in_blockLevel >= 0 && in_blockLevel < m_numBlockLevels);
        assert(in_blockIndex < getNumIndicesForLevel(in_blockLevel));

        return reinterpret_cast<void*>(m_buffer.get() + in_blockIndex * getBlockSize(in_blockLevel));
    }

    //------------------------------------------------------------------------------
    void BuddyAllocator::getAllocatedBlockInfo(void* in_blockPointer, std::size_t& out_level, std::size_t& out_index) const noexcept
    {
        assert(in_blockPointer);

        out_level = 0;
        out_index = 0;

        for (std::size_t level = 1; level < m_numBlockLevels; ++level)
        {
            if (MemoryUtils::isAligned(MemoryUtils::getPointerOffset(in_blockPointer, m_buffer.get()), getBlockSize(level)))
            {
                auto index = getBlockIndex(level, in_blockPointer);

                if (level == m_numBlockLevels - 1 || !m_splitTable.isSplit(level, index))
                {
                    out_level = level;
                    out_index = index;
                    break;
                }
            }
        }

        assert(out_level != 0);
    }

    //------------------------------------------------------------------------------
    void BuddyAllocator::splitBlock(std::size_t in_blockLevel) noexcept
    {
        assert(in_blockLevel > 0 && in_blockLevel < m_numBlockLevels - 1);

        auto blockPointer = m_freeListTable.getStart(in_blockLevel);
        if (!blockPointer)
        {
            assert(in_blockLevel > 1);

            splitBlock(in_blockLevel - 1);

            blockPointer = m_freeListTable.getStart(in_blockLevel);
            assert(blockPointer);
        }

        auto blockIndex = getBlockIndex(in_blockLevel, blockPointer);

        m_freeListTable.remove(in_blockLevel, blockPointer);
        m_allocatedTable.toggleAllocatedFlag(in_blockLevel, blockIndex);
        m_splitTable.setSplit(in_blockLevel, blockIndex, true);
        
        std::size_t childBlockLevel = in_blockLevel + 1;
        m_freeListTable.add(childBlockLevel, blockPointer);
        m_freeListTable.add(childBlockLevel, reinterpret_cast<std::uint8_t*>(blockPointer) + getBlockSize(childBlockLevel));
    }

    //------------------------------------------------------------------------------
    void BuddyAllocator::tryMergeBlock(std::size_t in_blockLevel, std::size_t in_blockIndex) noexcept
    {
        assert(in_blockLevel >= 0 && in_blockLevel < m_numBlockLevels - 1);

        std::size_t childBlockLevel = in_blockLevel + 1;
        std::size_t childBlockIndexA, childBlockIndexB;
        getChildBlockIndices(in_blockLevel, in_blockIndex, childBlockIndexA, childBlockIndexB);

        if (!m_allocatedTable.getAllocatedFlag(childBlockLevel, childBlockIndexA))
        {
            m_freeListTable.remove(childBlockLevel, getBlockPointer(childBlockLevel, childBlockIndexA));
            m_freeListTable.remove(childBlockLevel, getBlockPointer(childBlockLevel, childBlockIndexB));

            m_splitTable.setSplit(in_blockLevel, in_blockIndex, false);
            m_allocatedTable.toggleAllocatedFlag(in_blockLevel, in_blockIndex);

            m_freeListTable.add(in_blockLevel, getBlockPointer(in_blockLevel, in_blockIndex));

            std::size_t parentLevel = in_blockLevel - 1;
            if (parentLevel > 0)
            {
                std::size_t parentIndex = getParentBlockIndex(in_blockLevel, in_blockIndex);
                tryMergeBlock(parentLevel, parentIndex);
            }
        }
    }

    //------------------------------------------------------------------------------
    BuddyAllocator::FreeListTable::FreeListTable(std::size_t in_numBlockLevels, void* in_buffer) noexcept
        : m_numBlockLevels(in_numBlockLevels)
    {
        m_freeListTable = reinterpret_cast<ListNode**>(in_buffer);

        for (std::size_t i = 0; i < m_numBlockLevels; ++i)
        {
            m_freeListTable[i] = nullptr;
        }
    }

    //------------------------------------------------------------------------------
    void* BuddyAllocator::FreeListTable::getStart(std::size_t in_tableLevel) const noexcept
    {
        assert(in_tableLevel >= 0 && in_tableLevel < m_numBlockLevels);

        return reinterpret_cast<void*>(m_freeListTable[in_tableLevel]);
    }

    //------------------------------------------------------------------------------
    void* BuddyAllocator::FreeListTable::getNext(void* in_listElement) const noexcept
    {
        assert(in_listElement != nullptr);

        ListNode* listNode = reinterpret_cast<ListNode*>(in_listElement);
        return reinterpret_cast<void*>(listNode->m_next);
    }

    //------------------------------------------------------------------------------
    void* BuddyAllocator::FreeListTable::getPrevious(void* in_listElement) const noexcept
    {
        assert(in_listElement != nullptr);

        ListNode* listNode = reinterpret_cast<ListNode*>(in_listElement);
        return reinterpret_cast<void*>(listNode->m_previous);
    }

    //------------------------------------------------------------------------------
    void BuddyAllocator::FreeListTable::add(std::size_t in_tableLevel, void* in_listElement) noexcept
    {
        assert(in_tableLevel >= 0 && in_tableLevel < m_numBlockLevels);
        assert(in_listElement != nullptr);

        ListNode* newStart = reinterpret_cast<ListNode*>(in_listElement);
        newStart->m_previous = nullptr;
        newStart->m_next = m_freeListTable[in_tableLevel];

        if (m_freeListTable[in_tableLevel])
        {
            assert(m_freeListTable[in_tableLevel]->m_previous == nullptr);
            m_freeListTable[in_tableLevel]->m_previous = newStart;
        }

        m_freeListTable[in_tableLevel] = newStart;
    }

    //------------------------------------------------------------------------------
    void BuddyAllocator::FreeListTable::remove(std::size_t in_tableLevel, void* in_listElement) noexcept
    {
        assert(in_tableLevel >= 0 && in_tableLevel < m_numBlockLevels);
        assert(in_listElement != nullptr);

        ListNode* toRemove = reinterpret_cast<ListNode*>(in_listElement);

        if (toRemove == m_freeListTable[in_tableLevel])
        {
            m_freeListTable[in_tableLevel] = toRemove->m_next;
        }

        if (toRemove->m_next)
        {
            toRemove->m_next->m_previous = toRemove->m_previous;
        }

        if (toRemove->m_previous)
        {
            toRemove->m_previous->m_next = toRemove->m_next;
        }
    }

    //------------------------------------------------------------------------------
    BuddyAllocator::AllocatedTable::AllocatedTable(std::size_t in_numBlockLevels, void* in_buffer) noexcept
        : m_numBlockLevels(in_numBlockLevels), m_allocatedTable(in_buffer)
    {
        memset(m_allocatedTable, 0, calcBlockDataTableSizeAligned(in_numBlockLevels));
    }

    //------------------------------------------------------------------------------
    bool BuddyAllocator::AllocatedTable::getAllocatedFlag(std::size_t in_blockLevel, std::size_t in_blockIndex) const noexcept
    {
        assert(in_blockLevel > 0 && in_blockLevel < m_numBlockLevels);
        assert(in_blockIndex < getNumIndicesForLevel(in_blockLevel));

        auto tableLevel = in_blockLevel - 1;
        auto tableIndex = in_blockIndex >> 1;

        auto flagIndex = (1 << tableLevel) - 1 + tableIndex;
        auto flagByteIndex = flagIndex / CHAR_BIT;
        auto flagBitIndex = flagIndex % CHAR_BIT;

        return (reinterpret_cast<std::uint8_t*>(m_allocatedTable)[flagByteIndex] & (1 << flagBitIndex)) != 0;
    }

    //------------------------------------------------------------------------------
    void BuddyAllocator::AllocatedTable::toggleAllocatedFlag(std::size_t in_blockLevel, std::size_t in_blockIndex) noexcept
    {
        assert(in_blockLevel > 0 && in_blockLevel < m_numBlockLevels);
        assert(in_blockIndex < getNumIndicesForLevel(in_blockLevel));

        auto tableLevel = in_blockLevel - 1;
        auto tableIndex = in_blockIndex >> 1;

        auto flagIndex = (1 << tableLevel) - 1 + tableIndex;
        auto flagByteIndex = flagIndex / CHAR_BIT;
        auto flagBitIndex = flagIndex % CHAR_BIT;

        reinterpret_cast<std::uint8_t*>(m_allocatedTable)[flagByteIndex] ^= (1 << flagBitIndex);
    }

    //------------------------------------------------------------------------------
    BuddyAllocator::SplitTable::SplitTable(std::size_t in_numBlockLevels, void* in_buffer) noexcept
        : m_numBlockLevels(in_numBlockLevels), m_splitTable(in_buffer)
    {
        memset(m_splitTable, 0, calcBlockDataTableSizeAligned(in_numBlockLevels));
    }

    //------------------------------------------------------------------------------
    bool BuddyAllocator::SplitTable::isSplit(std::size_t in_blockLevel, std::size_t in_blockIndex) const noexcept
    {
        assert(in_blockLevel >= 0 && in_blockLevel < m_numBlockLevels);
        assert(in_blockIndex < getNumIndicesForLevel(in_blockLevel));

        auto flagIndex = (1 << in_blockLevel) - 1 + in_blockIndex;
        auto flagByteIndex = flagIndex / CHAR_BIT;
        auto flagBitIndex = flagIndex % CHAR_BIT;

        return (reinterpret_cast<std::uint8_t*>(m_splitTable)[flagByteIndex] & (1 << flagBitIndex)) != 0;
    }

    //------------------------------------------------------------------------------
    void BuddyAllocator::SplitTable::setSplit(std::size_t in_blockLevel, std::size_t in_blockIndex, bool in_isSplit) noexcept
    {
        assert(in_blockLevel >= 0 && in_blockLevel < m_numBlockLevels);
        assert(in_blockIndex < getNumIndicesForLevel(in_blockLevel));

        auto bufferBlockIndex = (1 << in_blockLevel) - 1 + in_blockIndex;
        auto bufferByteIndex = bufferBlockIndex / CHAR_BIT;
        auto bufferBitIndex = bufferBlockIndex % CHAR_BIT;

        if (in_isSplit)
        {
            reinterpret_cast<std::uint8_t*>(m_splitTable)[bufferByteIndex] |= (1 << bufferBitIndex);
        }
        else
        {
            reinterpret_cast<std::uint8_t*>(m_splitTable)[bufferByteIndex] &= ~(1 << bufferBitIndex);
        }
    }
}