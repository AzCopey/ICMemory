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
        /// @param in_numLevels
        ///     The number of levels.
        ///
        /// @return The free list table size.
        ///
        constexpr std::size_t calcFreeListTableSize(std::size_t in_numLevels) noexcept
        {
            return in_numLevels * sizeof(std::uintptr_t);
        }

        /// Calculates the size of the split or allocated tables in bits based on the number
        /// of levels.
        ///
        /// @param in_numLevels
        ///     The number of levels.
        ///
        /// @return The split or allocated table size in bits.
        ///
        constexpr std::size_t calcBlockDataTableSizeBits(std::size_t in_numLevels) noexcept
        {
            return (1 << (in_numLevels - 1)) - 1;
        }

        /// Calculates the size of the split or allocated tables in bytes based on the number
        /// of levels.
        ///
        /// @param in_numLevels
        ///     The number of levels.
        ///
        /// @return The split or allocated table size in bytes.
        ///
        constexpr std::size_t calcBlockDataTableSize(std::size_t in_numLevels) noexcept
        {
            return (calcBlockDataTableSizeBits(in_numLevels) + CHAR_BIT - 1) / CHAR_BIT;
        }

        /// Calculates the size of the split or allocated tables in bytes, aligned to the size
        /// of a pointer, based on the number of levels.
        ///
        /// @param in_numLevels
        ///     The number of levels.
        ///
        /// @return The aligned split or allocated table size in bytes.
        ///
        inline std::size_t calcBlockDataTableSizeAligned(std::size_t in_numLevels) noexcept
        {
            return MemoryUtils::align(calcBlockDataTableSize(in_numLevels), sizeof(std::uintptr_t));
        }

        /// Calculates the total size of the header, based on the number of levels.
        ///
        /// @param in_numLevels
        ///     The number of levels.
        ///
        /// @return The total header size.
        ///
        inline std::size_t calcHeaderSize(std::size_t in_numLevels) noexcept
        {
            return calcFreeListTableSize(in_numLevels) + (calcBlockDataTableSizeAligned(in_numLevels) * 2);
        }

        /// @param in_level
        ///     The level requested.
        ///
        /// @return The number of indices in the requested level.
        ///
        constexpr std::size_t getNumIndicesForLevel(std::size_t in_level) noexcept
        {
            return (1 << in_level);
        }
    }

    //------------------------------------------------------------------------------
    BuddyAllocator::BuddyAllocator(std::size_t in_bufferSize, std::size_t in_minBlockSize) noexcept
        : m_bufferSize(in_bufferSize),
        m_minBlockSize(in_minBlockSize),
        m_numLevels(calcNumLevels(m_bufferSize, m_minBlockSize)),
        m_freeListTableSize(calcFreeListTableSize(m_numLevels)),
        m_blockDataTableSize(calcBlockDataTableSize(m_numLevels)),
        m_blockDataTableSizeAligned(calcBlockDataTableSizeAligned(m_numLevels)),
        m_blockDataTableSizeBits(calcBlockDataTableSizeBits(m_numLevels)),
        m_headerSize(calcHeaderSize(m_numLevels))
    {
        assert(MemoryUtils::isPowerOfTwo(m_bufferSize));
        assert(MemoryUtils::isPowerOfTwo(m_minBlockSize));
        assert(m_minBlockSize > sizeof(std::uintptr_t) * 2);
        assert(m_numLevels > 1);
        assert(m_headerSize < m_bufferSize);

        m_buffer = std::unique_ptr<std::uint8_t[]>(new std::uint8_t[m_bufferSize]);

        initFreeListTable();
        initAllocatedTable();
        initSplitTable();
    }

    //------------------------------------------------------------------------------
    void BuddyAllocator::initFreeListTable() noexcept
    {
        m_freeListTable = reinterpret_cast<void**>(m_buffer.get());
        memset(m_freeListTable, 0, m_freeListTableSize);

        auto relativeBufferBodyStart = static_cast<std::uintptr_t>(MemoryUtils::align(m_headerSize, m_minBlockSize));
        for (std::size_t level = 0; level < m_numLevels; ++level)
        {
            auto relativeFirstFreeBlock = MemoryUtils::align(relativeBufferBodyStart, getBlockSize(level));
            if (relativeFirstFreeBlock < m_bufferSize)
            {
                void* firstFreeBlock = m_buffer.get() + relativeFirstFreeBlock;
                setFreeListStart(level, firstFreeBlock);
            }
        }
    }

    //------------------------------------------------------------------------------
    void BuddyAllocator::initAllocatedTable() noexcept
    {
        m_allocatedTable = m_buffer.get() + m_freeListTableSize;
        memset(m_allocatedTable, 0, m_blockDataTableSize);

        auto relativeBufferBodyStart = static_cast<std::uintptr_t>(MemoryUtils::align(m_headerSize, m_minBlockSize));
        auto bufferBodyStart = m_buffer.get() + relativeBufferBodyStart;
        auto bottomLevel = m_numLevels - 1;
        auto secondBottomLevel = bottomLevel - 1;
        auto bodyStartIndex = getBlockIndex(bottomLevel, bufferBodyStart);

        for (std::size_t index = 0; index < bodyStartIndex; ++index)
        {
            std::size_t parentIndex = getParentBlockIndex(bottomLevel, index);
            toggleChildrenAllocated(secondBottomLevel, parentIndex);
        }
    }

    //------------------------------------------------------------------------------
    void BuddyAllocator::initSplitTable() noexcept
    {
        m_splitTable = m_buffer.get() + m_freeListTableSize + m_blockDataTableSizeAligned;
        memset(m_splitTable, 0, m_blockDataTableSize);

        auto relativeBufferBodyStart = static_cast<std::uintptr_t>(MemoryUtils::align(m_headerSize, m_minBlockSize));

        for (std::size_t level = 0; level < m_numLevels - 1; ++level)
        {
            auto relativeLastSplitBlock = MemoryUtils::align(relativeBufferBodyStart, getBlockSize(level)) - getBlockSize(level);
            auto lastSplitBlock = m_buffer.get() + relativeLastSplitBlock;
            auto lastSplitBlockIndex = getBlockIndex(level, lastSplitBlock);

            for (std::size_t index = 0; index <= lastSplitBlockIndex; ++index)
            {
                setSplit(level, index, true);
            }
        }
    }

    //------------------------------------------------------------------------------
    void* BuddyAllocator::allocate(std::size_t in_allocationSize) noexcept
    {
        auto blockSize = MemoryUtils::align(MemoryUtils::nextPowerofTwo(in_allocationSize), m_minBlockSize);
        auto level = getLevel(blockSize);
        assert(level != 0);

        std::unique_lock<std::mutex> lock(m_mutex);

        auto block = getFreeListStart(level);
        if (block)
        {
            auto blockIndex = getBlockIndex(level, block);
            auto parentBlockIndex = getParentBlockIndex(level, blockIndex);
            toggleChildrenAllocated(level - 1, blockIndex);
        }
        else
        {
            //TODO: !?
        }

        return block;
    }

    //------------------------------------------------------------------------------
    void BuddyAllocator::deallocate(void* in_buffer) noexcept
    {
        //TODO: !?
    }

    //------------------------------------------------------------------------------
    std::size_t BuddyAllocator::getBlockSize(std::size_t in_level) const noexcept
    {
        assert(in_level >= 0 && in_level < m_numLevels);

        auto output = m_bufferSize;
        for (std::size_t level = 0; level < in_level; ++level)
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
    std::size_t BuddyAllocator::getBlockIndex(std::size_t in_level, void* in_pointer) const noexcept
    {
        assert(in_level >= 0 && in_level < m_numLevels);
        assert(in_pointer >= m_buffer.get());
        assert(MemoryUtils::getPointerOffset(in_pointer, m_buffer.get()) < m_bufferSize);
        assert(MemoryUtils::isAligned(MemoryUtils::getPointerOffset(in_pointer, m_buffer.get()), getBlockSize(in_level)));

        auto pointerDiff = reinterpret_cast<std::uintptr_t>(in_pointer) - reinterpret_cast<std::uintptr_t>(m_buffer.get());
        return static_cast<std::size_t>(pointerDiff) / getBlockSize(in_level);
    }

    //------------------------------------------------------------------------------
    std::size_t BuddyAllocator::getParentBlockIndex(std::size_t in_level, std::size_t in_blockIndex) const noexcept
    {
        assert(in_level > 0 && in_level < m_numLevels);
        assert(in_blockIndex < getNumIndicesForLevel(in_level));

        return in_blockIndex >> 1;
    }

    //------------------------------------------------------------------------------
    void* BuddyAllocator::getBlockPointer(std::size_t in_level, std::size_t in_blockIndex) const noexcept
    {
        //TODO: !?
        return nullptr;
    }

    //------------------------------------------------------------------------------
    void* BuddyAllocator::getFreeListStart(std::size_t in_level) noexcept
    {
        assert(in_level >= 0 && in_level < m_numLevels);

        return m_freeListTable[in_level];
    }

    //------------------------------------------------------------------------------
    void BuddyAllocator::setFreeListStart(std::size_t in_level, void* in_start) noexcept
    {
        assert(in_level >= 0 && in_level < m_numLevels);
        assert(in_start >= m_buffer.get());
        assert(MemoryUtils::isAligned(MemoryUtils::getPointerOffset(in_start, m_buffer.get()), getBlockSize(in_level)));

        m_freeListTable[in_level] = in_start;
    }

    //------------------------------------------------------------------------------
    void BuddyAllocator::toggleChildrenAllocated(std::size_t in_level, std::size_t in_blockIndex) noexcept
    {
        assert(in_level >= 0 && in_level < m_numLevels - 1);
        assert(in_blockIndex < getNumIndicesForLevel(in_level));

        auto bufferBlockIndex = (1 << in_level) - 1 + in_blockIndex;
        auto bufferByteIndex = bufferBlockIndex / CHAR_BIT;
        auto bufferBitIndex = bufferBlockIndex - bufferByteIndex;

        reinterpret_cast<std::uint8_t*>(m_allocatedTable)[bufferByteIndex] ^= (1 << bufferBitIndex);
    }

    //------------------------------------------------------------------------------
    void BuddyAllocator::setSplit(std::size_t in_level, std::size_t in_blockIndex, bool isSplit) noexcept
    {
        assert(in_level >= 0 && in_level < m_numLevels - 1);
        assert(in_blockIndex < getNumIndicesForLevel(in_level));

        auto bufferBlockIndex = (1 << in_level) - 1 + in_blockIndex;
        auto bufferByteIndex = bufferBlockIndex / CHAR_BIT;
        auto bufferBitIndex = bufferBlockIndex - bufferByteIndex;

        if (isSplit)
        {
            reinterpret_cast<std::uint8_t*>(m_allocatedTable)[bufferByteIndex] |= (1 << bufferBitIndex);
        }
        else
        {
            reinterpret_cast<std::uint8_t*>(m_allocatedTable)[bufferByteIndex] &= ~(1 << bufferBitIndex);
        }
    }
}