// Created by Ian Copland on 2016-05-05
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

#include "SmallObjectPool.h"

namespace IC
{
	//------------------------------------------------------------------------------
	SmallObjectPool::SmallObjectPool(std::size_t pageSize) noexcept
		: m_pageSize(pageSize), m_level1Pool(m_pageSize / k_level1DataSize), m_level2Pool(m_pageSize / k_level2DataSize), m_level3Pool(m_pageSize / k_level3DataSize), m_level4Pool(m_pageSize / k_level4DataSize)
	{
	}

	//------------------------------------------------------------------------------
	SmallObjectPool::SmallObjectPool(IAllocator& allocator, std::size_t pageSize) noexcept
		: m_pageSize(pageSize), m_level1Pool(allocator, m_pageSize / k_level1DataSize), m_level2Pool(allocator, m_pageSize / k_level2DataSize), m_level3Pool(allocator, m_pageSize / k_level3DataSize),
		m_level4Pool(allocator, m_pageSize / k_level4DataSize)
	{
	}
}