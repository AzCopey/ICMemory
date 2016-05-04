// Created by Ian Copland on 2016-02-06
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

#ifndef _ICMEMORY_CONTAINER_VECTOR_H_
#define _ICMEMORY_CONTAINER_VECTOR_H_

#include "../Allocator/BuddyAllocator.h"
#include "../Allocator/LinearAllocator.h"
#include "../Allocator/AllocatorWrapper.h"

#include <vector>

namespace IC
{
    template <typename TType> using Vector = std::vector<TType, AllocatorWrapper<TType>>;

    /// Creates a new empty vector. The given allocator is used for all memory allocations.
    ///
    /// @param allocator
    ///     The allocator which should be used.
    ///
    /// @return The new vector.
    ///
	template <typename TType> Vector<TType> MakeVector(BuddyAllocator& allocator) noexcept;

    /// Creates a new empty vector. The given allocator is used for all memory allocations.
    ///
    /// @param allocator
    ///     The allocator which should be used.
    ///
    /// @return The new vector.
    ///
	template <typename TType> Vector<TType> MakeVector(LinearAllocator& allocator) noexcept;

    /// Creates a new vector from the given range. The given allocator is used for all 
    /// memory allocations.
    ///
    /// @param allocator
    ///     The allocator which should be used.
    /// @param first
    ///     The iterator pointing to the start of the range.
    /// @param last
    ///     The iterator pointing to the end of the range.
    ///
    /// @return The new vector.
    ///
	template <typename TValueType, typename TIteratorType> Vector<TValueType> MakeVector(BuddyAllocator& allocator, const TIteratorType& first, const TIteratorType& last) noexcept;

    /// Creates a new vector from the given range. The given allocator is used for all 
    /// memory allocations.
    ///
    /// @param allocator
    ///     The allocator which should be used.
    /// @param first
    ///     The iterator pointing to the start of the range.
    /// @param last
    ///     The iterator pointing to the end of the range.
    ///
    /// @return The new vector.
    ///
	template <typename TValueType, typename TIteratorType> Vector<TValueType> MakeVector(LinearAllocator& allocator, const TIteratorType& first, const TIteratorType& last) noexcept;

    /// Creates a new vector from the std::vector. The given allocator is used for all 
    /// memory allocations.
    ///
    /// @param allocator
    ///     The allocator which should be used.
    /// @param toCopy
    ///     The std::vector which should be copied.
    ///
    /// @return The new vector.
    ///
	template <typename TType> Vector<TType> MakeVector(BuddyAllocator& allocator, const std::vector<TType>& toCopy) noexcept;

    /// Creates a new vector from the std::vector. The given allocator is used for all 
    /// memory allocations.
    ///
    /// @param allocator
    ///     The allocator which should be used.
    /// @param toCopy
    ///     The std::vector which should be copied.
    ///
    /// @return The new vector.
    ///
	template <typename TType> Vector<TType> MakeVector(LinearAllocator& allocator, const std::vector<TType>& toCopy) noexcept;
}

#include "VectorImpl.h"

#endif
