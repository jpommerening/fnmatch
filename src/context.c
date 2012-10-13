#define _IN_FNMATCH_
#include "fnmatch.h"
#include "internal.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

void fnmatch_context_init( fnmatch_context_t* context, fnmatch_pattern_t* pattern ) {
  assert( context );
  assert( pattern );
  
  buffer_init( &(context->buffer), pattern->stats.mchars );
  context->pattern = pattern;
  
  fnmatch_context_reset( context );
}

void fnmatch_context_destroy( fnmatch_context_t* context ) {
  assert( context );
  
  buffer_destroy( &(context->buffer) );
}

fnmatch_state_t fnmatch_context_match( fnmatch_context_t* context ) {
  assert( context );
  
  switch( context->state ) {
    case FNMATCH_MATCH:
      if( context->opcode == FNMATCH_OP_END )
        context->state = FNMATCH_POP;
      else
        context->state = fnmatch_vm_next( context );
      break;
    case FNMATCH_NOMATCH:
      context->state = FNMATCH_POP;
      break;
    case FNMATCH_PUSH:
    case FNMATCH_POP:
      context->state = FNMATCH_STOP;
      break;
    default:
      break;
  }

  while( context->state == FNMATCH_CONTINUE ) {
    if( context->op.offset >= context->buffer.length ) {
      context->state = FNMATCH_PUSH;
    } else {
      context->state = fnmatch_vm_op( context );
    
      if( FNMATCH_MATCH == context->state ) {
        
        if( FNMATCH_OP_END == context->opcode ) {
          context->nmatch++;
          /* return (full) match */
        } else if( FNMATCH_OP_SEP == context->opcode ||
                 ( FNMATCH_OP_DEEP == context->opcode &&
                   FNMATCH_SEP == context->buffer.data[context->op.offset] ) ) {
          /* return (partial) match */
        } else {
          context->state = fnmatch_vm_next( context );
        }
      } else {
        assert( context->state == FNMATCH_NOMATCH );
        
        if( FNMATCH_CONTINUE == fnmatch_vm_retry( context ) ) {
          context->state = FNMATCH_CONTINUE;
        } else {
          context->nnomatch++;
          /* return nomatch */
        }
      }
    }
  }
  
  return context->state;
}

void fnmatch_context_reset( fnmatch_context_t* context ) {
  assert( context );
  context->state  = FNMATCH_PUSH;
  context->opcode = FNMATCH_OP_END;
  context->op.opptr    = 0;
  context->op.offset   = 0;
  context->any.opptr   = 0;
  context->any.offset  = 0;
  context->deep.opptr  = 0;
  context->deep.offset = 0;

  context->nmatch   = 0;
  context->nnomatch = 0;
}

void fnmatch_context_push( fnmatch_context_t* context, const char* str ) {
  size_t length;

  assert( context );

  if( !(context->state == FNMATCH_PUSH || context->state == FNMATCH_CONTINUE) ) {
    context->state = FNMATCH_ERROR;
    return;
  }

  length = (str != NULL) ? strlen( str ) : 0;
  if( length > 0 ) {
    buffer_append( &(context->buffer), str, length );
  } else {
    buffer_appendc( &(context->buffer), '\0' );
  }
  context->state = FNMATCH_CONTINUE;
}

const char * fnmatch_context_pop( fnmatch_context_t* context ) {
  assert( context );

  if( context->state != FNMATCH_POP ) {
    context->state = FNMATCH_ERROR;
    return NULL;
  }

  fnmatch_vm_rewind( context );
  context->state = FNMATCH_CONTINUE;
    
  return &(context->buffer.data[context->op.offset]);
}

