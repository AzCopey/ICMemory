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

#ifndef _ICMEMORY_CONTAINER_UNORDEREDMAP_H_
#define _ICMEMORY_CONTAINER_UNORDEREDMAP_H_

#include "../Allocator/AllocatorWrapper.h"
#include "../Allocator/IAllocator.h"

#include <unordered_map>

namespace IC
{
    template <typename TKey, typename TValue> using UnorderedMap = std::unordered_map<TKey, TValue, std::hash<TKey>, std::equal_to<TKey>, AllocatorWrapper<std::pair<TKey, TValue>>>;

    /// Creates a new empty map. The given allocator is used for all memory allocations.
    ///
    /// @param allocator
    ///     The allocator which should be used.
    ///
    /// @return The new map.
    ///
    template <typename TKey, typename TValue> UnorderedMap<TKey, TValue> MakeUnorderedMap(IAllocator& allocator) noexcept;

    /// Creates a new map from the given range. The given allocator is used for all 
    /// memory allocations.
    ///
    /// @param allocator
    ///     The allocator which should be used.
    /// @param first
    ///     The iterator pointing to the start of the range.
    /// @param last
    ///     The iterator pointing to the end of the range.
    ///
    /// @return The new map.
    ///
    template <typename TKey, typename TValue, typename TIteratorType> UnorderedMap<TKey, TValue> MakeUnorderedMap(IAllocator& allocator, const TIteratorType& first, const TIteratorType& last) noexcept;

    /// Creates a new map from the std::map. The given allocator is used for all 
    /// memory allocations.
    ///
    /// @param allocator
    ///     The allocator which should be used.
    /// @param toCopy
    ///     The std::map which should be copied.
    ///
    /// @return The new map.
    ///
    template <typename TKey, typename TValue> UnorderedMap<TKey, TValue> MakeUnorderedMap(IAllocator& allocator, const std::unordered_map<TKey, TValue>& toCopy) noexcept;
}

#include "UnorderedMapImpl.h"

#endif
