// Created by Ian Copland on 2016-04-02
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

#ifndef _ICMEMORY_STRING_H_
#define _ICMEMORY_STRING_H_

#include "../Allocators/AllocatorWrapper.h"
#include "../Allocators/BuddyAllocator.h"
#include "../Allocators/LinearAllocator.h"

#include <string>

namespace IC
{
    using String = std::basic_string<char, std::char_traits<char>, AllocatorWrapper<char>>;

    /// Creates a new empty string. The given allocator is used for all memory allocations.
    ///
    /// @param allocator
    ///     The allocator which should be used.
    ///
    /// @return The new string.
    ///
    String MakeString(BuddyAllocator& allocator) noexcept;

    /// Creates a new empty string. The given allocator is used for all memory allocations.
    ///
    /// @param allocator
    ///     The allocator which should be used.
    ///
    /// @return The new string.
    ///
    String MakeString(LinearAllocator& allocator) noexcept;

    /// Creates a new string from the given C string. The given allocator is used for all 
    /// memory allocations.
    ///
    /// @param allocator
    ///     The allocator which should be used.
    /// @param cString
    ///     The C string. Must be null terminated.
    ///
    /// @return The new string.
    ///
    String MakeString(BuddyAllocator& allocator, const char* cString) noexcept;

    /// Creates a new string from the given C string. The given allocator is used for all 
    /// memory allocations.
    ///
    /// @param allocator
    ///     The allocator which should be used.
    /// @param cString
    ///     The C string. Must be null terminated.
    ///
    /// @return The new string.
    ///
    String MakeString(LinearAllocator& allocator, const char* cString) noexcept;

    /// Creates a new string from the buffer. The given allocator is used for all 
    /// memory allocations.
    ///
    /// @param allocator
    ///     The allocator which should be used.
    /// @param buffer
    ///     The buffer.
    /// @param bufferSize
    ///     The size of the buffer.
    ///
    /// @return The new string.
    ///
    String MakeString(BuddyAllocator& allocator, const char* buffer, std::size_t bufferSize) noexcept;

    /// Creates a new string from the buffer. The given allocator is used for all 
    /// memory allocations.
    ///
    /// @param allocator
    ///     The allocator which should be used.
    /// @param buffer
    ///     The buffer.
    /// @param bufferSize
    ///     The size of the buffer.
    ///
    /// @return The new string.
    ///
    String MakeString(LinearAllocator& allocator, const char* buffer, std::size_t bufferSize) noexcept;

    /// Creates a new string from a std::string. The given allocator is used for all  
    /// memory allocations.
    ///
    /// @param allocator
    ///     The allocator which should be used.
    /// @param toCopy
    ///     The string to copy.
    ///
    /// @return The new string.
    ///
    String MakeString(BuddyAllocator& allocator, const std::string& toCopy) noexcept;

    /// Creates a new string from a std::string. The given allocator is used for all  
    /// memory allocations.
    ///
    /// @param allocator
    ///     The allocator which should be used.
    /// @param toCopy
    ///     The string to copy.
    ///
    /// @return The new string.
    ///
    String MakeString(LinearAllocator& allocator, const std::string& toCopy) noexcept;

    /// Creates a new string from the given range. The given allocator is used for all 
    /// memory allocations.
    ///
    /// @param allocator
    ///     The allocator which should be used.
    /// @param first
    ///     The iterator pointing to the start of the range.
    /// @param last
    ///     The iterator pointing to the end of the range.
    ///
    /// @return The new string.
    ///
	template <typename TIteratorType> String MakeString(BuddyAllocator& allocator, const TIteratorType& first, const TIteratorType& last) noexcept;

    /// Creates a new string from the given range. The given allocator is used for all 
    /// memory allocations.
    ///
    /// @param allocator
    ///     The allocator which should be used.
    /// @param first
    ///     The iterator pointing to the start of the range.
    /// @param last
    ///     The iterator pointing to the end of the range.
    ///
    /// @return The new string.
    ///
	template <typename TIteratorType> String MakeString(LinearAllocator& allocator, const TIteratorType& first, const TIteratorType& last) noexcept;
}

#include "StringImpl.h"

#endif
