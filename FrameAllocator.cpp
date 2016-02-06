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

#include "FrameAllocator.h"

#include "BuddyAllocator.h"
#include "MemoryUtils.h"
#include "MakeUnique.h"

#include <cassert>

namespace IC
{
    //------------------------------------------------------------------------------
    FrameAllocator::FrameAllocator(BuddyAllocator& in_buddyAllocator, std::size_t in_pageSize) noexcept
        : m_pageSize(in_pageSize), m_buddyAllocator(in_buddyAllocator)
    {
    }

    //------------------------------------------------------------------------------
    void* FrameAllocator::allocate(std::size_t in_allocationSize) noexcept
    {
        assert(in_allocationSize <= m_pageSize);

        if (!m_currentPage || m_nextPointer + in_allocationSize > m_currentPage.get() + m_pageSize)
        {
            createPage();
        }

        std::uint8_t* output = m_nextPointer;
        m_nextPointer = MemoryUtils::align(m_nextPointer + in_allocationSize, sizeof(std::intptr_t));

        ++m_activeAllocationCount;

        return output;
    }

    //------------------------------------------------------------------------------
    void FrameAllocator::deallocate(void* in_pointer) noexcept
    {
        --m_activeAllocationCount;
    }

    //------------------------------------------------------------------------------
    void FrameAllocator::reset() noexcept
    {
        assert(m_activeAllocationCount == 0);

        m_previousPages.clear();
        m_currentPage.reset();
        m_nextPointer = nullptr;
    }

    //------------------------------------------------------------------------------
    void FrameAllocator::createPage() noexcept
    {
        if (m_currentPage)
        {
            m_previousPages.push_back(std::move(m_currentPage));
        }

        m_currentPage = makeUniqueArray<std::uint8_t>(m_buddyAllocator, m_pageSize);
        m_nextPointer = MemoryUtils::align(m_currentPage.get(), sizeof(std::intptr_t));
    }
}