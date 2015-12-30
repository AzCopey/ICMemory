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

#ifndef _IC_FRAMEALLOCATOR_H_
#define _IC_FRAMEALLOCATOR_H_

#include <cstdint>
#include <memory>
#include <functional>

namespace IC
{
    /// TODO
    ///
    class FrameAllocator final
    {
    public:
        /// TODO
        ///
        template <typename TType> using UniquePtr = std::unique_ptr<TType, decltype(&destroy<TType>)>;

        /// TODO
        /// 
        FrameAllocator(std::size_t in_bufferSize, std::uint32_t in_alignment = sizeof(std::intptr_t)) noexcept;

        /// TODO
        /// 
        template <typename TType, typename... TConstructorArgs> UniquePtr<TType> allocate(TConstructorArgs&&... in_constructorArgs) noexcept;

        /// TODO
        /// 
        void reset() noexcept;

    private:
        void* allocate(std::size_t in_allocationSize) noexcept;

        const std::size_t m_bufferSize;
        const std::size_t m_alignment;
        std::unique_ptr<std::uint8_t[]> m_buffer;
        std::uint8_t* m_nextPointer = 0;
    };

    //-----------------------------------------------------------------------------
    template <typename TType, typename... TConstructorArgs> FrameAllocator::UniquePtr<TType> FrameAllocator::allocate(TConstructorArgs&&... in_constructorArgs) noexcept
    {
        void* memory = allocate(sizeof(TType));
        TType* object = new (memory) TType(std::forward<TConstructorArgs>(in_constructorArgs)...);
        return UniquePtr<TType>(object, destroy);
    }

    //-----------------------------------------------------------------------------
    template <typename TType> void destroy(TType* in_object) noexcept
    {
        in_object->~TType();
    }
}

#endif