# ICMemory #

A collection of efficient memory allocators and pools. This provides four allocator types:

* `BuddyAllocator`: A general allocator which efficiently handles external fragmentation by splitting blocks into "buddies". This is primarily for large allocations.
* `LinearAllocator`: A very fast general allocator which allocates from a linear buffer, and deallocates the entire buffer when `Reset()` is called. This is primarily for large numbers of short lived allocations.
* `BlockAllocator`: A very fast allocator for fixed sized blocks. This is primarily used by `ObjectPool`.
* `SmallObjectAllocator`: A very fast allocator for small objects. This is similar to `BlockAllocator` but has several block sizes. This is primarily used for small objects that aren't suitable for pooling.

Paged versions of some of the allocators are also available, which scale the allocator size if it has run out of space.

For more information on the different allocator types, see the class documentation in the headers.

# Usage #

Allocating using the allocators is simply a case of using one of the various factory methods provided.

```
IC::LinearAllocator linearAllocator(1024);
auto allocated = IC::MakeUnique<int>(linearAllocator);
```

Allocating from a pool is just as simple:

```
IC::ObjectPool<int> objectPool(128);
auto allocated = objectPool.Create();
```

# Links #

* [Website](http://www.icopland.co.uk/)
* [Unit Tests](https://github.com/AzCopey/ICMemoryTest)
* [Benchmarks](https://github.com/AzCopey/ICMemoryBenchmark)
