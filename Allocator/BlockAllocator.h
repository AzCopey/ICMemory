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
	/// An allocator which allocates fixed size memory blocks from a fixed size buffer.
	/// Allocations of a bigger size than that of a single block are not possible.
	///
	/// A BlockAllocator can be backed by other allocator types, from which the block
	/// buffer will be allocated, otherwise it is allocated from the free store.
	///
	/// Note that this is not thread-safe and should not be accessed from multiple
	/// threads at the same time.
	///
	class BlockAllocator final : public IAllocator
	{
	public:
		/// Creates a new BlockAllocator with a buffer allocated from the free store.
		///
		/// @param blockSize
		///		The size of each block in the allocator. The block size must be multiple
		///		of the size of a pointer, at at least twice the size of a pointer.
		/// @param numBlocks
		///		The number of blocks available to the block allocator.
		/// 
		BlockAllocator(std::size_t blockSize, std::size_t numBlocks) noexcept;

		/// Creates a new BlockAllocator with a buffer allocated from the given allocator.
		///
		/// @param allocator
		///		The allocator from which to allocate the block buffer.
		/// @param blockSize
		///		The size of each block in the allocator. The block size must be multiple
		///		of the size of a pointer, at at least twice the size of a pointer.
		/// @param numBlocks
		///		The number of blocks available to the block allocator.
		/// 
		BlockAllocator(IAllocator& parentAllocator, std::size_t blockSize, std::size_t numBlocks) noexcept;

		/// This is thread safe.
		///
		/// @return The maximum allocation size from this allocator. Will be the size of a 
		/// single block.
		///
		std::size_t GetMaxAllocationSize() const noexcept override { return GetBlockSize(); }

		/// This is thread-safe.
		///
		/// @return The size of each block in the buffer.
		///
		std::size_t GetBlockSize() const noexcept { return m_blockSize; }

		/// This is thread-safe.
		///
		/// @return The total number of blocks available to the allocator.
		///
		std::size_t GetNumBlocks() const noexcept { return m_numBlocks; }

		/// @return The current number of allocated blocks in the allocator.
		///
		std::size_t GetNumAllocatedBlocks() const noexcept { return m_numAllocatedBlocks; }

		/// @return The current number of free blocks in the allocator.
		///
		std::size_t GetNumFreeBlocks() const noexcept { return GetNumBlocks() - GetNumAllocatedBlocks(); }

		/// Allocates a block from the allocator. The allocation size must be smaller than 
		/// that of a block, otherwise this will assert. If there are no free blocks in the
		/// buffer then this will assert.
		///
		/// @param allocationSize
		///		The size of allocation required.
		///
		/// @return The block of memory.
		///
		void* Allocate(std::size_t allocationSize) noexcept override;

		/// Deallocates the given block, freeing it for reuse.
		///
		/// @param pointer
		///		The pointer which should be deallocated.
		///
		void Deallocate(void* pointer) noexcept override;

		/// Evaluates whether or not the given block pointer was allocated from this block
		/// allocator.
		///
		/// @param block
		///		The block pointer.
		///
		/// @return Whether or not the block was allocated from this allocator.
		///
		bool ContainsBlock(void* block) noexcept;

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

		IAllocator* m_parentAllocator = nullptr;

		void* m_buffer = nullptr;
		FreeBlock* m_freeBlockList = nullptr;
		std::size_t m_numAllocatedBlocks = 0;
	};
}

#endif