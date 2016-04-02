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

#ifndef _ICMEMORY_QUEUE_H_
#define _ICMEMORY_QUEUE_H_

#include "ForwardDeclarations.h"

#include "BuddyAllocator.h"
#include "Deque.h"
#include "LinearAllocator.h"
#include "AllocatorWrapper.h"

#include <queue>

namespace IC
{
    template <typename TType> using Queue = std::queue<TType, Deque<TType>>;

    /// Creates a new empty queue. The given allocator is used for all memory allocations.
    ///
    /// @param allocator
    ///     The allocator which should be used.
    ///
    /// @return The new queue.
    ///
    template <typename TType> Queue<TType> MakeQueue(BuddyAllocator& allocator) noexcept
    {
        return IC::Queue<TType>(AllocatorWrapper<TType>(&allocator));
    }

    /// Creates a new empty queue. The given allocator is used for all memory allocations.
    ///
    /// @param allocator
    ///     The allocator which should be used.
    ///
    /// @return The new queue.
    ///
    template <typename TType> Queue<TType> MakeQueue(LinearAllocator& allocator) noexcept
    {
        return IC::Queue<TType>(AllocatorWrapper<TType>(&allocator));
    }
}

#endif
