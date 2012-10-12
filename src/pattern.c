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

  pattern->stats.mchars = 0;
  pattern->stats.groups = 0;
  pattern->stats.parts  = 0;
}

static void fnmatch__pattern_free( fnmatch_pattern_t* pattern ) {
  if( pattern->pattern != NULL )
    free( pattern->pattern );
  if( pattern->program != NULL )
    free( pattern->program );
}

void fnmatch_pattern_destroy( fnmatch_pattern_t* pattern ) {
  assert( pattern );
  fnmatch__pattern_free( pattern );
}

fnmatch_state_t fnmatch_pattern_compile( fnmatch_pattern_t* pattern, const char* expr, int flags ) {
  void*  program;
  size_t proglen;
  fnmatch_stats_t stats;
  
  assert( pattern );
  assert( expr );
  
  program = fnmatch_compile( expr, flags, &proglen, &stats );
  
  if( program == NULL )
    return FNMATCH_ERROR;
  
  fnmatch__pattern_free( pattern );
  
  pattern->pattern = strdup( expr );
  pattern->program = program;
  pattern->proglen = proglen;
  pattern->stats.mchars = stats.mchars;
  pattern->stats.groups = stats.groups;
  pattern->stats.parts  = stats.parts;
  
  return FNMATCH_CONTINUE;
}

fnmatch_state_t fnmatch_pattern_match( fnmatch_pattern_t* pattern, const char* str ) {
  fnmatch_context_t context;
  fnmatch_state_t   state;
  
  assert( pattern );
  assert( str );
  
  fnmatch_context_init( &context, pattern );
  fnmatch_context_push( &context, str );
  
  do {
    state = fnmatch_context_match( &context );
  } while( state == FNMATCH_MATCH );
  
  if( state == FNMATCH_PUSH ) {
    fnmatch_context_push( &context, NULL );
    state = fnmatch_context_match( &context );
    assert( state == FNMATCH_MATCH || state == FNMATCH_NOMATCH );
  } else {
    assert( state == FNMATCH_NOMATCH );
  }
  
  fnmatch_context_destroy( &context );
  return state;
}

fnmatch_state_t fnmatch_pattern_render( fnmatch_pattern_t* pattern, fnmatch_match_t* match ) {
  return FNMATCH_NOMATCH;
}