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

#ifndef _ICMEMORY_ALLOCATOR_H_
#define _ICMEMORY_ALLOCATOR_H_

#include "ForwardDeclarations.h"

namespace IC
{
    // specialize for void:
    template <typename TAllocatorType> class StdAllocator<void, TAllocatorType>
    {
    public:
        using pointer = void*;
        using const_pointer = const void*;
        using value_type = void;

        template <class TOtherValueType> struct rebind
        {
            using other = StdAllocator<TOtherValueType, TAllocatorType>;
        };
    };

    template <typename TValueType, typename TAllocatorType> class StdAllocator
    {
    public:
        using size_type = size_t;
        using difference_type = ptrdiff_t;
        using pointer = TValueType*;
        using const_pointer = const TValueType*;
        using reference = TValueType&;
        using const_reference = const TValueType&;
        using value_type = TValueType;

        template <class TOtherValueType> struct rebind {
            using other = StdAllocator<TOtherValueType, TAllocatorType>;
        };

        StdAllocator(TAllocatorType& in_allocator) noexcept
            : m_allocator(in_allocator)
        {
        }

        StdAllocator(const StdAllocator& in_stdAllocator) noexcept
            : m_allocator(in_stdAllocator.get_allocator())
        {
        }

        template <class TOtherValueType> StdAllocator(const StdAllocator<TOtherValueType, TAllocatorType>& in_stdAllocator) noexcept
            : m_allocator(in_stdAllocator.get_allocator())
        {
        }

        pointer address(reference in_ref) const noexcept 
        { 
            return &in_ref; 
        }

        const_pointer address(const_reference in_ref) const noexcept 
        { 
            return &in_ref; 
        }

        pointer allocate(size_type in_count, std::allocator<void>::const_pointer in_hint = nullptr) noexcept
        {
            return reinterpret_cast<TValueType*>(m_allocator.allocate(sizeof(TValueType) * in_count));
        }

        void deallocate(pointer in_pointer, size_type in_count) noexcept
        {
            m_allocator.deallocate(reinterpret_cast<void*>(in_pointer));
        }

        size_type max_size() const noexcept
        {
            return m_allocator.getMaxAllocationSize();
        }

        void construct(pointer in_pointer, const TValueType& in_value) noexcept
        {
            new (in_pointer) TValueType(in_value);
        }

        void destroy(pointer in_pointer) noexcept
        {
            in_pointer->~TValueType();
        }

        TAllocatorType& get_allocator() const noexcept
        {
            return m_allocator;
        }

        bool operator==(const StdAllocator& in_stdAllocator) noexcept
        { 
            return (&m_allocator == &in_stdAllocator.get_allocator());
        }

        bool operator!=(const StdAllocator& in_stdAllocator) noexcept
        { 
            return !operator==(in_stdAllocator); 
        }

    private:
        TAllocatorType& m_allocator;
    };
}

#endif