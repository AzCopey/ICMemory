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

#include "String.h"

namespace IC
{
    //------------------------------------------------------------------------------
    String MakeString(BuddyAllocator& allocator) noexcept
    {
        return String(AllocatorWrapper<char>(&allocator));
    }

    //------------------------------------------------------------------------------
    String MakeString(LinearAllocator& allocator) noexcept
    {
        return String(AllocatorWrapper<char>(&allocator));
    }

    //------------------------------------------------------------------------------
    String MakeString(BuddyAllocator& allocator, const char* cString) noexcept
    {
        return String(cString, AllocatorWrapper<char>(&allocator));
    }

    //------------------------------------------------------------------------------
    String MakeString(LinearAllocator& allocator, const char* cString) noexcept
    {
        return String(cString, AllocatorWrapper<char>(&allocator));
    }

    //------------------------------------------------------------------------------
    String MakeString(BuddyAllocator& allocator, const char* buffer, std::size_t bufferSize) noexcept
    {
        return String(buffer, bufferSize, AllocatorWrapper<char>(&allocator));
    }

    //------------------------------------------------------------------------------
    String MakeString(LinearAllocator& allocator, const char* buffer, std::size_t bufferSize) noexcept
    {
        return String(buffer, bufferSize, AllocatorWrapper<char>(&allocator));
    }

    //------------------------------------------------------------------------------
    String MakeString(BuddyAllocator& allocator, const std::string& toCopy) noexcept
    {
        return String(toCopy.c_str(), toCopy.size(), AllocatorWrapper<char>(&allocator));
    }

    //------------------------------------------------------------------------------
    String MakeString(LinearAllocator& allocator, const std::string& toCopy) noexcept
    {
        return String(toCopy.c_str(), toCopy.size(), AllocatorWrapper<char>(&allocator));
    }
}