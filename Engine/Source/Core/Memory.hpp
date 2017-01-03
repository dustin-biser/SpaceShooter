//
// Memory.hpp
//
#pragma once

#include "Core/Types.hpp"


/// Base class for memory allocators.
class Allocator {
public:
	Allocator() { }
	virtual ~Allocator () {}

	virtual void * allocate (
		size_t size,
		size_t align
	) = 0;

	virtual size_t allocatedSize (
		void * ptr
	) = 0;

	virtual size_t totalAllocated ( ) = 0;


	/// Forbid copying of Allocator objects.
	Allocator (const Allocator & other) = delete;
	Allocator & operator = (const Allocator& other) = delete;
};


/// A forward incrementing linear allocator.
/// 
/// Ideal for persisent memory such as per-level assets and data.
/// This allocator uses a pre-allocate memory store on construction.  Each allocation
/// request moves the allocator's free pointer forward. The free pointer is only moved
/// back only when reset() is called.
class LinearAllocator : public Allocator {
public:
	/// Constructs allocator using pre-allocated memory as its backing storage.
	LinearAllocator (
		byte * backingStore, ///< Pointer to backing memory arena.
		size_t size          ///< Size in bytes of backing store.
	);

	~LinearAllocator ();

	void * allocate (
		size_t size,
		size_t align
	) override;

	size_t allocatedSize (
		void * ptr
	) override;

	size_t totalAllocated () override;

	/// Resets the allocator back to its initial state, effectively deallocating all
	/// previous allocations.
	void reset ();

private:
	byte * const _start; //< Start of memory arena.
	byte * const _end;   //< End of memory arena.
	byte * _free;  //< Address of next free byte for allocation.
};



/// Creates a new object of type T using the supplied memory allocator.
#define make_new(a, T, ...)  (new ((a).allocate(sizeof(T), alignof(T))) T(__VA_ARGS__))

/// Frees an object allocated with make_new.
#define make_delete(a, T, p)  do {if (p) {(p)->~T(); a.deallocate(p);}} while (0)

namespace memory
{
	inline void * align_forward (
		void * p,
		size_t align
	);
}


void * memory::align_forward (
	void * p,
	size_t align
) {
	uintptr_t pUint = reinterpret_cast<uintptr_t>(p);
	return reinterpret_cast<void *>((pUint + (align - 1)) & ~(align - 1));
}

/// Functions for accessing global memory data.
namespace memory_globals
{
	/// Initializes the global memory allocators.
	void init ();

	/// Returns the default linear allocator for persistent allocations.
	///
	/// A prior call to memory_globals::init() must be made for this allocator to be
	/// available.
	LinearAllocator & linearAllocator ();


#if false 

	//TODO Dustin - Implement frameAllocator based on ring buffer.

	/// Returns the default frame allocator for per frame data.
	///
	/// You need to call init() for this allocator to be available.
	FrameAllocator & frameAllocator ();
#endif

	/// Shuts down the global memory allocators created by init().
	void shutdown ();
}



//TODO Dustin - Do we want to disable malloc, new, and friends?
//#define DISABLE_EXPLICIT_MEMORY_ALLOCATIONS
#ifdef DISABLE_EXPLICIT_MEMORY_ALLOCATIONS

// Disable linker warning for duplicate symbols of malloc, realloc, calloc.
#pragma warning (disable : 4273) 

/// Disallow C memory allocation related functions.
void * malloc (size_t size);
void * realloc (void* ptr, size_t size);
void * calloc (size_t num, size_t size);
void free (void * ptr);

/// Disallow C++ memory allocation related functions.
void * operator new (std::size_t size);
void * operator new (std::size_t size, const std::nothrow_t& nothrow_value) noexcept;
void * operator new[] (std::size_t size);
void * operator new[] (std::size_t size, const std::nothrow_t& nothrow_value) noexcept;
void operator delete (void * ptr) noexcept;
void operator delete[] (void * ptr) noexcept;

#endif