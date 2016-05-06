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

#ifndef _ICMEMORY_ALLOCATOR_BLOCKALLOCATOR_H_
#define _ICMEMORY_ALLOCATOR_BLOCKALLOCATOR_H_

#include "IAllocator.h"

#include <vector>

namespace IC
{
	/// An allocator which allocates fixed size memory blocks from a fixed size 
	/// buffer.
	///
	class BlockAllocator final : public IAllocator
	{
	public:
		/// TODO
		/// 
		BlockAllocator(std::size_t blockSize, std::size_t numBlocks) noexcept;

		/// TODO
		/// 
		BlockAllocator(IAllocator& parentAllocator, std::size_t blockSize, std::size_t numBlocks) noexcept;

		/// TODO
		///
		std::size_t GetMaxAllocationSize() const noexcept override { return GetBlockSize(); }

		/// TODO
		///
		std::size_t GetBlockSize() const noexcept { return m_blockSize; }

		/// TODO
		///
		std::size_t GetNumBlocks() const noexcept { return m_numBlocks; }

		/// TODO
		///
		std::size_t GetNumFreeBlocks() const noexcept { return GetNumBlocks() - GetNumAllocatedBlocks(); }

		/// TODO
		///
		std::size_t GetNumAllocatedBlocks() const noexcept { return m_numAllocatedBlocks; }

		/// TODO
		///
		void* Allocate(std::size_t allocationSize) noexcept override;

		/// TODO
		///
		void Deallocate(void* pointer) noexcept override;

		~BlockAllocator() noexcept;

	private:
		/// A container for information on free blocks within the pool.
		///
		struct FreeBlock final
		{
			FreeBlock* m_next = nullptr;
			FreeBlock* m_previous = nullptr;
		};

		BlockAllocator(BlockAllocator&) = delete;
		BlockAllocator& operator=(BlockAllocator&) = delete;
		BlockAllocator(BlockAllocator&&) = delete;
		BlockAllocator& operator=(BlockAllocator&&) = delete;

		/// Iterates over the memory buffer setting all blocks to "free", and
		/// storing the free block list as an in-place doubly linked list.
		///
		void InitFreeBlockList() noexcept;

		const std::size_t m_blockSize;
		const std::size_t m_numBlocks;
		const std::size_t m_bufferSize;

		IAllocator* m_allocator = nullptr;

		void* m_buffer = nullptr;
		FreeBlock* m_freeBlockList = nullptr;
		std::size_t m_numAllocatedBlocks = 0;
	};
}

#endif