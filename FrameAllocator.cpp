// Created by Ian Copland on 2015-09-15
//
// The MIT License(MIT)
// 
// Copyright(c) 2015 Ian Copland
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

#include "FrameAllocator.h"

#include "MemoryUtils.h"

#include <cassert>

namespace IC
{
    //------------------------------------------------------------------------------
    FrameAllocator::FrameAllocator(BuddyAllocator& in_buddyAllocator, std::size_t in_pageSize) noexcept
        : m_pageSize(in_pageSize), m_buddyAllocator(in_buddyAllocator)
    {
        //TODO: !?
    }

    //------------------------------------------------------------------------------
    void FrameAllocator::reset() noexcept
    {
        //TODO: !?
        //m_nextPointer = MemoryUtils::align(m_buffer.get(), m_alignment);
    }

    //------------------------------------------------------------------------------
    void* FrameAllocator::allocate(std::size_t in_allocationSize) noexcept
    {
        //TODO: !?
        //void* output = m_nextPointer;

        //m_nextPointer = MemoryUtils::align(m_nextPointer + in_allocationSize, m_alignment);

        //return output;
        
        return nullptr;
    }
}