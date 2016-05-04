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

#include "LinearAllocator.h"

#include "BuddyAllocator.h"
#include "../Utility/MemoryUtils.h"

#include <cassert>

namespace IC
{
    //------------------------------------------------------------------------------
    LinearAllocator::LinearAllocator(std::size_t pageSize) noexcept
        : m_pageSize(pageSize)
    {
    }

    //------------------------------------------------------------------------------
    LinearAllocator::LinearAllocator(BuddyAllocator& buddyAllocator, std::size_t pageSize) noexcept
        : m_pageSize(pageSize), m_buddyAllocator(&buddyAllocator)
    {
    }

    //------------------------------------------------------------------------------
    void* LinearAllocator::Allocate(std::size_t allocationSize) noexcept
    {
        assert(allocationSize <= m_pageSize);

        if (!m_currentPage || m_nextPointer + allocationSize > m_currentPage + m_pageSize)
        {
            CreatePage();
        }

        std::uint8_t* output = m_nextPointer;
        m_nextPointer = MemoryUtils::Align(m_nextPointer + allocationSize, sizeof(std::intptr_t));

		++m_activeAllocationCount;

        return output;
    }

    //------------------------------------------------------------------------------
    void LinearAllocator::Deallocate(void* pointer) noexcept
    {
		--m_activeAllocationCount;
    }

    //------------------------------------------------------------------------------
    void LinearAllocator::Reset() noexcept
    {
        assert(m_activeAllocationCount == 0);

        if (m_currentPage)
        {
            if (m_buddyAllocator)
            {
                for (auto& page : m_previousPages)
                {
                    m_buddyAllocator->Deallocate(reinterpret_cast<void*>(page));
                }

                m_buddyAllocator->Deallocate(reinterpret_cast<void*>(m_currentPage));
            }
            else
            {
                for (auto& page : m_previousPages)
                {
                    delete[] page;
                }

                delete[] m_currentPage;
            }

            m_previousPages.clear();
            m_currentPage = nullptr;
            m_nextPointer = nullptr;
        }
    }

    //------------------------------------------------------------------------------
    void LinearAllocator::CreatePage() noexcept
    {
        if (m_currentPage)
        {
            m_previousPages.push_back(m_currentPage);
        }

        if (m_buddyAllocator)
        {
            m_currentPage = reinterpret_cast<std::uint8_t*>(m_buddyAllocator->Allocate(m_pageSize));
        }
        else
        {
            m_currentPage = new std::uint8_t[m_pageSize];
        }

        m_nextPointer = MemoryUtils::Align(m_currentPage, sizeof(std::intptr_t));
    }

	//------------------------------------------------------------------------------
	LinearAllocator::~LinearAllocator() noexcept
	{
		Reset();
	}
}