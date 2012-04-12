#include "fnmatch.h"
#include <assert.h>

void fnmatch_scanner_init( fnmatch_scanner_t* scanner, fnmatch_pattern_t* pattern,
  fnmatch_push_cb push_cb, fnmatch_pop_cb pop_cb, fnmatch_match_cb match_cb ) {
  assert( scanner );
  assert( pattern );
  
  fnmatch_context_init( &(scanner->context), pattern );
  scanner->push_cb  = push_cb;
  scanner->pop_cb   = pop_cb;
  scanner->match_cb = match_cb;
}

void fnmatch_scanner_destroy( fnmatch_scanner_t* scanner ) {
  assert( scanner );
  fnmatch_context_destroy( &(scanner->context) );
}

static fnmatch_state_t fnmatch__scanner_push( fnmatch_scanner_t* scanner, void* info ) {
  assert( scanner );
  if( scanner->push_cb )
    return (scanner->push_cb)( &(scanner->context), info );

  fnmatch_context_push( &(scanner->context), NULL );
  return FNMATCH_CONTINUE;
}

static fnmatch_state_t fnmatch__scanner_pop( fnmatch_scanner_t* scanner, void* info ) {
  assert( scanner );
  if( scanner->pop_cb )
    return (scanner->pop_cb)( &(scanner->context), info );

  fnmatch_context_pop( &(scanner->context) );
  return FNMATCH_CONTINUE;
}

static fnmatch_state_t fnmatch__scanner_match( fnmatch_scanner_t* scanner, void* info ) {
  assert( scanner );
  if( scanner->match_cb )
    return (scanner->match_cb)( &(scanner->context), NULL, info );

  return FNMATCH_CONTINUE;
}

fnmatch_state_t fnmatch_scanner_match( fnmatch_scanner_t* scanner, void* info ) {
  fnmatch_state_t state;

  assert( scanner );

  state = scanner->context.state;
  
  do {
    switch( state ) {
      case FNMATCH_MATCH:
        state = fnmatch__scanner_match( scanner, info );
        break;
      case FNMATCH_PUSH:
        state = fnmatch__scanner_push( scanner, info );
        break;
      case FNMATCH_POP:
        state = fnmatch__scanner_pop( scanner, info );
        break;
      case FNMATCH_ERROR:
        return FNMATCH_ERROR;
      default:
        state = fnmatch_context_match( &(scanner->context) );
        break;
    }
  } while( state != FNMATCH_STOP );
  
  return state;
}

void fnmatch_scanner_reset( fnmatch_scanner_t* scanner ) {
  fnmatch_context_reset( &(scanner->context) );
}
