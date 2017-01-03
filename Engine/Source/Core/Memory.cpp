//
// Memory.cpp
//
#include "pch.h"

#include "Core/Memory.hpp"

// Allocation storage to reserve from bss segment of executable.
#define BSS_ARENA_RESERVED  20971520  // 20 MiB


//---------------------------------------------------------------------------------------
LinearAllocator::LinearAllocator (
	byte * backingStore,
	size_t size
) 
	: _start(backingStore),
	  _end(backingStore + size)
{
	_free = _start;
}

//---------------------------------------------------------------------------------------
LinearAllocator::~LinearAllocator ()
{
	// Assert no memory leaks.
	assert (_free == _start);
}

//---------------------------------------------------------------------------------------
void * LinearAllocator::allocate (
	size_t size,
	size_t align
) {
	// Compute next aligned memory location
	void * result = memory::align_forward (_free, align);

	// Bump free pointer
	_free = reinterpret_cast<byte *>(result) + size;

	return result;
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
	assert (_free >= _start);
	return _free - _start;
}

//---------------------------------------------------------------------------------------
void LinearAllocator::reset ()
{
	// Reset free pointer to start of arena.
	_free = _start;
}

//---------------------------------------------------------------------------------------
namespace
{
	struct MemoryGlobals {
		// Statically allocated memory for storing global allocators.
		static const uint32 ALLOCATOR_MEMORY = sizeof (LinearAllocator);

		// Bootstrap memory to hold memory_global allocators.
		byte buffer[ALLOCATOR_MEMORY];

		// Allocation arena witin .bss segment of executable.
		byte bssArena[BSS_ARENA_RESERVED];

		LinearAllocator * linearAllocator;

		MemoryGlobals () : linearAllocator (nullptr) { }
	};

	MemoryGlobals _memory_globals;
	
} // end namespace



//---------------------------------------------------------------------------------------
namespace memory_globals
{
	// Must be idempotent.
	void init ()
	{
		byte * p = _memory_globals.buffer;
		byte * backingStore = _memory_globals.bssArena;

		_memory_globals.linearAllocator = 
			new (p) LinearAllocator (backingStore, BSS_ARENA_RESERVED);

	}

	LinearAllocator & linearAllocator ()
	{
		return *_memory_globals.linearAllocator;
	}

	void shutdown ()
	{
		_memory_globals.linearAllocator->~LinearAllocator ();
		new (&_memory_globals) MemoryGlobals (); // Reset members
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
