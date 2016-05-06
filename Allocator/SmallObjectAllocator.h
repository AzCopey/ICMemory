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

#ifndef _ICMEMORY_ALLOCATOR_SMALLOBJECTPOOL_H_
#define _ICMEMORY_ALLOCATOR_SMALLOBJECTPOOL_H_

#include "BlockAllocator.h"

#include <array>

namespace IC
{
	/// TODO: !?
	///
	class SmallObjectAllocator final : public IAllocator
	{
	public:
		/// TODO: !?
		/// 
		SmallObjectAllocator(std::size_t bufferSize) noexcept;

		/// TODO: !?
		/// 
		SmallObjectAllocator(IAllocator& parentAllocator, std::size_t bufferSize) noexcept;

		/// TODO: !?
		///
		std::size_t GetMaxAllocationSize() const noexcept override { return k_level4BlockSize; }

		/// TODO: !?
		///
		void* Allocate(std::size_t allocationSize) noexcept override;

		/// TODO: !?
		///
		void Deallocate(void* pointer) noexcept override;

	private:
		static constexpr std::size_t k_level1BlockSize = sizeof(std::intptr_t) * 2;
		static constexpr std::size_t k_level2BlockSize = sizeof(std::intptr_t) * 4;
		static constexpr std::size_t k_level3BlockSize = sizeof(std::intptr_t) * 8;
		static constexpr std::size_t k_level4BlockSize = sizeof(std::intptr_t) * 16;

		SmallObjectAllocator(SmallObjectAllocator&) = delete;
		SmallObjectAllocator& operator=(SmallObjectAllocator&) = delete;
		SmallObjectAllocator(SmallObjectAllocator&&) = delete;
		SmallObjectAllocator& operator=(SmallObjectAllocator&&) = delete;

		BlockAllocator m_level1Allocator;
		BlockAllocator m_level2Allocator;
		BlockAllocator m_level3Allocator;
		BlockAllocator m_level4Allocator;
	};
}

#endif