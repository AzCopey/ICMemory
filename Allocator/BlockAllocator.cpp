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

#include "BlockAllocator.h"
#include "../Utility/MemoryUtils.h"

#include <vector>

namespace IC
{
	//------------------------------------------------------------------------------
	BlockAllocator::BlockAllocator(std::size_t blockSize, std::size_t numBlocks) noexcept
		: m_blockSize(blockSize), m_numBlocks(numBlocks), m_bufferSize(m_blockSize * m_numBlocks)
	{
		assert(MemoryUtils::IsAligned(blockSize, sizeof(std::intptr_t)));
		assert(blockSize >= sizeof(FreeBlock));

		m_buffer = new std::int8_t[m_bufferSize];

		InitFreeBlockList();
	}

	//------------------------------------------------------------------------------
	BlockAllocator::BlockAllocator(IAllocator& parentAllocator, std::size_t blockSize, std::size_t numBlocks) noexcept		
		: m_blockSize(blockSize), m_numBlocks(numBlocks), m_bufferSize(m_blockSize * m_numBlocks), m_parentAllocator(&parentAllocator)
	{
		assert(MemoryUtils::IsAligned(blockSize, sizeof(std::intptr_t)));
		assert(blockSize >= sizeof(FreeBlock));

		m_buffer = m_parentAllocator->Allocate(m_bufferSize);

		InitFreeBlockList();
	}

	//------------------------------------------------------------------------------
	void* BlockAllocator::Allocate(std::size_t allocationSize) noexcept
	{
		assert(allocationSize <= m_blockSize);
		assert(m_freeBlockList);

		auto block = m_freeBlockList;
		m_freeBlockList = block->m_next;

		if (m_freeBlockList)
		{
			m_freeBlockList->m_previous = nullptr;
		}

		++m_numAllocatedBlocks;

		return block;
	}

	//------------------------------------------------------------------------------
	void BlockAllocator::Deallocate(void* pointer) noexcept
	{
		assert(ContainsBlock(pointer));

		auto next = m_freeBlockList;
		m_freeBlockList = reinterpret_cast<FreeBlock*>(pointer);
		m_freeBlockList->m_next = next;
		m_freeBlockList->m_previous = nullptr;

		--m_numAllocatedBlocks;
	}

	//------------------------------------------------------------------------------
	bool BlockAllocator::ContainsBlock(void* block) noexcept
	{
		return (block >= m_buffer && block < reinterpret_cast<std::uint8_t*>(m_buffer) + m_bufferSize);
	}

	//------------------------------------------------------------------------------
	void BlockAllocator::InitFreeBlockList() noexcept
	{
		FreeBlock* previous = nullptr;
		for (std::size_t i = 0; i < m_numBlocks; ++i)
		{
			auto current = reinterpret_cast<FreeBlock*>(reinterpret_cast<std::int8_t*>(m_buffer) + m_blockSize * i);
			current->m_next = nullptr;
			current->m_previous = previous;

			if (previous)
			{
				previous->m_next = current;
			}

			previous = current;
		}

		m_freeBlockList = reinterpret_cast<FreeBlock*>(m_buffer);
	}
	
	//------------------------------------------------------------------------------
	BlockAllocator::~BlockAllocator() noexcept
	{
		assert(m_numAllocatedBlocks == 0);

		if (m_parentAllocator)
		{
			m_parentAllocator->Deallocate(m_buffer);
		}
		else
		{
			delete[] m_buffer;
		}
	}
}