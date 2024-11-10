#ifndef GT_ARENA_H
#define GT_ARENA_H
//	
//				Custom Arena & Stack Allocators
//	
//		Custom memory allocators
//		
//		To-Do List / Future Features:
//		 - STACK ALLOCATOR!
//		 - Global lists to keep track of allocators and memory use statistics?
//		 - Use statistics could help recommend good pool sizes
//		 - Custom 'pointer' type that allows for more compact pointers and error handling (i.e. index into memory pool that can be deref'd to a void *)
//		 - Block/Pool Allocator?
//		 - Combine the types into union "alloc"/"arena" type with macro bindings to simplify use?

#include <SDL2/SDL.h>
#include "screen.h"
#include "util.h"
#include "log.h"

//	
//		Constant Definitions
//	

#define ARENA_DEFAULT_SIZE 0x400
#define ARENA_BLOCK_SIZE 0x400 // How much to expand the memory pool by if needed


//	
//		Type Definitions
//	

//	Scratch/Monotonic Allocator
//	
typedef struct {
	void *curr;
	size_t cap;
} GTAllocScratch;

//	Stack Allocator
//	
typedef struct {
	void *root;
	void *curr;
	size_t cap;
} GTAllocStack;


//	
//		Function Declarations
//	

////	Scratch Allocator

//	Creates a Scratch/Monotonic style Allocator
//	
//	If `cap` is 0, the default (`ARENA_DEFAULT_SIZE`) is used instead
GTAllocScratch *GTAlloc_Scratch_Create(size_t cap);

//	Destroys a Scratch Allocator
//	
//	Does nothing if `alo` is `NULL`
//	Note that the allocator's memory is shared with the data pool,
//	so `(SDL_)free`-ing `alo` is technically safe.
//	This should be avoided though, to ensure internal state is correct.
void GTAlloc_Scratch_Destroy(GTAllocScratch *alo);

//	Resizes the memory pool of an existing allocator
//	
//	The pool is resized in increments of `ARENA_BLOCK_SIZE`, so
//	it may actually be larger than `size`
//	
//	`size` should be the new requested size.
//	If it's less than or equal to the current capacity, nothing is done.
//	
//	Returns the pointer to the resized allocator or
//	NULL if `alo` is NULL
GTAllocScratch *GTAlloc_Scratch_Resize(GTAllocScratch *alo, size_t size);

//	Calculates how much memory is in use in the pool
//	
//	Used internally to check if an allocation has space
//	Returns 0 if `alo` is NULL.
size_t GTAlloc_Scratch_Used(GTAllocScratch *alo);

//	Prints basic statistics on the allocator
//	
//	Only really for debugging
void GTAlloc_Scratch_PrintStats(GTAllocScratch *alo);

//	Allocates some memory from a Scratch allocator's memory pool
//	
//	If `size` would go over the current capacity, it returns NULL and logs an error
//	Returns NULL if `alo` is NULL, and logs an error.
//	Returns NULL if `size` is 0, and logs an error.
void *GTAlloc_Scratch_Alloc(GTAllocScratch *alo, size_t size);

//	Frees the memory pool of a Scratch allocator
//	
//	Note that, as a scratch allocator, this frees *all* pointers allocated
//	from the given allocator
//	Also note, though: This doesn't free the allocator itself,
//	For that you must use `GTAlloc_Scratch_Destroy()`
void GTAlloc_Scratch_FreeAll(GTAllocScratch *alo);


////	Stack Allocator


#endif