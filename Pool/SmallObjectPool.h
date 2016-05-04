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

#ifndef _ICMEMORY_POOL_SMALLOBJECTPOOL_H_
#define _ICMEMORY_POOL_SMALLOBJECTPOOL_H_

#include "PagedObjectPool.h"

namespace IC
{
	/// TODO
	///
	class SmallObjectPool final
	{
	public:
		/// TODO
		///
		SmallObjectPool() noexcept;

		/// TODO
		///
		SmallObjectPool(IAllocator& allocator) noexcept;

		/// TODO
		///
		template <typename TObjectType, typename... TConstructorArgs> UniquePtr<TObjectType> Create(TConstructorArgs&&... constructorArgs) noexcept;

	private:
		static constexpr std::size_t k_level1DataSize = sizeof(std::uintptr_t) * 2;
		static constexpr std::size_t k_level2DataSize = sizeof(std::uintptr_t) * 4;
		static constexpr std::size_t k_level3DataSize = sizeof(std::uintptr_t) * 8;
		static constexpr std::size_t k_level4DataSize = sizeof(std::uintptr_t) * 16;

		/// TODO
		///
		template <std::size_t TDataSize> struct DataBlock final
		{
			std::int8_t m_data[TDataSize];
		};

		SmallObjectPool(SmallObjectPool&) = delete;
		SmallObjectPool& operator=(SmallObjectPool&) = delete;
		SmallObjectPool(SmallObjectPool&&) = delete;
		SmallObjectPool& operator=(SmallObjectPool&&) = delete;

		IAllocator* m_allocator = nullptr;

		PagedObjectPool<DataBlock<k_level1DataSize>> m_level1Pool;
		PagedObjectPool<DataBlock<k_level2DataSize>> m_level2Pool;
		PagedObjectPool<DataBlock<k_level3DataSize>> m_level3Pool;
		PagedObjectPool<DataBlock<k_level4DataSize>> m_level4Pool;
	};
}

#include "SmallObjectPoolImpl.h"

#endif