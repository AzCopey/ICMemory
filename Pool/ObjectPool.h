// Created by Ian Copland on 2016-05-03
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

#ifndef _ICMEMORY_POOL_OBJECTPOOL_H_
#define _ICMEMORY_POOL_OBJECTPOOL_H_

#include "../Utility/MemoryUtils.h"
#include "../Container/UniquePtr.h"

#include <cstdint>
#include <functional>
#include <vector>

namespace IC
{
	/// A fixed size object pool. A contiguous memory block is pre-allocated for all
	/// objects in the pool, which is then used as each new object created.
	///
	/// The object pool can be backed by any of the allocators.
	///
	/// This is not thread-safe and should not be accessed from multiple threads at
	/// the same time.
	///
	template <typename TObjectType> class ObjectPool final
	{
	public:
		/// Creates a new object pool containing the given number of objects. The
		/// memory block used for the pool is allocated from the free store.
		///
		/// @param numObjects
		///		The number of objects which should be in the pool.
		///
		ObjectPool(std::size_t numObjects) noexcept;

		/// Creates a new object pool containing the given number of objects. The 
		/// memory block used for the pool is allocated from the given allocator.
		///
		/// @param allocator
		///		The allocator from which to allocate the memory block.
		/// @param numObjects
		///		The number of objects in the pool.
		///
		ObjectPool(IAllocator& allocator, std::size_t numObjects) noexcept;
		
		/// @return The number of objects in the pool.
		///
		std::size_t GetNumObjects() const noexcept { return m_numObjects; }

		/// @return The number of objects in the pool which are not yet allocated.
		///
		std::size_t GetNumAllocatedObjects() const noexcept { return m_numAllocatedObjects; }

		/// @return The number of objects in the pool which are not yet allocated.
		///
		std::size_t GetNumFreeObjects() const noexcept { return GetNumObjects() - GetNumAllocatedObjects(); }

		/// Creates a new object from the pool. If there are no free objects left
		/// in the pool then this will assert.
		///
		///  @param constructorArgs
		///		The arguments for the constructor if appropriate.
		///
		/// @return The newly constructed object.
		///
		template <typename... TConstructorArgs> UniquePtr<TObjectType> Create(TConstructorArgs&&... constructorArgs) noexcept;

		~ObjectPool() noexcept;

	private:
		ObjectPool(ObjectPool&) = delete;
		ObjectPool& operator=(ObjectPool&) = delete;
		ObjectPool(ObjectPool&&) = delete;
		ObjectPool& operator=(ObjectPool&&) = delete;

		/// A container for information on slots within the pool.
		///
		struct Slot final
		{
			Slot* m_next = nullptr;
			Slot* m_previous = nullptr;
		};

		/// Iterates over the memory buffer setting all slots to "free", and
		/// storing the free slot list as an in-place doubly linked list.
		///
		void InitFreeSlotList() noexcept;

		/// Takes the first item in the free slot list and returns it for use.
		/// If there are no free slots then this will assert.
		///
		void* ClaimNextSlot() noexcept;

		/// Returns the given buffer to the free slot list.
		///
		void ReleaseSlot(void* buffer) noexcept;

		const std::size_t m_numObjects;
		const std::size_t m_alignedObjectSize;
		const std::size_t m_bufferSize;

		IAllocator* m_allocator = nullptr;

		void* m_buffer;
		Slot* m_freeSlotList = nullptr;
		std::size_t m_numAllocatedObjects = 0;
	};
}

#include "ObjectPoolImpl.h"

#endif