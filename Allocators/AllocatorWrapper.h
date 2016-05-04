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

#include "../ForwardDeclarations.h"

namespace IC
{
    /// A wrapper arround a IC::IAllocator which can be used by the standard library
    /// classes. This implements the std::allocator protocol.
    ///
    /// Note that the wrapper must not out live the allocator which it contains.
    ///
    template <typename TValueType> class AllocatorWrapper
    {
    public:
        using size_type = size_t;
        using difference_type = ptrdiff_t;
        using pointer = TValueType*;
        using const_pointer = const TValueType*;
        using reference = TValueType&;
        using const_reference = const TValueType&;
        using value_type = TValueType;

        /// Provides the type name of this std::allocator type if it used a different
        /// value type.
        ///
        template <class TOtherType> struct rebind 
        {
            using other = AllocatorWrapper<TOtherType>;
        };

        /// Constructs a new wrapper with the given allocator. The wrapper must not
        /// out live the given allocator.
        ///
        /// @param allocator
        ///     The allocator which should be wrapped.
        ///
		AllocatorWrapper(IAllocator* allocator) noexcept;

        /// Constructors a new wrapper using the allocator contained by the given
        /// wrapper.
        ///
        /// @param allocator
        ///     The allocator which should be wrapped.
        ///
		AllocatorWrapper(const AllocatorWrapper& allocatorWrapper) noexcept;

        /// Constructors a new wrapper using the allocator contained by the given
        /// wrapper.
        ///
        /// @param allocator
        ///     The allocator which should be wrapped.
        ///
		template <class TOtherType> AllocatorWrapper(const AllocatorWrapper<TOtherType>& allocatorWrapper) noexcept;
#
        /// @return The address of the given reference.
        ///
		pointer address(reference ref) const noexcept;

        /// @return The address of the given reference.
        ///
		const_pointer address(const_reference ref) const noexcept;

        /// Allocates a series of ValueType objects from the wrapped allocator.
        ///
        /// @param count
        ///     The number of objects to allocate.
        /// @param hint
        ///     Unused in this implementation. Should be left null.
        ///
		pointer allocate(size_type count, std::allocator<void>::const_pointer hint = nullptr) noexcept;

        /// Deallocates the memory block previously allocated though allocate.
        /// The count must be identical to that provided to allocator otherwise the
        /// behaviour is undefined.
        ///
        /// @param pointer
        ///     The memory location to deallocate.
        /// @param count
        ///     The number of objects in the original allocation.
        ///
		void deallocate(pointer pointer, size_type count) noexcept;

        /// @return The max size the contained allocator can allocate in a single block.
        ///
		size_type max_size() const noexcept;

        /// Calls the constructor on the object at the given memory address.
        ///
        /// @param pointer
        ///     The memory address.
        /// @param value
        ///     The value to construct the object with.
        ///
		void construct(pointer pointer, const TValueType& value) noexcept;

        /// Calls the destructor on the object at the given memory address.
        ///
		void destroy(pointer pointer) noexcept;

        /// @return The contained allocator.
        ///
		IAllocator* get_allocator() const noexcept;

        /// @param allocatorWrapper
        ///     The wrapper to compare
        ///
        /// @return Whether or not the two wrappers contain the same allocator.
        ///
		bool operator==(const AllocatorWrapper& allocatorWrapper) noexcept;

        /// @param allocatorWrapper
        ///     The wrapper to compare
        ///
        /// @return Whether or not the two wrappers contain different allocators.
        ///
		bool operator!=(const AllocatorWrapper& allocatorWrapper) noexcept;

    private:
        IAllocator* m_allocator;
    };
}

#include "AllocatorWrapperImpl.h"

#endif