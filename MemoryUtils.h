// Created by Ian Copland on 2015-09-19
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

#ifndef _ICMEMORY_MEMORYUTILS_H_
#define _ICMEMORY_MEMORYUTILS_H_

#include "ForwardDeclarations.h"

#include <cstdint>
#include <cassert>
#include <type_traits>

namespace IC
{
    namespace MemoryUtils
    {
        /// Aligns the given pointer to the given alignment. The alignment should be a power
        /// of two.
        ///
        /// @param value
        ///     The value to align.
        /// @param alignment
        ///     The alignment.
        ///
        /// @return The aligned pointer.
        ///
        template <typename TValueType, typename TAlignmentType> TValueType* Align(TValueType* value, TAlignmentType alignment)
        {
            static_assert(std::is_integral<TAlignmentType>::value, "Alignment must be integral type.");
            static_assert(std::is_unsigned<TAlignmentType>::value, "Alignment must be unsigned.");

            std::uintptr_t valueIntPtr = reinterpret_cast<std::uintptr_t>(value);
            std::uintptr_t alignmentIntPtr = static_cast<std::uintptr_t>(alignment);

            return reinterpret_cast<TValueType*>((valueIntPtr + alignmentIntPtr - 1) & ~(alignmentIntPtr - 1));
        }

        /// Aligns the given integer to the given alignment. The alignment should be a power
        /// of two.
        ///
        /// @param value
        ///     The value to align.
        /// @param alignment
        ///     The alignment.
        ///
        /// @return The aligned integer.
        ///
        template <typename TValueType, typename TAlignmentType> TValueType Align(TValueType value, TAlignmentType alignment)
        {
            static_assert(std::is_integral<TValueType>::value, "Value must be integral type.");
            static_assert(std::is_unsigned<TValueType>::value, "Value must be unsigned.");
            static_assert(std::is_integral<TAlignmentType>::value, "Alignment must be integral type.");
            static_assert(std::is_unsigned<TAlignmentType>::value, "Alignment must be unsigned.");

            std::uintptr_t valueIntPtr = static_cast<std::uintptr_t>(value);
            std::uintptr_t alignmentIntPtr = static_cast<std::uintptr_t>(alignment);

            return static_cast<TValueType>((valueIntPtr + alignmentIntPtr - 1) & ~(alignmentIntPtr - 1));
        }

        /// @param value
        ///     The value.
        /// @param alignment
        ///     The alignment.
        ///
        /// @return Whether or not the given integer is aligned.
        ///
        template <typename TValueType, typename TAlignmentType> TValueType IsAligned(TValueType value, TAlignmentType alignment)
        {
            static_assert(std::is_integral<TAlignmentType>::value, "Alignment must be integral type.");
            static_assert(std::is_unsigned<TAlignmentType>::value, "Alignment must be unsigned.");

            return static_cast<TValueType>((static_cast<TAlignmentType>(value) & (alignment - 1)) == 0);
        }

        /// @param value
        ///     The value to check.
        ///
        /// @return Whether or not the given integer is a power of two.
        ///
        template <typename TType> constexpr bool IsPowerOfTwo(TType value)
        {
            static_assert(std::is_integral<TType>::value, "Value must be integral type.");
            static_assert(std::is_unsigned<TType>::value, "Value must be unsigned.");

            return (value > 0) && ((value & (~value + 1)) == value);
        }

        /// @param value
        ///     The value.
        ///
        /// @return The next power ot two on from the given value. This only supports 32-bit values.
        ///
        template <typename TType> TType NextPowerofTwo(TType value)
        {
            static_assert(std::is_integral<TType>::value, "Value must be integral type.");
            static_assert(std::is_unsigned<TType>::value, "Value must be unsigned.");

            value--;
            value |= value >> 1;
            value |= value >> 2;
            value |= value >> 4;
            value |= value >> 8;
            value |= value >> 16;
            value++;

            return value;
        }

        /// @param value
        ///     The value. Must be a power of two.
        ///
        /// @return The number of times 0x1 has to be shifted to get the given value. 
        ///
        template <typename TType> std::size_t CalcShift(TType value)
        {
            assert(IsPowerOfTwo(value));

            TType output = 0;
            while (value > 1)
            {
                output++;
                value >>= 1;
            }

            return output;
        }

        /// @param pointer
        ///     The pointer.
        /// @param relativeTo
        ///     The pointer that the other is relative to. Must have a lower memory address
        ///     than the value.
        ///
        /// @return The offset in bytes between the two pointers. 
        ///
        template <typename TTypeA, typename TTypeB> std::uintptr_t GetPointerOffset(TTypeA* pointer, TTypeB* relativeTo)
        {
            auto pointerInt = reinterpret_cast<std::uintptr_t>(pointer);
            auto relativeToInt = reinterpret_cast<std::uintptr_t>(relativeTo);

            assert(pointerInt >= relativeToInt);
            return (pointerInt - relativeToInt);
        }
    }
}

#endif