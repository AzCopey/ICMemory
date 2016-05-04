// Created by Ian Copland on 2016-05-04
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

#ifndef _ICMEMORY_ALLOCATOR_ALLOCATORIMPL_H_
#define _ICMEMORY_ALLOCATOR_ALLOCATORIMPL_H_

namespace IC
{
	/// Void specialisation of the AllocatorWrapper.
	///
	template<> class AllocatorWrapper<void>
	{
	public:
		using pointer = void*;
		using const_pointer = const void*;
		using value_type = void;

		/// Void specialisation of rebind.
		///
		template <class TOtherType> struct rebind
		{
			using other = AllocatorWrapper<TOtherType>;
		};
	};

	//------------------------------------------------------------------------------
	template <typename TValueType> AllocatorWrapper<TValueType>::AllocatorWrapper(IAllocator* allocator) noexcept
		: m_allocator(allocator)
	{
	}

	//------------------------------------------------------------------------------
	template <typename TValueType> AllocatorWrapper<TValueType>::AllocatorWrapper(const AllocatorWrapper& allocatorWrapper) noexcept
		: m_allocator(allocatorWrapper.get_allocator())
	{
	}

	//------------------------------------------------------------------------------
	template <typename TValueType> template <class TOtherType> AllocatorWrapper<TValueType>::AllocatorWrapper(const AllocatorWrapper<TOtherType>& allocatorWrapper) noexcept
		: m_allocator(allocatorWrapper.get_allocator())
	{
	}

	//------------------------------------------------------------------------------
	template <typename TValueType> typename AllocatorWrapper<TValueType>::pointer AllocatorWrapper<TValueType>::address(reference ref) const noexcept
	{
		return &ref;
	}

	//------------------------------------------------------------------------------
	template <typename TValueType> typename AllocatorWrapper<TValueType>::const_pointer AllocatorWrapper<TValueType>::address(const_reference ref) const noexcept
	{
		return &ref;
	}

	//------------------------------------------------------------------------------
	template <typename TValueType> typename AllocatorWrapper<TValueType>::pointer AllocatorWrapper<TValueType>::allocate(size_type count, std::allocator<void>::const_pointer hint = nullptr) noexcept
	{
		return reinterpret_cast<TValueType*>(m_allocator->Allocate(sizeof(TValueType) * count));
	}

	//------------------------------------------------------------------------------
	template <typename TValueType> void AllocatorWrapper<TValueType>::deallocate(pointer pointer, size_type count) noexcept
	{
		m_allocator->Deallocate(reinterpret_cast<void*>(pointer));
	}

	//------------------------------------------------------------------------------
	template <typename TValueType> typename AllocatorWrapper<TValueType>::size_type AllocatorWrapper<TValueType>::max_size() const noexcept
	{
		return m_allocator->GetMaxAllocationSize();
	}

	//------------------------------------------------------------------------------
	template <typename TValueType> void AllocatorWrapper<TValueType>::construct(pointer pointer, const TValueType& value) noexcept
	{
		new (pointer) TValueType(value);
	}

	//------------------------------------------------------------------------------
	template <typename TValueType> void AllocatorWrapper<TValueType>::destroy(pointer pointer) noexcept
	{
		pointer->~TValueType();
	}

	//------------------------------------------------------------------------------
	template <typename TValueType> IAllocator* AllocatorWrapper<TValueType>::get_allocator() const noexcept
	{
		return m_allocator;
	}

	//------------------------------------------------------------------------------
	template <typename TValueType> bool AllocatorWrapper<TValueType>::operator==(const AllocatorWrapper& allocatorWrapper) noexcept
	{
		return (&m_allocator == &allocatorWrapper.get_allocator());
	}

	//------------------------------------------------------------------------------
	template <typename TValueType> bool AllocatorWrapper<TValueType>::operator!=(const AllocatorWrapper& allocatorWrapper) noexcept
	{
		return !operator==(allocatorWrapper);
	}
}

#endif