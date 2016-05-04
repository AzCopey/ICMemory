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

#ifndef _ICMEMORY_POOL_PAGEDOBJECTPOOLIMPL_H_
#define _ICMEMORY_POOL_PAGEDOBJECTPOOLIMPL_H_



namespace IC
{
	//------------------------------------------------------------------------------
	template <typename TObjectType> PagedObjectPool<TObjectType>::PagedObjectPool(std::size_t numObjectsPerPage) noexcept
		: m_numObjectsPerPage(numObjectsPerPage), m_freeStorePools()
	{
		m_freeStorePools.push_back(std::unique_ptr<ObjectPool<TObjectType>>(new ObjectPool<TObjectType>(m_numObjectsPerPage)));
	}

	//------------------------------------------------------------------------------
	template <typename TObjectType> PagedObjectPool<TObjectType>::PagedObjectPool(IAllocator& allocator, std::size_t numObjectsPerPage) noexcept
		: m_numObjectsPerPage(numObjectsPerPage), m_allocator(&allocator), m_allocatorPools(MakeVector<UniquePtr<ObjectPool<TObjectType>>>(*m_allocator))
	{
		m_allocatorPools.push_back(MakeUnique<ObjectPool<TObjectType>>(*m_allocator, *m_allocator, m_numObjectsPerPage));
	}

	//------------------------------------------------------------------------------
	template <typename TObjectType> template <typename... TConstructorArgs> UniquePtr<TObjectType> PagedObjectPool<TObjectType>::Create(TConstructorArgs&&... constructorArgs) noexcept
	{
		if (m_allocator)
		{
			for (const auto& pool : m_allocatorPools)
			{
				if (pool->GetNumFreeObjects() > 0)
				{
					return pool->Create(std::forward<TConstructorArgs>(constructorArgs)...);
				}
			}

			m_allocatorPools.push_back(MakeUnique<ObjectPool<TObjectType>>(*m_allocator, *m_allocator, m_numObjectsPerPage));
			return m_allocatorPools.back()->Create(std::forward<TConstructorArgs>(constructorArgs)...);
		}
		else
		{
			for (const auto& pool : m_freeStorePools)
			{
				if (pool->GetNumFreeObjects() > 0)
				{
					return pool->Create(std::forward<TConstructorArgs>(constructorArgs)...);
				}
			}

			m_freeStorePools.push_back(std::unique_ptr<ObjectPool<TObjectType>>(new ObjectPool<TObjectType>(m_numObjectsPerPage)));
			return m_freeStorePools.back()->Create(std::forward<TConstructorArgs>(constructorArgs)...);
		}
	}

	template <typename TObjectType> PagedObjectPool<TObjectType>::~PagedObjectPool() noexcept
	{
		if (m_allocator)
		{
			m_allocatorPools.~vector();
		}
		else
		{
			m_freeStorePools.~vector();
		}
	}
}

#endif