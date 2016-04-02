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
        /// @param bufferSize
        ///     The buffer size. Must be a power of two.
        /// @param minBlockSize
        ///     The minimum block size. Must be a power of two.
        ///
        /// @return The number of levels required.
        ///
        inline std::size_t CalcNumLevels(std::size_t bufferSize, std::size_t minBlockSize) noexcept
        {
            return MemoryUtils::CalcShift(bufferSize / minBlockSize) + 1;
        }

        /// Calculates the free list table size based on the number of levels.
        ///
        /// @param numBlockLevels
        ///     The number of levels.
        ///
        /// @return The free list table size.
        ///
        constexpr std::size_t CalcFreeListTableSize(std::size_t numBlockLevels) noexcept
        {
            return numBlockLevels * sizeof(std::uintptr_t);
        }

        /// Calculates the size of the split or allocated tables in bits based on the number
        /// of levels.
        ///
        /// @param numBlockLevels
        ///     The number of levels.
        ///
        /// @return The split or allocated table size in bits.
        ///
        constexpr std::size_t CalcBlockDataTableSizeBits(std::size_t numBlockLevels) noexcept
        {
            return (1 << (numBlockLevels - 1)) - 1;
        }

        /// Calculates the size of the split or allocated tables in bytes based on the number
        /// of levels.
        ///
        /// @param numBlockLevels
        ///     The number of levels.
        ///
        /// @return The split or allocated table size in bytes.
        ///
        constexpr std::size_t CalcBlockDataTableSize(std::size_t numBlockLevels) noexcept
        {
            return (CalcBlockDataTableSizeBits(numBlockLevels) + CHAR_BIT - 1) / CHAR_BIT;
        }

        /// Calculates the size of the split or allocated tables in bytes, aligned to the size
        /// of a pointer, based on the number of levels.
        ///
        /// @param numBlockLevels
        ///     The number of levels.
        ///
        /// @return The aligned split or allocated table size in bytes.
        ///
        inline std::size_t CalcBlockDataTableSizeAligned(std::size_t numBlockLevels) noexcept
        {
            return MemoryUtils::Align(CalcBlockDataTableSize(numBlockLevels), sizeof(std::uintptr_t));
        }

        /// Calculates the total size of the header, based on the number of levels.
        ///
        /// @param numBlockLevels
        ///     The number of levels.
        ///
        /// @return The total header size.
        ///
        inline std::size_t CalcHeaderSize(std::size_t numBlockLevels) noexcept
        {
            return CalcFreeListTableSize(numBlockLevels) + 2 * CalcBlockDataTableSizeAligned(numBlockLevels);
        }

        /// @param blockLevel
        ///     The level requested.
        ///
        /// @return The number of indices in the requested level.
        ///
        constexpr std::size_t GetNumIndicesForLevel(std::size_t blockLevel) noexcept
        {
            return (1 << blockLevel);
        }
    }

    //------------------------------------------------------------------------------
    BuddyAllocator::BuddyAllocator(std::size_t bufferSize, std::size_t minBlockSize) noexcept
        : m_bufferSize(bufferSize),
        m_minBlockSize(minBlockSize),
        m_numBlockLevels(CalcNumLevels(m_bufferSize, m_minBlockSize)),
        m_headerSize(CalcHeaderSize(m_numBlockLevels))
    {
        assert(MemoryUtils::IsPowerOfTwo(m_bufferSize));
        assert(MemoryUtils::IsPowerOfTwo(m_minBlockSize));
        assert(m_minBlockSize > sizeof(std::uintptr_t) * 2);
        assert(m_numBlockLevels > 1);
        assert(m_headerSize < m_bufferSize);

        m_buffer = std::unique_ptr<std::uint8_t[]>(new std::uint8_t[m_bufferSize]);

        InitFreeListTable();
        InitAllocatedTable();
        InitSplitTable();
    }

    //------------------------------------------------------------------------------
    void* BuddyAllocator::Allocate(std::size_t allocationSize) noexcept
    {
        auto blockSize = MemoryUtils::Align(MemoryUtils::NextPowerofTwo(allocationSize), m_minBlockSize);
        auto level = GetLevel(blockSize);
        assert(level != 0);

        std::unique_lock<std::mutex> lock(m_mutex);

        auto block = m_freeListTable.GetStart(level);
        if (!block)
        {
            SplitBlock(level - 1);

            block = m_freeListTable.GetStart(level);
            assert(block);
        }

        m_freeListTable.Remove(level, block);

        auto blockIndex = GetBlockIndex(level, block);
        m_allocatedTable.ToggleAllocatedFlag(level, blockIndex);

        return block;
    }

    //------------------------------------------------------------------------------
    void BuddyAllocator::Deallocate(void* blockPointer) noexcept
    {
        assert(blockPointer >= m_buffer.get());
        assert(MemoryUtils::GetPointerOffset(blockPointer, m_buffer.get()) < m_bufferSize);

        std::unique_lock<std::mutex> lock(m_mutex);

        std::size_t level, index;
        GetAllocatedBlockInfo(blockPointer, level, index);
        assert(level > 0 && level < m_numBlockLevels);

        m_allocatedTable.ToggleAllocatedFlag(level, index);
        m_freeListTable.Add(level, blockPointer);

        std::size_t parentLevel = level - 1;
        std::size_t parentIndex = GetParentBlockIndex(level, index);
        TryMergeBlock(parentLevel, parentIndex);
    }

    //------------------------------------------------------------------------------
    void BuddyAllocator::InitFreeListTable() noexcept
    {
        m_freeListTable = FreeListTable(m_numBlockLevels, m_buffer.get());

        auto relativeBufferBodyStart = static_cast<std::uintptr_t>(MemoryUtils::Align(m_headerSize, m_minBlockSize));
        for (std::size_t level = 0; level < m_numBlockLevels; ++level)
        {
            auto relativeFirstFreeBlock = MemoryUtils::Align(relativeBufferBodyStart, GetBlockSize(level));
            if (relativeFirstFreeBlock < m_bufferSize)
            {
                void* firstFreeBlock = m_buffer.get() + relativeFirstFreeBlock;

                if (GetBlockIndex(level, firstFreeBlock) % 2 == 1)
                {
                    m_freeListTable.Add(level, firstFreeBlock);
                }
            }
        }
    }

    //------------------------------------------------------------------------------
    void BuddyAllocator::InitAllocatedTable() noexcept
    {
        m_allocatedTable = AllocatedTable(m_numBlockLevels, m_buffer.get() + CalcFreeListTableSize(m_numBlockLevels));

        for (std::size_t level = 1; level < m_numBlockLevels; ++level)
        {
            auto relativeEndOfAllocated = static_cast<std::uintptr_t>(MemoryUtils::Align(m_headerSize, GetBlockSize(level)));
            
            auto firstFreeIndex = GetNumIndicesForLevel(level);
            if (relativeEndOfAllocated < m_bufferSize)
            {
                auto endOfAllocated = m_buffer.get() + relativeEndOfAllocated;
                firstFreeIndex = GetBlockIndex(level, endOfAllocated);
            }

            for (std::size_t index = 0; index < firstFreeIndex; ++index)
            {
                m_allocatedTable.ToggleAllocatedFlag(level, index);
            }
        }
    }

    //------------------------------------------------------------------------------
    void BuddyAllocator::InitSplitTable() noexcept
    {
        const auto numParentLevels = m_numBlockLevels - 1;

        m_splitTable = SplitTable(numParentLevels, m_buffer.get() + CalcFreeListTableSize(m_numBlockLevels) + CalcBlockDataTableSizeAligned(m_numBlockLevels));

        auto relativeBufferBodyStart = static_cast<std::uintptr_t>(MemoryUtils::Align(m_headerSize, m_minBlockSize));

        for (std::size_t level = 0; level < numParentLevels; ++level)
        {
            auto relativeLastSplitBlock = MemoryUtils::Align(relativeBufferBodyStart, GetBlockSize(level)) - GetBlockSize(level);
            auto lastSplitBlock = m_buffer.get() + relativeLastSplitBlock;
            auto lastSplitBlockIndex = GetBlockIndex(level, lastSplitBlock);

            for (std::size_t index = 0; index <= lastSplitBlockIndex; ++index)
            {
                m_splitTable.SetSplit(level, index, true);
            }
        }
    }

    //------------------------------------------------------------------------------
    std::size_t BuddyAllocator::GetBlockSize(std::size_t blockLevel) const noexcept
    {
        assert(blockLevel >= 0 && blockLevel < m_numBlockLevels);

        auto output = m_bufferSize;
        for (std::size_t level = 0; level < blockLevel; ++level)
        {
            output >>= 1;
        }

        return output;
    }

    //------------------------------------------------------------------------------
    std::size_t BuddyAllocator::GetLevel(std::size_t blockSize) const noexcept
    {
        assert(MemoryUtils::IsPowerOfTwo(blockSize));
        assert(blockSize >= m_minBlockSize);
        assert(blockSize <= GetBlockSize(0));

        return MemoryUtils::CalcShift(m_bufferSize / blockSize);
    }

    //------------------------------------------------------------------------------
    std::size_t BuddyAllocator::GetBlockIndex(std::size_t blockLevel, void* blockPointer) const noexcept
    {
        assert(blockLevel >= 0 && blockLevel < m_numBlockLevels);
        assert(blockPointer >= m_buffer.get());
        assert(MemoryUtils::GetPointerOffset(blockPointer, m_buffer.get()) < m_bufferSize);
        assert(MemoryUtils::IsAligned(MemoryUtils::GetPointerOffset(blockPointer, m_buffer.get()), GetBlockSize(blockLevel)));

        auto pointerDiff = reinterpret_cast<std::uintptr_t>(blockPointer) - reinterpret_cast<std::uintptr_t>(m_buffer.get());
        return static_cast<std::size_t>(pointerDiff) / GetBlockSize(blockLevel);
    }

    //------------------------------------------------------------------------------
    std::size_t BuddyAllocator::GetParentBlockIndex(std::size_t blockLevel, std::size_t blockIndex) const noexcept
    {
        assert(blockLevel > 0 && blockLevel < m_numBlockLevels);
        assert(blockIndex < GetNumIndicesForLevel(blockLevel));

        return blockIndex >> 1;
    }

    //------------------------------------------------------------------------------
    void BuddyAllocator::GetChildBlockIndices(std::size_t parentBlockLevel, std::size_t parentBlockIndex, std::size_t& out_childBlockIndexA, std::size_t& out_childBlockIndexB) const noexcept
    {
        assert(parentBlockLevel >= 0 && parentBlockLevel < m_numBlockLevels - 1);
        assert(parentBlockIndex < GetNumIndicesForLevel(parentBlockLevel));

        out_childBlockIndexA = parentBlockIndex << 1;
        out_childBlockIndexB = out_childBlockIndexA + 1;
    }

    //------------------------------------------------------------------------------
    void* BuddyAllocator::GetBlockPointer(std::size_t blockLevel, std::size_t blockIndex) const noexcept
    {
        assert(blockLevel >= 0 && blockLevel < m_numBlockLevels);
        assert(blockIndex < GetNumIndicesForLevel(blockLevel));

        return reinterpret_cast<void*>(m_buffer.get() + blockIndex * GetBlockSize(blockLevel));
    }

    //------------------------------------------------------------------------------
    void BuddyAllocator::GetAllocatedBlockInfo(void* blockPointer, std::size_t& out_level, std::size_t& out_index) const noexcept
    {
        assert(blockPointer);

        out_level = 0;
        out_index = 0;

        for (std::size_t level = 1; level < m_numBlockLevels; ++level)
        {
            if (MemoryUtils::IsAligned(MemoryUtils::GetPointerOffset(blockPointer, m_buffer.get()), GetBlockSize(level)))
            {
                auto index = GetBlockIndex(level, blockPointer);

                if (level == m_numBlockLevels - 1 || !m_splitTable.IsSplit(level, index))
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
    void BuddyAllocator::SplitBlock(std::size_t blockLevel) noexcept
    {
        assert(blockLevel > 0 && blockLevel < m_numBlockLevels - 1);

        auto blockPointer = m_freeListTable.GetStart(blockLevel);
        if (!blockPointer)
        {
            assert(blockLevel > 1);

            SplitBlock(blockLevel - 1);

            blockPointer = m_freeListTable.GetStart(blockLevel);
            assert(blockPointer);
        }

        auto blockIndex = GetBlockIndex(blockLevel, blockPointer);

        m_freeListTable.Remove(blockLevel, blockPointer);
        m_allocatedTable.ToggleAllocatedFlag(blockLevel, blockIndex);
        m_splitTable.SetSplit(blockLevel, blockIndex, true);
        
        std::size_t childBlockLevel = blockLevel + 1;
        m_freeListTable.Add(childBlockLevel, blockPointer);
        m_freeListTable.Add(childBlockLevel, reinterpret_cast<std::uint8_t*>(blockPointer) + GetBlockSize(childBlockLevel));
    }

    //------------------------------------------------------------------------------
    void BuddyAllocator::TryMergeBlock(std::size_t blockLevel, std::size_t blockIndex) noexcept
    {
        assert(blockLevel >= 0 && blockLevel < m_numBlockLevels - 1);

        std::size_t childBlockLevel = blockLevel + 1;
        std::size_t childBlockIndexA, childBlockIndexB;
        GetChildBlockIndices(blockLevel, blockIndex, childBlockIndexA, childBlockIndexB);

        if (!m_allocatedTable.GetAllocatedFlag(childBlockLevel, childBlockIndexA))
        {
            m_freeListTable.Remove(childBlockLevel, GetBlockPointer(childBlockLevel, childBlockIndexA));
            m_freeListTable.Remove(childBlockLevel, GetBlockPointer(childBlockLevel, childBlockIndexB));

            m_splitTable.SetSplit(blockLevel, blockIndex, false);
            m_allocatedTable.ToggleAllocatedFlag(blockLevel, blockIndex);

            m_freeListTable.Add(blockLevel, GetBlockPointer(blockLevel, blockIndex));

            std::size_t parentLevel = blockLevel - 1;
            if (parentLevel > 0)
            {
                std::size_t parentIndex = GetParentBlockIndex(blockLevel, blockIndex);
                TryMergeBlock(parentLevel, parentIndex);
            }
        }
    }

    //------------------------------------------------------------------------------
    BuddyAllocator::FreeListTable::FreeListTable(std::size_t numBlockLevels, void* buffer) noexcept
        : m_numBlockLevels(numBlockLevels)
    {
        m_freeListTable = reinterpret_cast<ListNode**>(buffer);

        for (std::size_t i = 0; i < m_numBlockLevels; ++i)
        {
            m_freeListTable[i] = nullptr;
        }
    }

    //------------------------------------------------------------------------------
    void* BuddyAllocator::FreeListTable::GetStart(std::size_t tableLevel) const noexcept
    {
        assert(tableLevel >= 0 && tableLevel < m_numBlockLevels);

        return reinterpret_cast<void*>(m_freeListTable[tableLevel]);
    }

    //------------------------------------------------------------------------------
    void* BuddyAllocator::FreeListTable::GetNext(void* listElement) const noexcept
    {
        assert(listElement != nullptr);

        ListNode* listNode = reinterpret_cast<ListNode*>(listElement);
        return reinterpret_cast<void*>(listNode->m_next);
    }

    //------------------------------------------------------------------------------
    void* BuddyAllocator::FreeListTable::GetPrevious(void* listElement) const noexcept
    {
        assert(listElement != nullptr);

        ListNode* listNode = reinterpret_cast<ListNode*>(listElement);
        return reinterpret_cast<void*>(listNode->m_previous);
    }

    //------------------------------------------------------------------------------
    void BuddyAllocator::FreeListTable::Add(std::size_t tableLevel, void* listElement) noexcept
    {
        assert(tableLevel >= 0 && tableLevel < m_numBlockLevels);
        assert(listElement != nullptr);

        ListNode* newStart = reinterpret_cast<ListNode*>(listElement);
        newStart->m_previous = nullptr;
        newStart->m_next = m_freeListTable[tableLevel];

        if (m_freeListTable[tableLevel])
        {
            assert(m_freeListTable[tableLevel]->m_previous == nullptr);
            m_freeListTable[tableLevel]->m_previous = newStart;
        }

        m_freeListTable[tableLevel] = newStart;
    }

    //------------------------------------------------------------------------------
    void BuddyAllocator::FreeListTable::Remove(std::size_t tableLevel, void* listElement) noexcept
    {
        assert(tableLevel >= 0 && tableLevel < m_numBlockLevels);
        assert(listElement != nullptr);

        ListNode* toRemove = reinterpret_cast<ListNode*>(listElement);

        if (toRemove == m_freeListTable[tableLevel])
        {
            m_freeListTable[tableLevel] = toRemove->m_next;
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
    BuddyAllocator::AllocatedTable::AllocatedTable(std::size_t numBlockLevels, void* buffer) noexcept
        : m_numBlockLevels(numBlockLevels), m_allocatedTable(buffer)
    {
        memset(m_allocatedTable, 0, CalcBlockDataTableSizeAligned(numBlockLevels));
    }

    //------------------------------------------------------------------------------
    bool BuddyAllocator::AllocatedTable::GetAllocatedFlag(std::size_t blockLevel, std::size_t blockIndex) const noexcept
    {
        assert(blockLevel > 0 && blockLevel < m_numBlockLevels);
        assert(blockIndex < GetNumIndicesForLevel(blockLevel));

        auto tableLevel = blockLevel - 1;
        auto tableIndex = blockIndex >> 1;

        auto flagIndex = (1 << tableLevel) - 1 + tableIndex;
        auto flagByteIndex = flagIndex / CHAR_BIT;
        auto flagBitIndex = flagIndex % CHAR_BIT;

        return (reinterpret_cast<std::uint8_t*>(m_allocatedTable)[flagByteIndex] & (1 << flagBitIndex)) != 0;
    }

    //------------------------------------------------------------------------------
    void BuddyAllocator::AllocatedTable::ToggleAllocatedFlag(std::size_t blockLevel, std::size_t blockIndex) noexcept
    {
        assert(blockLevel > 0 && blockLevel < m_numBlockLevels);
        assert(blockIndex < GetNumIndicesForLevel(blockLevel));

        auto tableLevel = blockLevel - 1;
        auto tableIndex = blockIndex >> 1;

        auto flagIndex = (1 << tableLevel) - 1 + tableIndex;
        auto flagByteIndex = flagIndex / CHAR_BIT;
        auto flagBitIndex = flagIndex % CHAR_BIT;

        reinterpret_cast<std::uint8_t*>(m_allocatedTable)[flagByteIndex] ^= (1 << flagBitIndex);
    }

    //------------------------------------------------------------------------------
    BuddyAllocator::SplitTable::SplitTable(std::size_t numBlockLevels, void* buffer) noexcept
        : m_numBlockLevels(numBlockLevels), m_splitTable(buffer)
    {
        memset(m_splitTable, 0, CalcBlockDataTableSizeAligned(numBlockLevels));
    }

    //------------------------------------------------------------------------------
    bool BuddyAllocator::SplitTable::IsSplit(std::size_t blockLevel, std::size_t blockIndex) const noexcept
    {
        assert(blockLevel >= 0 && blockLevel < m_numBlockLevels);
        assert(blockIndex < GetNumIndicesForLevel(blockLevel));

        auto flagIndex = (1 << blockLevel) - 1 + blockIndex;
        auto flagByteIndex = flagIndex / CHAR_BIT;
        auto flagBitIndex = flagIndex % CHAR_BIT;

        return (reinterpret_cast<std::uint8_t*>(m_splitTable)[flagByteIndex] & (1 << flagBitIndex)) != 0;
    }

    //------------------------------------------------------------------------------
    void BuddyAllocator::SplitTable::SetSplit(std::size_t blockLevel, std::size_t blockIndex, bool isSplit) noexcept
    {
        assert(blockLevel >= 0 && blockLevel < m_numBlockLevels);
        assert(blockIndex < GetNumIndicesForLevel(blockLevel));

        auto bufferBlockIndex = (1 << blockLevel) - 1 + blockIndex;
        auto bufferByteIndex = bufferBlockIndex / CHAR_BIT;
        auto bufferBitIndex = bufferBlockIndex % CHAR_BIT;

        if (isSplit)
        {
            reinterpret_cast<std::uint8_t*>(m_splitTable)[bufferByteIndex] |= (1 << bufferBitIndex);
        }
        else
        {
            reinterpret_cast<std::uint8_t*>(m_splitTable)[bufferByteIndex] &= ~(1 << bufferBitIndex);
        }
    }
}