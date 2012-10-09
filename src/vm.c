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
 
#define FNMATCHCTX_OPCODE(ctx) ((ctx)->pattern->program[(ctx)->op.opptr])
#define FNMATCHCTX_OPLEN(ctx)  ((ctx)->pattern->program[(ctx)->op.opptr+1])
#define FNMATCHCTX_OPRLEN(ctx) ((ctx)->pattern->program[(ctx)->op.opptr-1])
#define FNMATCHCTX_OPARG(ctx) &((ctx)->pattern->program[(ctx)->op.opptr+2])
#define FNMATCHCTX_STR(ctx)   &((ctx)->buffer.data[(ctx)->op.offset])
#define FNMATCHCTX_STEP(ctx,n) ((ctx)->op.offset += n )

/** @} */

/* MARK: - Markers *//**
 * @name Markers
 * @cond INTERNALS
 * @{
 */

static void fnmatch__vm_mark_any( fnmatch_context_t* context ) {
  if( context->any.offset == 0 ) {
    context->any.opptr  = context->op.opptr;
    context->any.offset = context->op.offset + 1;
  }
}

static void fnmatch__vm_unmark_any( fnmatch_context_t* context ) {
  context->any.opptr  = 0;
  context->any.offset = 0;
}

static void fnmatch__vm_mark_deep( fnmatch_context_t* context ) {
  if( context->deep.offset == 0 ) {
    context->deep.opptr  = context->op.opptr;
    context->deep.offset = context->op.offset + 1;
  }
}

static void fnmatch__vm_unmark_deep( fnmatch_context_t* context ) {
  context->deep.opptr  = 0;
  context->deep.offset = 0;
}

static fnmatch_state_t fnmatch__vm_restore( fnmatch_context_t* context ) {
  if( context->any.offset == 0 && context->deep.offset == 0 )
    return FNMATCH_STOP;

  if( context->any.opptr > context->deep.opptr ) {
    context->op.opptr  = context->any.opptr;
    context->op.offset = context->any.offset;
    fnmatch__vm_unmark_any( context );
  } else {
    context->op.opptr  = context->deep.opptr;
    context->op.offset = context->deep.offset;
    fnmatch__vm_unmark_deep( context );
  }
  
  return FNMATCH_CONTINUE;
}

/** @} */

/* MARK: - Opcode implementation *//**
 * @name Opcode implementation
 * @cond INTERNALS
 * @{
 */

static fnmatch_state_t fnmatch__vm_cond( fnmatch_context_t* context, int condition, size_t chars ) {
  if( condition ) {
    FNMATCHCTX_STEP(context, chars);
    return FNMATCH_MATCH;
  } else {
    return FNMATCH_NOMATCH;
  }
}

static fnmatch_state_t fnmatch__vm_fixed( fnmatch_context_t* context, const char* str,
                                          size_t oplen, const char* oparg ) {
  if( (oplen + context->op.offset) > context->buffer.length ) {
    /* signal an early exit for "*" matches? experimental .. */
    fnmatch__vm_unmark_any( context );
    return FNMATCH_NOMATCH;
  }
  return fnmatch__vm_cond( context, memcmp( str, oparg, oplen ) == 0, oplen );

}

static fnmatch_state_t fnmatch__vm_chars( fnmatch_context_t* context, const char* str,
                                          size_t oplen, const char* oparg ) {
  char c, neg = 0;
  size_t i = 0;

  if( oplen && oparg[0] == FNMATCH_CHARS_NEGATE ) {
    i   = 1;
    neg = 1;
  }

  for( ; i<oplen; i++ ) {
    c = oparg[i];
    
    if( i<(oplen-1) ) {
      if( c == FNMATCH_ESCAPE ) {
        i++;
      } else if( (c == '-') && (i > 0) ) {
        if( (str[0] > oparg[i-1]) &&
            (str[0] < oparg[i+1]) ) {
          return fnmatch__vm_cond( context, !neg, 1 );
        }
      }
    }
    
    if( str[0] == oparg[i] ) {
      return fnmatch__vm_cond( context, !neg, 1 );
    }
  }
  return fnmatch__vm_cond( context, neg, 1 );
}

static fnmatch_state_t fnmatch__vm_one( fnmatch_context_t* context, const char* str ) {
  return fnmatch__vm_cond( context, (str[0] != '\0') && (str[0] != FNMATCH_SEP), 1 );
}

static fnmatch_state_t fnmatch__vm_any( fnmatch_context_t* context, const char* str ) {
  if( str[0] == '\0' && context->op.offset < context->buffer.length ) {
    fnmatch__vm_unmark_any( context );
    fnmatch__vm_unmark_deep( context );
  } else if( (str[0] == FNMATCH_SEP) ) {
    fnmatch__vm_unmark_any( context );
  } else {
    fnmatch__vm_mark_any( context );
  }
  return FNMATCH_MATCH;
}

