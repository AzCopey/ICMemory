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

#include <cstdint>
#include <cassert>
#include <type_traits>

namespace IC
{
    namespace MemoryUtils
    {
        /// TODO
        ///
        template <typename TValueType, typename TAlignmentType> TValueType* align(TValueType* in_value, TAlignmentType in_alignment)
        {
            static_assert(std::is_integral<TAlignmentType>::value, "Alignment must be integral type.");
            static_assert(std::is_unsigned<TAlignmentType>::value, "Alignment must be unsigned.");

            std::uintptr_t value = reinterpret_cast<std::uintptr_t>(in_value);
            std::uintptr_t alignment = static_cast<std::uintptr_t>(in_alignment);

            return reinterpret_cast<TValueType*>((value + alignment - 1) & ~(alignment - 1));
        }

        /// TODO
        ///
        template <typename TValueType, typename TAlignmentType> TValueType align(TValueType in_value, TAlignmentType in_alignment)
        {
            static_assert(std::is_integral<TValueType>::value, "Value must be integral type.");
            static_assert(std::is_unsigned<TValueType>::value, "Value must be unsigned.");
            static_assert(std::is_integral<TAlignmentType>::value, "Alignment must be integral type.");
            static_assert(std::is_unsigned<TAlignmentType>::value, "Alignment must be unsigned.");

            std::uintptr_t value = static_cast<std::uintptr_t>(in_value);
            std::uintptr_t alignment = static_cast<std::uintptr_t>(in_alignment);

            return static_cast<TValueType>((value + alignment - 1) & ~(alignment - 1));
        }

        /// TODO
        ///
        template <typename TValueType, typename TAlignmentType> TValueType isAligned(TValueType in_value, TAlignmentType in_alignment)
        {
            static_assert(std::is_integral<TAlignmentType>::value, "Alignment must be integral type.");
            static_assert(std::is_unsigned<TAlignmentType>::value, "Alignment must be unsigned.");

            return static_cast<TValueType>((static_cast<TAlignmentType>(in_value) & (in_alignment - 1)) == 0);
        }

        /// TODO
        ///
        template <typename TType> constexpr bool isPowerOfTwo(TType in_value)
        {
            static_assert(std::is_integral<TType>::value, "Value must be integral type.");
            static_assert(std::is_unsigned<TType>::value, "Value must be unsigned.");

            return (in_value > 0) && ((in_value & (~in_value + 1)) == in_value);
        }

        /// TODO
        ///
        template <typename TType> TType nextPowerofTwo(TType in_value)
        {
            static_assert(std::is_integral<TType>::value, "Value must be integral type.");
            static_assert(std::is_unsigned<TType>::value, "Value must be unsigned.");

            in_value--;
            in_value |= in_value >> 1;
            in_value |= in_value >> 2;
            in_value |= in_value >> 4;
            in_value |= in_value >> 8;
            in_value |= in_value >> 16;
            in_value++;

            return in_value;
        }

        /// TODO
        ///
        template <typename TType> std::size_t calcShift(TType in_value)
        {
            assert(isPowerOfTwo(in_value));

            TType output = 0;
            while (in_value > 1)
            {
                output++;
                in_value >>= 1;
            }

            return output;
        }

        /// TODO
        ///
        template <typename TTypeA, typename TTypeB> std::uintptr_t getPointerOffset(TTypeA* in_pointer, TTypeB* in_relativeTo)
        {
            auto pointer = reinterpret_cast<std::uintptr_t>(in_pointer);
            auto relativeTo = reinterpret_cast<std::uintptr_t>(in_relativeTo);

            assert(pointer >= relativeTo);
            return (pointer - relativeTo);
        }
    }
}