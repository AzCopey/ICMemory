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

#ifndef _ICMEMORY_MAKEUNIQUE_H_
#define _ICMEMORY_MAKEUNIQUE_H_

#include "ForwardDeclarations.h"

#include "BuddyAllocator.h"

namespace IC
{
    /// Allocates a new unique pointer from the given Buddy Allocator with the
    /// given constructor parameters. This follows the make_* convention set in
    /// the standard library.
    ///
    /// @param in_allocator
    ///     The allocator from which to allocate the requested type.
    /// @param in_constructorArgs
    ///     The arguments for the constructor if appropriate.
    ///
    /// @return A unique pointer to the allocated instance.
    ///
    template <typename TType, typename... TConstructorArgs> UniquePtr<TType> makeUnique(BuddyAllocator& in_allocator, TConstructorArgs&&... in_constructorArgs) noexcept
    {
        void* memory = in_allocator.allocate(sizeof(TType));
        TType* object = new (memory) TType(std::forward<TConstructorArgs>(in_constructorArgs)...);
        return UniquePtr<TType>(object, [&in_allocator](TType* in_object) noexcept -> void
        {
            in_object->~TType();
            in_allocator.deallocate(reinterpret_cast<void*>(in_object));
        });
    }

    /// Allocates a new unique pointer from the given Frame Allocator with the
    /// given constructor parameters. This follows the make_* convention set in
    /// the standard library.
    ///
    /// @param in_allocator
    ///     The allocator from which to allocate the requested type.
    /// @param in_constructorArgs
    ///     The arguments for the constructor if appropriate.
    ///
    /// @return A unique pointer to the allocated instance.
    ///
    template <typename TType, typename... TConstructorArgs> UniquePtr<TType> makeUnique(FrameAllocator& in_allocator, TConstructorArgs&&... in_constructorArgs) noexcept
    {
        void* memory = in_allocator.allocate(sizeof(TType));
        TType* object = new (memory) TType(std::forward<TConstructorArgs>(in_constructorArgs)...);
        return UniquePtr<TType>(object, [&in_allocator](TType* in_object) noexcept -> void
        {
            in_object->~TType();
            in_allocator.deallocate(reinterpret_cast<void*>(in_object));
        });
    }

    /// Allocates a new unique pointer to an array from the given Buddy Allocator 
    /// with the given constructor parameters. This follows the make_* convention 
    /// set in the standard library. Note that, like new[], fundamental types  will 
    /// not be set to a default value. 
    ///
    /// @param in_allocator
    ///     The allocator from which to allocate the requested type.
    /// @param in_size
    ///     The size of the array.
    ///
    /// @return A unique pointer to the allocated array.
    ///
    template <typename TType> UniquePtr<TType[]> makeUniqueArray(BuddyAllocator& in_allocator, std::size_t in_size) noexcept
    {
        auto array = reinterpret_cast<TType*>(in_allocator.allocate(sizeof(TType) * in_size));
        if (!std::is_fundamental<TType>::value)
        {
            for (std::size_t i = 0; i < in_size; ++i)
            {
                new (array + i) TType();
            }
        }

        return UniquePtr<TType[]>(array, [&in_allocator, in_size](TType* in_array) noexcept -> void
        {
            if (!std::is_fundamental<TType>::value)
            {
                for (std::size_t i = 0; i < in_size; ++i)
                {
                    (in_array + i)->~TType();
                }
            }

            in_allocator.deallocate(reinterpret_cast<void*>(in_array));
        });
    }

    /// Allocates a new unique pointer to an array from the given Frame Allocator 
    /// with the given constructor parameters. This follows the make_* convention 
    /// set in the standard library. Note that, like new[], fundamental types  will 
    /// not be set to a default value. 
    ///
    /// @param in_allocator
    ///     The allocator from which to allocate the requested type.
    /// @param in_size
    ///     The size of the array.
    ///
    /// @return A unique pointer to the allocated array.
    ///
    template <typename TType> UniquePtr<TType[]> makeUniqueArray(FrameAllocator& in_allocator, std::size_t in_size) noexcept
    {
        auto array = reinterpret_cast<TType*>(in_allocator.allocate(sizeof(TType) * in_size));
        if (!std::is_fundamental<TType>::value)
        {
            for (std::size_t i = 0; i < in_size; ++i)
            {
                new (array + i) TType();
            }
        }

        return UniquePtr<TType[]>(array, [&in_allocator, in_size](TType* in_array) noexcept -> void
        {
            if (!std::is_fundamental<TType>::value)
            {
                for (std::size_t i = 0; i < in_size; ++i)
                {
                    (in_array + i)->~TType();
                }
            }

            in_allocator.deallocate(reinterpret_cast<void*>(in_array));
        });
    }
}

#endif