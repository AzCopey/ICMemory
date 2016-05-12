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

#include "../Utility/MemoryUtils.h"

#include <cassert>

namespace IC
{
    //------------------------------------------------------------------------------
    LinearAllocator::LinearAllocator(std::size_t pageSize) noexcept
        : m_bufferSize(pageSize)
    {
		m_buffer = new std::uint8_t[m_bufferSize];
		m_nextPointer = MemoryUtils::Align(m_buffer, sizeof(std::intptr_t));
    }

    //------------------------------------------------------------------------------
    LinearAllocator::LinearAllocator(IAllocator& parentAllocator, std::size_t pageSize) noexcept
        : m_bufferSize(pageSize), m_parentAllocator(&parentAllocator)
    {
		m_buffer = reinterpret_cast<std::uint8_t*>(m_parentAllocator->Allocate(m_bufferSize));
		m_nextPointer = MemoryUtils::Align(m_buffer, sizeof(std::intptr_t));
    }

	//------------------------------------------------------------------------------
	std::size_t LinearAllocator::GetFreeSpace() const noexcept
	{
		auto freeSpace = m_bufferSize - MemoryUtils::GetPointerOffset(m_nextPointer, m_buffer);
		auto freeSpaceAligned = freeSpace & ~(sizeof(std::intptr_t) - 1);
		return freeSpaceAligned;
	}

    //------------------------------------------------------------------------------
    void* LinearAllocator::Allocate(std::size_t allocationSize) noexcept
    {
		assert(allocationSize <= GetFreeSpace());

        std::uint8_t* output = m_nextPointer;
        m_nextPointer = MemoryUtils::Align(m_nextPointer + allocationSize, sizeof(std::intptr_t));

		++m_activeAllocationCount;

        return output;
    }

    //------------------------------------------------------------------------------
    void LinearAllocator::Deallocate(void* pointer) noexcept
    {
		assert(Contains(pointer));

		--m_activeAllocationCount;
    }

	//------------------------------------------------------------------------------
	bool LinearAllocator::Contains(void* pointer) noexcept
	{
		return (pointer >= m_buffer && pointer < reinterpret_cast<std::uint8_t*>(m_buffer) + m_bufferSize);
	}

    //------------------------------------------------------------------------------
    void LinearAllocator::Reset() noexcept
    {
        assert(m_activeAllocationCount == 0);

		m_nextPointer = MemoryUtils::Align(m_buffer, sizeof(std::intptr_t));
    }

	//------------------------------------------------------------------------------
	LinearAllocator::~LinearAllocator() noexcept
	{
		Reset();

		if (m_parentAllocator)
		{
			m_parentAllocator->Deallocate(m_buffer);
		}
		else
		{
			delete[] m_buffer;
		}

		m_buffer = nullptr;
	}
}