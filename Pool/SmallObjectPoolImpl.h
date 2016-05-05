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

#ifndef _ICMEMORY_POOL_SMALLOBJECTPOOLIMPL_H_
#define _ICMEMORY_POOL_SMALLOBJECTPOOLIMPL_H_

namespace IC
{
	//------------------------------------------------------------------------------
	template <typename TObjectType, typename... TConstructorArgs> UniquePtr<TObjectType> SmallObjectPool::Create(TConstructorArgs&&... constructorArgs) noexcept
	{
		assert(sizeof(TObjectType) <= GetMaxObjectSize());

		if (sizeof(TObjectType) <= k_level1DataSize)
		{
			return CreateFromPool<TObjectType>(m_level1Pool, std::forward<TConstructorArgs>(constructorArgs)...);
		}
		else if (sizeof(TObjectType) <= k_level2DataSize)
		{
			return CreateFromPool<TObjectType>(m_level2Pool, std::forward<TConstructorArgs>(constructorArgs)...);
		}
		else if (sizeof(TObjectType) <= k_level3DataSize)
		{
			return CreateFromPool<TObjectType>(m_level3Pool, std::forward<TConstructorArgs>(constructorArgs)...);
		}
		else
		{
			return CreateFromPool<TObjectType>(m_level4Pool, std::forward<TConstructorArgs>(constructorArgs)...);
		}
	}

	//------------------------------------------------------------------------------
	template <typename TObjectType, int TBlockSize, typename... TConstructorArgs> 
	UniquePtr<TObjectType> SmallObjectPool::CreateFromPool(PagedObjectPool<DataBlock<TBlockSize>>& pool, TConstructorArgs&&... constructorArgs) const noexcept
	{
		auto data = pool.Create();
		auto deleter = data.get_deleter();
		void* rawData = data.release();

		TObjectType* newObject = new (rawData) TObjectType(std::forward<TConstructorArgs>(constructorArgs)...);

		return UniquePtr<TObjectType>(newObject, [=](TObjectType* objectForDeallocation) noexcept -> void
		{
			objectForDeallocation->~TObjectType();
			deleter(reinterpret_cast<DataBlock<TBlockSize>*>(objectForDeallocation));
		});
	}
}

#endif