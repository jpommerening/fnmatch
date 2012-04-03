#define _IN_FNMATCH_
#include "fnmatch.h"
#include "internal.h"

int fnmatch( const char* expr, const char* str, int flags ) {
  fnmatch_state_t   state;
  fnmatch_pattern_t pattern;
  fnmatch_pattern_init( &pattern );
  state = fnmatch_pattern_compile( &pattern, expr );
  if( state == FNMATCH_ERROR ) return -1;
  state = fnmatch_pattern_match( &pattern, str );
  fnmatch_pattern_destroy( &pattern );
  switch( state ) {
    case FNMATCH_MATCH: return 0;
    case FNMATCH_NOMATCH: return FNM_NOMATCH;
    default: return -2;
  }
}
