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

#ifndef _IC_BUDDYALLOCATOR_H_
#define _IC_BUDDYALLOCATOR_H_

#include "UniquePtr.h"

#include <cstdint>
#include <memory>
#include <mutex>

namespace IC
{
	/// TODO
	///
	class BuddyAllocator final
	{
	public:
		/// TODO
		///
		BuddyAllocator(std::size_t in_bufferSize, std::size_t in_minBlockSize = 64) noexcept;

		/// TODO (Thread-safe)
		///
		std::size_t getBufferSize() const noexcept { return m_bufferSize; }

		/// TODO (Thread-safe)
		///
		std::size_t getMinBlockSize() const noexcept { return m_minBlockSize; }

		/// TODO (thread-safe)
		/// 
		template <typename TType, typename... TConstructorArgs> UniquePtr<TType> allocate(TConstructorArgs&&... in_constructorArgs) noexcept;

	private:
		/// TODO (NOT Thread-safe)
		///
		void initFreeListTable() noexcept;

		/// TODO (NOT Thread-safe)
		///
		void initAllocatedTable() noexcept;

		/// TODO (NOT Thread-safe)
		///
		void initSplitTable() noexcept;

		/// TODO (Thread-safe - locking)
		///
		void* allocate(std::size_t in_allocationSize) noexcept;

		/// TODO (Thread-safe - locking)
		///
		void deallocate(void* buffer) noexcept;

		/// TODO (Thread-safe)
		///
		std::size_t getBlockSize(std::size_t in_level) const noexcept;

		/// TODO (Thread-safe)
		///
		std::size_t getLevel(std::size_t in_blockSize) const noexcept;

		/// TODO (Thread-safe)
		///
		std::size_t getBlockIndex(std::size_t in_level, void* in_pointer) const noexcept;

		/// TODO (Thread-safe)
		///
		std::size_t getParentBlockIndex(std::size_t in_level, std::size_t in_blockIndex) const noexcept;

		/// TODO
		///
		void* getBlockPointer(std::size_t in_level, std::size_t in_blockIndex) const noexcept;

		/// TODO (NOT Thread-safe)
		///
		void* getFreeListStart(std::size_t in_level) noexcept;

		/// TODO (NOT Thread-safe)
		///
		void setFreeListStart(std::size_t in_level, void* in_start) noexcept;

		/// TODO (NOT Thread-safe)
		///
		void toggleChildrenAllocated(std::size_t in_level, std::size_t in_blockIndex) noexcept;

		/// TODO (NOT Thread-safe)
		///
		void setSplit(std::size_t in_level, std::size_t in_blockIndex, bool isSplit) noexcept;

		const std::size_t m_bufferSize;
		const std::size_t m_minBlockSize;
		const std::size_t m_numLevels;
		const std::size_t m_freeListTableSize;
		const std::size_t m_blockDataTableSize;
		const std::size_t m_blockDataTableSizeAligned;
		const std::size_t m_blockDataTableSizeBits;
		const std::size_t m_headerSize;

		std::unique_ptr<std::uint8_t[]> m_buffer;
		void** m_freeListTable;
		void* m_allocatedTable;
		void* m_splitTable;

		std::mutex m_mutex;
	};

	//-----------------------------------------------------------------------------
	template <typename TType, typename... TConstructorArgs> UniquePtr<TType> BuddyAllocator::allocate(TConstructorArgs&&... in_constructorArgs) noexcept
	{
		void* memory = allocate(sizeof(TType));
		TType* object = new (memory) TType(std::forward<TConstructorArgs>(in_constructorArgs)...);
		return UniquePtr<TType>(object, [](TType* in_object) -> void
		{
			in_object->~TType();
		});
	}
}

#endif