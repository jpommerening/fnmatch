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
  
  context->pattern = pattern;
  FNMATCH_ALLOC( context->buffer, pattern->mchars, &(context->alloc) );
  context->buflen = 0;
  
  fnmatch_context_reset( context );
}

void fnmatch_context_destroy( fnmatch_context_t* context ) {
  assert( context );
  free( context->buffer );
}

fnmatch_state_t fnmatch_context_match( fnmatch_context_t* context ) {
  assert( context );


  switch( context->state ) {
    case FNMATCH_MATCH:
      if( context->opcode == FNMATCH_OP_END ) {
        context->match++;
        context->state = FNMATCH_POP;
      } else if( context->opcode == FNMATCH_OP_SEP ) {
        context->state = fnmatch_vm_next( context );
      } else if( context->opcode == FNMATCH_OP_DEEP ) {
        context->state = fnmatch_vm_next( context );
      } else {
        context->state = FNMATCH_ERROR; /*fnmatch_vm_next( context );*/
      }
      break;
    case FNMATCH_NOMATCH:
      context->state = FNMATCH_POP;
      break;
    case FNMATCH_PUSH:
    case FNMATCH_POP:
      context->state = FNMATCH_STOP;
      break;
    case FNMATCH_CONTINUE:
fnmatch_context_match_continue:
      if( context->offset >= context->buflen || context->buflen == 0 ) {
        context->state = FNMATCH_PUSH;
      } else {
        context->state = fnmatch_vm_op( context );
      }
      break;
    case FNMATCH_STOP:
      break;
    case FNMATCH_ERROR:
      break;
  }
  
  /* clean this up */
  if( FNMATCH_MATCH == context->state ) {
    if( FNMATCH_OP_DEEP == context->opcode &&
        FNMATCH_SEP == context->buffer[context->offset] ) {
      /* return match */
    } else if( FNMATCH_OP_END != context->opcode &&
               FNMATCH_OP_SEP != context->opcode ) {
      context->state = fnmatch_vm_next( context );
    }
  } else if( FNMATCH_NOMATCH  == context->state &&
             FNMATCH_CONTINUE == fnmatch_vm_retry( context ) ) {
    context->state = FNMATCH_CONTINUE;
  }

  /* Yeah I know; but this time it's fine. Trust me. */
  if( context->state == FNMATCH_CONTINUE )
    goto fnmatch_context_match_continue;

  return context->state;
}

void fnmatch_context_reset( fnmatch_context_t* context ) {
  assert( context );
  context->state  = FNMATCH_PUSH;
  context->match  = 0;
  context->opptr  = 0;
  context->offset = 0;
  context->mark_opptr  = 0;
  context->mark_offset = 0;
}

void fnmatch_context_push( fnmatch_context_t* context, const char* str ) {
  size_t length;

  assert( context );

  if( context->state != FNMATCH_PUSH ) {
    context->state = FNMATCH_ERROR;
    return;
  }

  length = str ? strlen( str ) : 0;
  if( length > 0 ) {
    context->state = FNMATCH_CONTINUE;
    FNMATCH_GROW( context->buffer, context->buflen+length+1, &(context->alloc) );
    memcpy( &(context->buffer[context->buflen]), str, length+1 );
    context->buflen += length;
  } else if( context->buffer[context->buflen] == '\0' ) {
    /* expose the implicit '\0' to the matcher */
    context->buflen += 1;
    context->state = FNMATCH_CONTINUE;
  } else {
    context->state = FNMATCH_STOP;
  }
}

const char * fnmatch_context_pop( fnmatch_context_t* context ) {
  size_t offset = context->offset;

  assert( context );

  if( context->state != FNMATCH_POP ) {
    context->state = FNMATCH_ERROR;
    return NULL;
  }

  fnmatch_vm_rewind( context );

           /* what was the meaning of this? */
  if( 1 || (context->offset < offset) ) {
    context->state = FNMATCH_CONTINUE;
    return &(context->buffer[offset]);
  } else {
    context->state = FNMATCH_STOP;
    return NULL;
  }
}

