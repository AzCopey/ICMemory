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

#include "MemoryUtils.h"

#include <cassert>

namespace IC
{
	//------------------------------------------------------------------------------
	FrameAllocator::FrameAllocator(std::size_t in_bufferSize, std::uint32_t in_alignment) noexcept
		: m_bufferSize(in_bufferSize), m_alignment(in_alignment)
	{
		assert(MemoryUtils::isPowerOfTwo(in_bufferSize));
		assert(MemoryUtils::isPowerOfTwo(in_alignment));

		m_buffer = std::unique_ptr<std::uint8_t[]>(new std::uint8_t[m_bufferSize]);
		reset();
	}

	//------------------------------------------------------------------------------
	void FrameAllocator::reset() noexcept
	{
		m_nextPointer = MemoryUtils::align(m_buffer.get(), m_alignment);
	}

	//------------------------------------------------------------------------------
	void* FrameAllocator::allocate(std::size_t in_allocationSize) noexcept
	{
		void* output = m_nextPointer;

		m_nextPointer = MemoryUtils::align(m_nextPointer + in_allocationSize, m_alignment);

		return output;
	}
}