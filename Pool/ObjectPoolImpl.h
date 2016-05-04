// Created by Ian Copland on 2016-05-04
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

#ifndef _ICMEMORY_POOL_OBJECTPOOLIMPL_H_
#define _ICMEMORY_POOL_OBJECTPOOLIMPL_H_

namespace IC
{
	namespace
	{
		/// Calculates the size of an object, aligned to the size of a pointer. 
		/// The minimum size is twice that of the size of a pointer, ensuring
		/// slot data can be stored in free objects.
		///
		/// @return The aligned object size.
		///
		template <typename TObjectType> std::size_t CalcAlignedObjectSize()
		{
			return std::max(sizeof(std::intptr_t) * 2, MemoryUtils::Align(sizeof(TObjectType), sizeof(std::intptr_t)));
		}
	}

	//------------------------------------------------------------------------------
	template <typename TObjectType> ObjectPool<TObjectType>::ObjectPool(std::size_t numObjects) noexcept
		: m_numObjects(numObjects), m_alignedObjectSize(CalcAlignedObjectSize<TObjectType>()), m_bufferSize(m_numObjects * m_alignedObjectSize)
	{
		assert(m_numObjects > 0);

		m_buffer = new std::int8_t[m_bufferSize];

		InitFreeSlotList();
	}
	//------------------------------------------------------------------------------
	template <typename TObjectType> ObjectPool<TObjectType>::ObjectPool(IAllocator& allocator, std::size_t objectsPerPage) noexcept
		: m_numObjects(objectsPerPage), m_alignedObjectSize(CalcAlignedObjectSize<TObjectType>()), m_bufferSize(m_numObjects * m_alignedObjectSize), m_allocator(&allocator)
	{
		assert(m_numObjects > 0);

		m_buffer = m_allocator->Allocate(m_bufferSize);

		InitFreeSlotList();
	}

	//------------------------------------------------------------------------------
	template <typename TObjectType> template <typename... TConstructorArgs> UniquePtr<TObjectType> ObjectPool<TObjectType>::Create(TConstructorArgs&&... constructorArgs) noexcept
	{
		void* memory = ClaimNextSlot();
		TObjectType* newObject = new (memory) TObjectType(std::forward<TConstructorArgs>(constructorArgs)...);

		return UniquePtr<TObjectType>(newObject, [=](TObjectType* objectForDeallocation) noexcept -> void
		{
			objectForDeallocation->~TObjectType();
			ReleaseSlot(reinterpret_cast<void*>(objectForDeallocation));
		});
	}

	//------------------------------------------------------------------------------
	template <typename TObjectType> void ObjectPool<TObjectType>::InitFreeSlotList() noexcept
	{
		Slot* previous = nullptr;
		for (std::size_t i = 0; i < m_numObjects; ++i)
		{
			auto current = reinterpret_cast<Slot*>(reinterpret_cast<std::int8_t*>(m_buffer) + m_alignedObjectSize * i);
			current->m_next = nullptr;
			current->m_previous = previous;

			if (previous)
			{
				previous->m_next = current;
			}

			previous = current;
		}

		m_freeSlotList = reinterpret_cast<Slot*>(m_buffer);
	}

	//------------------------------------------------------------------------------
	template <typename TObjectType> void* ObjectPool<TObjectType>::ClaimNextSlot() noexcept
	{
		assert(m_freeSlotList);

		auto firstFree = m_freeSlotList;
		m_freeSlotList = firstFree->m_next;

		if (m_freeSlotList)
		{
			m_freeSlotList->m_previous = nullptr;
		}

		++m_numAllocatedObjects;

		return firstFree;
	}

	//------------------------------------------------------------------------------
	template <typename TObjectType> void ObjectPool<TObjectType>::ReleaseSlot(void* buffer) noexcept
	{
		assert(buffer >= m_buffer);
		assert(MemoryUtils::GetPointerOffset(buffer, m_buffer) < m_bufferSize);

		auto next = m_freeSlotList;
		m_freeSlotList = reinterpret_cast<Slot*>(buffer);
		m_freeSlotList->m_next = next;
		m_freeSlotList->m_previous = nullptr;

		--m_numAllocatedObjects;
	}

	//------------------------------------------------------------------------------
	template <typename TObjectType> ObjectPool<TObjectType>::~ObjectPool() noexcept
	{
		assert(m_numAllocatedObjects == 0);

		if (m_allocator)
		{
			m_allocator->Deallocate(m_buffer);
		}
		else
		{
			delete[] m_buffer;
		}
	}
}

#endif