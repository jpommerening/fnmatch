#define _IN_FNMATCH_
#include "fnmatch.h"
#include "internal.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/* MARK: - Program access *//**
 * @name Program access
 * @cond INTERNALS
 * @{
 */
 
#define FNMATCHCTX_OPCODE(ctx) ((ctx)->pattern->program[(ctx)->opptr])
#define FNMATCHCTX_OPLEN(ctx)  ((ctx)->pattern->program[(ctx)->opptr+1])
#define FNMATCHCTX_OPRLEN(ctx) ((ctx)->pattern->program[(ctx)->opptr-1])
#define FNMATCHCTX_OPARG(ctx) &((ctx)->pattern->program[(ctx)->opptr+2])
#define FNMATCHCTX_STR(ctx)   &((ctx)->buffer[(ctx)->offset])
#define FNMATCHCTX_STEP(ctx,n) ((ctx)->offset += n )

/** @} */

/* MARK: - Markers *//**
 * @name Markers
 * @cond INTERNALS
 * @{
 */

static void fnmatch__vm_mark( fnmatch_context_t *context ) {
  context->mark_opptr  = context->opptr;
  context->mark_offset = context->offset + 1;
}

static void fnmatch__vm_unmark( fnmatch_context_t *context ) {
  context->mark_opptr  = 0;
  context->mark_offset = 0;
}

static fnmatch_state_t fnmatch__vm_restore( fnmatch_context_t *context ) {
  if( context->mark_offset == 0 )
    return FNMATCH_STOP;
    
  context->opptr  = context->mark_opptr;
  context->offset = context->mark_offset;
  
  fnmatch__vm_unmark( context );
  return FNMATCH_CONTINUE;
}

/** @} */

/* MARK: - Opcode implementation *//**
 * @name Opcode implementation
 * @cond INTERNALS
 * @{
 */

static fnmatch_state_t fnmatch__vm_cond( fnmatch_context_t *context, int condition, size_t chars ) {
  if( condition ) {
    FNMATCHCTX_STEP(context, chars);
    return FNMATCH_MATCH;
  } else {
    return FNMATCH_NOMATCH;
  }
}

static fnmatch_state_t fnmatch__vm_fixed( fnmatch_context_t *context, const char* str,
                                          size_t oplen, const char* oparg ) {
  if( (oplen + context->offset) > context->buflen ) {
    /* signal an early exit for wildcard matches? experimental .. */
    fnmatch__vm_unmark( context );
    return FNMATCH_NOMATCH;
  }
  return fnmatch__vm_cond( context, memcmp( str, oparg, oplen ) == 0, oplen );

}

static fnmatch_state_t fnmatch__vm_chars( fnmatch_context_t *context, const char* str,
                                          size_t oplen, const char* oparg ) {
  char c;
  size_t i = 0;

  if( oparg[0] == '!' ) i=1;

  for( ; i<oplen; i++ ) {
    c = oparg[i];
    
    if( i<(oplen-1) ) {
      if( c == FNMATCH_ESCAPE ) {
        i++;
        c = oparg[i];
      } else if( (c == '-') && (i > 0) ) {
        if( (str[0] > oparg[i-1]) &&
            (str[0] < oparg[i+1]) ) {
          return fnmatch__vm_cond( context, oparg[0] != FNMATCH_CHARS_NEGATE, 1 );
        }
      }
    }
    
    if( str[0] == oparg[i] ) {
      return fnmatch__vm_cond( context, oparg[0] != FNMATCH_CHARS_NEGATE, 1 );
    }
  }
  return fnmatch__vm_cond( context, oparg[0] == FNMATCH_CHARS_NEGATE, 1 );
}

static fnmatch_state_t fnmatch__vm_one( fnmatch_context_t *context, const char* str ) {
  return fnmatch__vm_cond( context, (str[0] != '\0') && (str[0] != FNMATCH_SEP), 1 );
}

static fnmatch_state_t fnmatch__vm_any( fnmatch_context_t *context, const char* str ) {
  if( (str[0] == '\0') || (str[0] == FNMATCH_SEP) ) {
    fnmatch__vm_unmark( context );
  } else {
    fnmatch__vm_mark( context );
  }
  return FNMATCH_MATCH;
}

static fnmatch_state_t fnmatch__vm_deep( fnmatch_context_t *context, const char* str ) {
  if( str[0] == '\0' ) {
    fnmatch__vm_unmark( context );
  } else {
    fnmatch__vm_mark( context );
  }
  return FNMATCH_MATCH;
}

static fnmatch_state_t fnmatch__vm_sep( fnmatch_context_t *context, const char* str ) {
  if( str[0] == FNMATCH_SEP ) {
    FNMATCHCTX_STEP(context, 1);
    
    if( context->pattern->program[context->mark_opptr] != FNMATCH_OP_DEEP ) {
      /* no turning back to an "*" if we're past a separator */
      fnmatch__vm_unmark( context );
    }
    return FNMATCH_MATCH;
  }
  return FNMATCH_NOMATCH;
}

static fnmatch_state_t fnmatch__vm_end( fnmatch_context_t *context, const char* str ) {
  if( str[0] == '\0' ) {
    FNMATCHCTX_STEP(context, 1);

    /* no turning back to "*" or "**" if we're finished */
    fnmatch__vm_unmark( context );
    return FNMATCH_MATCH;
  }
  return FNMATCH_NOMATCH;
}

/** @} */

/* MARK: - VM API *//**
 * @name VM API
 * @{
 */

fnmatch_state_t fnmatch_vm_next( fnmatch_context_t *context ) {
  size_t oplen;
  
  if( FNMATCHCTX_OPCODE(context) == FNMATCH_OP_END )
    return FNMATCH_STOP;
  if( context->opptr >= context->pattern->proglen )
    return FNMATCH_STOP;
  
  oplen = FNMATCHCTX_OPLEN(context);
  if( oplen ) context->opptr += oplen+3;
  else        context->opptr += 2;
  return FNMATCH_CONTINUE;
}

fnmatch_state_t fnmatch_vm_prev( fnmatch_context_t *context ) {
  size_t oplen;

  if( context->opptr <= 0 )
    return FNMATCH_STOP;
  
  oplen = FNMATCHCTX_OPRLEN(context);
  if( oplen ) context->opptr -= oplen+3;
  else        context->opptr -= 2;
  return FNMATCH_CONTINUE;
}

fnmatch_state_t fnmatch_vm_retry( fnmatch_context_t *context ) {
  if( context->opptr == 0 )
    return FNMATCH_STOP;
  
  return fnmatch__vm_restore( context );
}

fnmatch_state_t fnmatch_vm_rewind( fnmatch_context_t *context ) {
  if( fnmatch_vm_retry( context ) == FNMATCH_CONTINUE )
    return FNMATCH_CONTINUE;

  /* throw away buffer from end to last separator
     rewind program until last separator */

  while( fnmatch_vm_prev( context ) == FNMATCH_CONTINUE ) {
    if( FNMATCHCTX_OPCODE(context) == FNMATCH_OP_SEP ) break;
  }
  
  while( (context->offset) > 0 ) {
    context->offset--;
    if( *FNMATCHCTX_STR(context) == FNMATCH_SEP ) break;
  }
  context->buflen = context->offset;
  return FNMATCH_CONTINUE;
}

#include <stdio.h>

fnmatch_state_t fnmatch_vm_op( fnmatch_context_t *context ) {
  size_t oplen = FNMATCHCTX_OPLEN(context);
  char*  oparg = oplen > 0 ? FNMATCHCTX_OPARG(context) : NULL;
  char*  str   = FNMATCHCTX_STR(context);
  
  context->opcode = FNMATCHCTX_OPCODE(context);
  printf( "OPCODE %i ON %s\n", context->opcode, str );
  switch( context->opcode ) {
    case FNMATCH_OP_FIXED: return fnmatch__vm_fixed( context, str, oplen, oparg );
    case FNMATCH_OP_CHARS: return fnmatch__vm_chars( context, str, oplen, oparg );
    case FNMATCH_OP_ONE:   return fnmatch__vm_one( context, str );
    case FNMATCH_OP_ANY:   return fnmatch__vm_any( context, str );
    case FNMATCH_OP_DEEP:  return fnmatch__vm_deep( context, str );
    case FNMATCH_OP_SEP:   return fnmatch__vm_sep( context, str );
    case FNMATCH_OP_END:   return fnmatch__vm_end( context, str );
    default:
      return FNMATCH_ERROR;
  }
  
  assert( 0 ); /* unreachable */
}

/** @} */
