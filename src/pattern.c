#define _IN_FNMATCH_
#include "fnmatch.h"
#include "internal.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

void fnmatch_pattern_init( fnmatch_pattern_t* pattern ) {
  assert( pattern );
  
  pattern->pattern = NULL;
  pattern->program = NULL;
  pattern->proglen = 0;
  pattern->alloc = 0;
  pattern->mchars = 0;
  pattern->groups = 0;
  pattern->parts = 0;
}

void fnmatch_pattern_destroy( fnmatch_pattern_t* pattern ) {
  assert( pattern );
  free( pattern->pattern );
  free( pattern->program );
}

fnmatch_state_t fnmatch_pattern_compile( fnmatch_pattern_t* pattern, const char* expr ) {
  size_t length = strlen( expr ) + 1;
  pattern->pattern = malloc( length );
  memcpy( pattern->pattern, expr, length );
  return fnmatch_compile( pattern );
}

fnmatch_state_t fnmatch_pattern_match( fnmatch_pattern_t* pattern, const char* str ) {
  fnmatch_context_t context;
  fnmatch_state_t   state;
  
  assert( pattern );
  assert( str );
  
  fnmatch_context_init( &context, pattern );
  fnmatch_context_push( &context, str );
  do {
    fnmatch_context_debug( &context );
    state = fnmatch_context_match( &context );
    switch( state ) {
      case FNMATCH_PUSH:
        fnmatch_context_push( &context, NULL );
        break;
      case FNMATCH_POP:
        fnmatch_context_pop( &context );
        break;
      case FNMATCH_ERROR:
        return FNMATCH_ERROR;
      default:
        break;
    }
  } while( state != FNMATCH_STOP );

  fnmatch_context_destroy( &context );
  return context.match == 0 ? FNMATCH_NOMATCH : FNMATCH_MATCH;
}

