// Created by Ian Copland on 2016-05-06
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

#include "PagedBlockAllocator.h"

#include <cassert>

namespace IC
{

    //------------------------------------------------------------------------------
    PagedBlockAllocator::PagedBlockAllocator(std::size_t blockSize, std::size_t numBlocksPerPage) noexcept
        : m_blockSize(blockSize), m_numBlocksPerPage(numBlocksPerPage), m_pageSize(m_blockSize * m_numBlocksPerPage), m_freeStoreBlockAllocators()
    {
        m_freeStoreBlockAllocators.push_back(std::unique_ptr<BlockAllocator>(new BlockAllocator(m_blockSize, m_numBlocksPerPage)));
    }

    //------------------------------------------------------------------------------
    PagedBlockAllocator::PagedBlockAllocator(IAllocator& parentAllocator, std::size_t blockSize, std::size_t numBlocksPerPage) noexcept
        : m_blockSize(blockSize), m_numBlocksPerPage(numBlocksPerPage), m_pageSize(m_blockSize * m_numBlocksPerPage), m_parentAllocator(&parentAllocator),
        m_parentAllocatorBlockAllocators(MakeVector<UniquePtr<BlockAllocator>>(*m_parentAllocator))
    {
        m_parentAllocatorBlockAllocators.push_back(MakeUnique<BlockAllocator>(*m_parentAllocator, *m_parentAllocator, m_blockSize, m_numBlocksPerPage));
    }

    //------------------------------------------------------------------------------
    std::size_t PagedBlockAllocator::GetNumPages() const noexcept
    {
        if (m_parentAllocator)
        {
            return m_parentAllocatorBlockAllocators.size();
        }
        else
        {
            return m_freeStoreBlockAllocators.size();
        }
    }

    //------------------------------------------------------------------------------
    void* PagedBlockAllocator::Allocate(std::size_t allocationSize) noexcept
    {
        if (m_parentAllocator)
        {
            for (const auto& blockAllocator : m_parentAllocatorBlockAllocators)
            {
                if (blockAllocator->GetNumFreeBlocks() > 0)
                {
                    return blockAllocator->Allocate(allocationSize);
                }
            }

            m_parentAllocatorBlockAllocators.push_back(MakeUnique<BlockAllocator>(*m_parentAllocator, *m_parentAllocator, m_blockSize, m_numBlocksPerPage));
            return m_parentAllocatorBlockAllocators.back()->Allocate(allocationSize);
        }
        else
        {
            for (const auto& blockAllocator : m_freeStoreBlockAllocators)
            {
                if (blockAllocator->GetNumFreeBlocks() > 0)
                {
                    return blockAllocator->Allocate(allocationSize);
                }
            }

            m_freeStoreBlockAllocators.push_back(std::unique_ptr<BlockAllocator>(new BlockAllocator(m_blockSize, m_numBlocksPerPage)));
            return m_freeStoreBlockAllocators.back()->Allocate(allocationSize);
        }
    }

    //------------------------------------------------------------------------------
    void PagedBlockAllocator::Deallocate(void* pointer) noexcept
    {
        if (m_parentAllocator)
        {
            for (const auto& blockAllocator : m_parentAllocatorBlockAllocators)
            {
                if (blockAllocator->ContainsBlock(pointer))
                {
                    return blockAllocator->Deallocate(pointer);
                }
            }
        }
        else
        {
            for (const auto& blockAllocator : m_freeStoreBlockAllocators)
            {
                if (blockAllocator->ContainsBlock(pointer))
                {
                    return blockAllocator->Deallocate(pointer);
                }
            }
        }

        assert(false);
    }
    
    //------------------------------------------------------------------------------
    PagedBlockAllocator::~PagedBlockAllocator() noexcept
    {
        if (m_parentAllocator)
        {
            m_parentAllocatorBlockAllocators.~vector();
        }
        else
        {
            m_freeStoreBlockAllocators.~vector();
        }
    }
}