static fnmatch_state_t fnmatch__vm_deep( fnmatch_context_t* context, const char* str ) {
  if( str[0] == '\0' && context->op.offset < context->buffer.length ) {
    fnmatch__vm_unmark_deep( context );
  } else {
    fnmatch__vm_mark_deep( context );
  }
  return FNMATCH_MATCH;
}

static fnmatch_state_t fnmatch__vm_sep( fnmatch_context_t* context, const char* str ) {
  if( str[0] == FNMATCH_SEP ) {
    FNMATCHCTX_STEP(context, 1);
    
    if( context->any.opptr >= context->deep.opptr ) {
      /* no turning back to an "*" if we're past a separator */
      fnmatch__vm_unmark_any( context );
    }
    return FNMATCH_MATCH;
  }
  return FNMATCH_NOMATCH;
}

static fnmatch_state_t fnmatch__vm_end( fnmatch_context_t* context, const char* str ) {
  if( str[0] == '\0' && context->op.offset < context->buffer.length ) {
    FNMATCHCTX_STEP(context, 1);

    /* no turning back to "*" or "**" if we're finished */
    fnmatch__vm_unmark_any( context );
    fnmatch__vm_unmark_deep( context );
    return FNMATCH_MATCH;
  }
  return FNMATCH_NOMATCH;
}

/** @} */

/* MARK: - VM API *//**
 * @name VM API
 * @{
 */

fnmatch_state_t fnmatch_vm_next( fnmatch_context_t* context ) {
  size_t oplen;
  
  if( FNMATCHCTX_OPCODE(context) == FNMATCH_OP_END )
    return FNMATCH_STOP;
  if( context->op.opptr >= context->pattern->proglen )
    return FNMATCH_STOP;
  
  oplen = FNMATCHCTX_OPLEN(context);
  if( oplen ) context->op.opptr += oplen+3;
  else        context->op.opptr += 2;
  return FNMATCH_CONTINUE;
}

fnmatch_state_t fnmatch_vm_prev( fnmatch_context_t* context ) {
  size_t oplen;

  if( context->op.opptr <= 0 )
    return FNMATCH_STOP;
  
  oplen = FNMATCHCTX_OPRLEN(context);
  if( oplen ) context->op.opptr -= oplen+3;
  else        context->op.opptr -= 2;
  return FNMATCH_CONTINUE;
}

fnmatch_state_t fnmatch_vm_retry( fnmatch_context_t* context ) {
  if( context->op.opptr == 0 )
    return FNMATCH_STOP;
  
  return fnmatch__vm_restore( context );
}

fnmatch_state_t fnmatch_vm_rewind( fnmatch_context_t *context ) {
  if( fnmatch_vm_retry( context ) == FNMATCH_CONTINUE )
    return FNMATCH_CONTINUE;

  /* throw away buffer from end to last separator */

  /* clean me up */
  
  if( *FNMATCHCTX_STR(context) == '\0' )
    context->op.offset--;
  if( *FNMATCHCTX_STR(context) == FNMATCH_SEP )
    context->op.offset--;
  
  while( (context->op.offset) > 0 ) {
    if( *FNMATCHCTX_STR(context) == FNMATCH_SEP ) {
      context->op.offset++;
      break;
    }
    context->op.offset--;
  }
  
  /* rewind the program accordingly */
  while( fnmatch_vm_prev( context ) == FNMATCH_CONTINUE ) {
    /* there's an error around here */
    if( FNMATCHCTX_OPCODE(context) == FNMATCH_OP_DEEP &&
        context->deep.offset < context->op.offset )
      break;
    if( FNMATCHCTX_OPCODE(context) == FNMATCH_OP_SEP ) {
      fnmatch_vm_next( context );
      break;
    }
  }
  
  /* we don't use buffer_setlen() here because we do not want to
     put a \0 at the beginning of the string that is returned by
     fnmatch_pop(). We do not need that \0 yet anyway. :) */
  context->buffer.length = context->op.offset;
  return FNMATCH_CONTINUE;
}

/*#include <stdio.h>*/

fnmatch_state_t fnmatch_vm_op( fnmatch_context_t* context ) {
  size_t oplen = FNMATCHCTX_OPLEN(context);
  char*  oparg = oplen > 0 ? FNMATCHCTX_OPARG(context) : NULL;
  char*  str   = FNMATCHCTX_STR(context);
  
  context->opcode = FNMATCHCTX_OPCODE(context);
  /*printf( "OPCODE %i ON %s\n", context->opcode, str );*/
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
