//
// Memory.cpp
//
#include "pch.h"

#include "Core/Memory.hpp"

// Allocation storage to reserve from bss segment of executable.
#define BSS_STORAGE_RESERVED  20971520  // 20 MiB


//---------------------------------------------------------------------------------------
LinearAllocator::LinearAllocator (
	byte * backingStore,
	size_t size
) {
	_start = backingStore;
	_end = _start + size;
}

//---------------------------------------------------------------------------------------
void * LinearAllocator::allocate (
	size_t size,
	size_t align
) {
	//TODO Dustin - Finish implementation.

	return nullptr;
}

//---------------------------------------------------------------------------------------
size_t LinearAllocator::allocatedSize (
	void * ptr
) {
	//TODO Dustin - Finish implementation.

	return 0u;
}

//---------------------------------------------------------------------------------------
size_t LinearAllocator::totalAllocated ()
{
	//TODO Dustin - Finish implementation.

	return 0u;
}

//---------------------------------------------------------------------------------------
void LinearAllocator::reset ()
{

}

//---------------------------------------------------------------------------------------
namespace
{
	struct MemoryGlobals {
		static const uint32 ALLOCATOR_MEMORY = sizeof (LinearAllocator);

		// Bootstrap memory to hold memory_global allocators.
		byte buffer[ALLOCATOR_MEMORY];

		// Memory to use witin .bss segment of executable.
		// Backing storeage for LinearAllocator.
		byte bssStore[BSS_STORAGE_RESERVED];

		LinearAllocator * linearAllocator;

		MemoryGlobals () : linearAllocator (nullptr) { }
	};

	MemoryGlobals _memory_globals;

} // end namespace


//---------------------------------------------------------------------------------------
namespace memory_globals
{
	void init ()
	{
		byte * p = _memory_globals.buffer;
		byte * backingStore = _memory_globals.bssStore;
		_memory_globals.linearAllocator = 
			new (p) LinearAllocator (backingStore, BSS_STORAGE_RESERVED);

	}

	LinearAllocator & linearAllocator ()
	{
		return *_memory_globals.linearAllocator;
	}

	void shutdown ()
	{

	}

}



#ifdef DISABLE_EXPLICIT_MEMORY_ALLOCATIONS
#define ALLOCATOR_MESSAGE "Memory allocations must be made through Allocator objects." 


//---------------------------------------------------------------------------------------
void * malloc (
	size_t size
) {
	ForceBreak ("Call to malloc forbidden. " ALLOCATOR_MESSAGE);
	return nullptr;
}

//---------------------------------------------------------------------------------------
void * realloc (
	void* ptr,
	size_t size
) {
	ForceBreak ("Call to realloc forbidden. " ALLOCATOR_MESSAGE);
	return nullptr;
}

//---------------------------------------------------------------------------------------
void * calloc (
	size_t num,
	size_t size
) {
	ForceBreak ("Call to calloc forbidden. " ALLOCATOR_MESSAGE);
	return nullptr;
}


//---------------------------------------------------------------------------------------
void * operator new (
	std::size_t size
) {
	ForceBreak ("Call to non-placement new forbidden. " ALLOCATOR_MESSAGE);
	return nullptr;
}

//---------------------------------------------------------------------------------------
void * operator new (
	std::size_t size,
	const std::nothrow_t & nothrow_value
) noexcept {
	ForceBreak ("Call to non-placement new forbidden. " ALLOCATOR_MESSAGE);
	return nullptr;
}

#endif // DISABLE_EXPLICIT_MEMORY_ALLOCATIONS
