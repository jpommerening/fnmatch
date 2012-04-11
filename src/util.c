#define _IN_FNMATCH_
#include "fnmatch.h"
#include "internal.h"
#include <stdlib.h>

#define ALLOC_SIZE(s) (((s)+16)/8*13)

#if defined(__APPLE__) || defined(__MACH__)
#include <malloc/malloc.h>
#define USABLE_SIZE(m,s) malloc_size(m)
#elif defined(__linux__)
#define USABLE_SIZE(m,s) malloc_usable_size(m)
#else
#define USABLE_SIZE(m,s) s
#endif

void* _alloc( size_t size, size_t* alloc ) {
  return _grow( NULL, size, alloc );
}

void* _grow( void* mem, size_t size, size_t* alloc ) {
  size = ALLOC_SIZE(size);  
  if( mem ) {
    if( size <= *alloc ) return mem;
    
    mem = realloc( mem, size );
  } else {
    mem = malloc( size );
  }
  
  if( alloc ) *alloc = mem ? USABLE_SIZE(mem,size) : 0;
  
  return mem;
}

