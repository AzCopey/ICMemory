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

#include <algorithm>

namespace IC
{
	//------------------------------------------------------------------------------
	template <typename TObject> ObjectPool<TObject>::ObjectPool(std::size_t numObjects) noexcept
		: m_blockAllocator(MemoryUtils::GetBlockSize<TObject>(), numObjects)
	{
	}
	//------------------------------------------------------------------------------
	template <typename TObject> ObjectPool<TObject>::ObjectPool(IAllocator& allocator, std::size_t numObjects) noexcept
		: m_blockAllocator(allocator, MemoryUtils::GetBlockSize<TObject>(), numObjects)
	{
	}

	//------------------------------------------------------------------------------
	template <typename TObject> template <typename... TConstructorArgs> UniquePtr<TObject> ObjectPool<TObject>::Create(TConstructorArgs&&... constructorArgs) noexcept
	{
		void* memory = m_blockAllocator.Allocate(sizeof(TObject));
		TObject* newObject = new (memory) TObject(std::forward<TConstructorArgs>(constructorArgs)...);

		return UniquePtr<TObject>(newObject, [=](TObject* objectForDeallocation) noexcept -> void
		{
			objectForDeallocation->~TObject();
			m_blockAllocator.Deallocate(reinterpret_cast<void*>(objectForDeallocation));
		});
	}
}

#endif