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

#ifndef _ICMEMORY_VECTOR_H_
#define _ICMEMORY_VECTOR_H_

#include "ForwardDeclarations.h"

#include "BuddyAllocator.h"
#include "LinearAllocator.h"
#include "AllocatorWrapper.h"

namespace IC
{
    template <typename TType> using Vector = std::vector<TType, AllocatorWrapper<TType>>;

    /// Creates a new empty vector. The given allocator is used for all memory allocations.
    ///
    /// @param in_allocator
    ///     The allocator which should be used.
    ///
    /// @return The new vector.
    ///
    template <typename TType> Vector<TType> makeVector(BuddyAllocator& in_allocator) noexcept
    {
        return IC::Vector<TType>(AllocatorWrapper<TType>(&in_allocator));
    }

    /// Creates a new empty vector. The given allocator is used for all memory allocations.
    ///
    /// @param in_allocator
    ///     The allocator which should be used.
    ///
    /// @return The new vector.
    ///
    template <typename TType> Vector<TType> makeVector(LinearAllocator& in_allocator) noexcept
    {
        return IC::Vector<TType>(AllocatorWrapper<TType>(&in_allocator));
    }

    /// Creates a new vector from the given range. The given allocator is used for all 
    /// memory allocations.
    ///
    /// @param in_allocator
    ///     The allocator which should be used.
    /// @param in_first
    ///     The iterator pointing to the start of the range.
    /// @param in_last
    ///     The iterator pointing to the end of the range.
    ///
    /// @return The new vector.
    ///
    template <typename TValueType, typename TIteratorType> Vector<TValueType> makeVector(BuddyAllocator& in_allocator, const TIteratorType& in_first, const TIteratorType& in_last) noexcept
    {
        return IC::Vector<TValueType>(in_first, in_last, AllocatorWrapper<TValueType>(&in_allocator));
    }

    /// Creates a new vector from the given range. The given allocator is used for all 
    /// memory allocations.
    ///
    /// @param in_allocator
    ///     The allocator which should be used.
    /// @param in_first
    ///     The iterator pointing to the start of the range.
    /// @param in_last
    ///     The iterator pointing to the end of the range.
    ///
    /// @return The new vector.
    ///
    template <typename TValueType, typename TIteratorType> Vector<TValueType> makeVector(LinearAllocator& in_allocator, const TIteratorType& in_first, const TIteratorType& in_last) noexcept
    {
        return IC::Vector<TValueType>(in_first, in_last, AllocatorWrapper<TValueType>(&in_allocator));
    }

    /// Creates a new vector from the std::vector. The given allocator is used for all 
    /// memory allocations.
    ///
    /// @param in_allocator
    ///     The allocator which should be used.
    /// @param in_toCopy
    ///     The std::vector which should be copied.
    ///
    /// @return The new vector.
    ///
    template <typename TType> Vector<TType> makeVector(BuddyAllocator& in_allocator, const std::vector<TType>& in_toCopy) noexcept
    {
        return IC::Vector<TType>(in_toCopy.begin(), in_toCopy.end(), AllocatorWrapper<TType>(&in_allocator));
    }

    /// Creates a new vector from the std::vector. The given allocator is used for all 
    /// memory allocations.
    ///
    /// @param in_allocator
    ///     The allocator which should be used.
    /// @param in_toCopy
    ///     The std::vector which should be copied.
    ///
    /// @return The new vector.
    ///
    template <typename TType> Vector<TType> makeVector(LinearAllocator& in_allocator, const std::vector<TType>& in_toCopy) noexcept
    {
        return IC::Vector<TType>(in_toCopy.begin(), in_toCopy.end(), AllocatorWrapper<TType>(&in_allocator));
    }
}

#endif
