#define _IN_FNMATCH_
#include "fnmatch.h"
#include "internal.h"
#include <stdlib.h>
/*
void fnmatch_match_init( fnmatch_match_t* match, fnmatch_pattern_t* pattern ) {
  match->buffer  = malloc( pattern->groups );
  match->buflen  = 0;
  match->alloc   = pattern->groups;
  match->argc    = 0;
  match->argv[0] = NULL;
}

void fnmatch_match_destroy( fnmatch_match_t* match ) {
  free( match->buffer );
}
*/