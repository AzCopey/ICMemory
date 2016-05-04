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

#ifndef _ICMEMORY_CONTAINER_VECTORIMPL_H_
#define _ICMEMORY_CONTAINER_VECTORIMPL_H_

namespace IC
{
	//------------------------------------------------------------------------------
	template <typename TType> Vector<TType> MakeVector(IAllocator& allocator) noexcept
	{
		return IC::Vector<TType>(AllocatorWrapper<TType>(&allocator));
	}

	//------------------------------------------------------------------------------
	template <typename TType> Vector<TType> MakeVector(BuddyAllocator& allocator) noexcept
	{
		return IC::Vector<TType>(AllocatorWrapper<TType>(&allocator));
	}

	//------------------------------------------------------------------------------
	template <typename TType> Vector<TType> MakeVector(LinearAllocator& allocator) noexcept
	{
		return IC::Vector<TType>(AllocatorWrapper<TType>(&allocator));
	}

	//------------------------------------------------------------------------------
	template <typename TValueType, typename TIteratorType> Vector<TValueType> MakeVector(BuddyAllocator& allocator, const TIteratorType& first, const TIteratorType& last) noexcept
	{
		return IC::Vector<TValueType>(first, last, AllocatorWrapper<TValueType>(&allocator));
	}

	//------------------------------------------------------------------------------
	template <typename TValueType, typename TIteratorType> Vector<TValueType> MakeVector(LinearAllocator& allocator, const TIteratorType& first, const TIteratorType& last) noexcept
	{
		return IC::Vector<TValueType>(first, last, AllocatorWrapper<TValueType>(&allocator));
	}

	//------------------------------------------------------------------------------
	template <typename TType> Vector<TType> MakeVector(BuddyAllocator& allocator, const std::vector<TType>& toCopy) noexcept
	{
		return IC::Vector<TType>(toCopy.begin(), toCopy.end(), AllocatorWrapper<TType>(&allocator));
	}

	//------------------------------------------------------------------------------
	template <typename TType> Vector<TType> MakeVector(LinearAllocator& allocator, const std::vector<TType>& toCopy) noexcept
	{
		return IC::Vector<TType>(toCopy.begin(), toCopy.end(), AllocatorWrapper<TType>(&allocator));
	}
}

#endif
