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

#include "SmallObjectAllocator.h"

#include "../Utility/MemoryUtils.h"

namespace IC
{
    //------------------------------------------------------------------------------
    SmallObjectAllocator::SmallObjectAllocator(std::size_t bufferSize) noexcept
        : m_level1Allocator(k_level1BlockSize, bufferSize / k_level1BlockSize), m_level2Allocator(k_level2BlockSize, bufferSize / k_level2BlockSize), 
        m_level3Allocator(k_level3BlockSize, bufferSize / k_level3BlockSize), m_level4Allocator(k_level4BlockSize, bufferSize / k_level4BlockSize)
    {
        assert(MemoryUtils::IsPowerOfTwo(bufferSize));
    }

    //------------------------------------------------------------------------------
    SmallObjectAllocator::SmallObjectAllocator(IAllocator& parentAllocator, std::size_t bufferSize) noexcept
        : m_level1Allocator(parentAllocator, k_level1BlockSize, bufferSize / k_level1BlockSize), m_level2Allocator(parentAllocator, k_level2BlockSize, bufferSize / k_level2BlockSize),
        m_level3Allocator(parentAllocator, k_level3BlockSize, bufferSize / k_level3BlockSize), m_level4Allocator(parentAllocator, k_level4BlockSize, bufferSize / k_level4BlockSize)
    {
        assert(MemoryUtils::IsPowerOfTwo(bufferSize));
    }

    //------------------------------------------------------------------------------
    void* SmallObjectAllocator::Allocate(std::size_t allocationSize) noexcept
    {
        auto roundedBlockSize = std::max(sizeof(std::intptr_t) * 2, MemoryUtils::NextPowerofTwo(allocationSize));

        switch (roundedBlockSize)
        {
        case k_level1BlockSize:
            return m_level1Allocator.Allocate(allocationSize);
        case k_level2BlockSize:
            return m_level2Allocator.Allocate(allocationSize);
        case k_level3BlockSize:
            return m_level3Allocator.Allocate(allocationSize);
        case k_level4BlockSize:
            return m_level4Allocator.Allocate(allocationSize);
        default:
            assert(false);
            return nullptr;
        }
    }

    //------------------------------------------------------------------------------
    void SmallObjectAllocator::Deallocate(void* pointer) noexcept
    {
        if (m_level1Allocator.ContainsBlock(pointer))
        {
            m_level1Allocator.Deallocate(pointer);
        }
        else if (m_level2Allocator.ContainsBlock(pointer))
        {
            m_level2Allocator.Deallocate(pointer);
        }
        else if (m_level3Allocator.ContainsBlock(pointer))
        {
            m_level3Allocator.Deallocate(pointer);
        }
        else if (m_level4Allocator.ContainsBlock(pointer))
        {
            m_level4Allocator.Deallocate(pointer);
        }
        else
        {
            assert(false);
        }
    }
}