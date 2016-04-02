// Created by Ian Copland on 2016-01-18
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

#ifndef _ICMEMORY_FORWARDDECLARATIONS_H_
#define _ICMEMORY_FORWARDDECLARATIONS_H_

#include <deque>
#include <functional>
#include <memory>
#include <queue>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace IC
{
    class IAllocator;
    class BuddyAllocator;
    class FrameAllocator;
    template <typename TValueType> class AllocatorWrapper;
    template <typename TType> using UniquePtr = std::unique_ptr<TType, std::function<void(typename std::remove_all_extents<TType>::type*)>>;
    template <typename TType> using SharedPtr = std::shared_ptr<TType>;
    
    using String = std::basic_string<char, std::char_traits<char>, AllocatorWrapper<char>>;
    template <typename TType> using Deque = std::deque<TType, AllocatorWrapper<TType>>;
    template <typename TType> using Vector = std::vector<TType, AllocatorWrapper<TType>>;
    template <typename TType> using Queue = std::queue<TType, AllocatorWrapper<TType>>;
    template <typename TType> using UnorderedMap = std::unordered_map<TType, AllocatorWrapper<TType>>;
    template <typename TType> using UnorderedSet = std::unordered_set<TType, AllocatorWrapper<TType>>;
}

#endif