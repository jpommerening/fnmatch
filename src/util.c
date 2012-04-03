#define _IN_FNMATCH_
#include "fnmatch.h"
#include "internal.h"
#include <stdlib.h>

#define ALLOC_SIZE(s) (((s)+16)/8*13)

#if defined(HAS_MALLOC_USABLE_SIZE)
#define USABLE_SIZE(m,s) malloc_usable_size(m)
#else
#define USABLE_SIZE(m,s) s
#endif

void* _alloc( size_t size, size_t* alloc ) {
  return _grow( NULL, size, alloc );
}

void* _grow( void* mem, size_t size, size_t* alloc ) {
  size = ALLOC_SIZE(size);
  if( mem )
    mem = realloc( mem, size );
  else
    mem = malloc( size );
  
  if( alloc ) *alloc = mem ? USABLE_SIZE(mem,size) : 0;
  
  return mem;
}

