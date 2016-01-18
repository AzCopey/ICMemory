// Created by Ian Copland on 2016-01-18
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

#ifndef _IC_MAKEUNIQUE_H_
#define _IC_MAKEUNIQUE_H_

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
        return in_allocator.allocate<TType>(std::forward<TConstructorArgs>(in_constructorArgs)...);
    }
}

#endif