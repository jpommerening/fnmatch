#define _IN_FNMATCH_
#include "fnmatch.h"
#include "internal.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

static void fnmatch__compiler_append( fnmatch_pattern_t* pattern, const char* data, size_t length ) {
  size_t avail = pattern->alloc - pattern->proglen;
  if( avail < length ) {
    FNMATCH_GROW( pattern->program, length, &(pattern->alloc) );
  }
  FNMATCH_CPY( pattern->program, pattern->proglen, data, 0, length );
  pattern->proglen += length;
}

/**
 * @brief Start an operation. Store the opcode and reserve a byte
 * for parameter length.
 * @param pattern the pattern to write to.
 * @param opcode  the opcode to write.
 */
static void fnmatch__compiler_opcode( fnmatch_pattern_t* pattern, fnmatch_opcode_t opcode ) {
  char word[2] = { 0, 0 };
  word[0] = opcode;
  fnmatch__compiler_append( pattern, &(word[0]), 2 );
  printf( "Push opcode %i\n", opcode );
}

static int character_in_set( char c, const char* set, size_t length ) {
  while( length > 0 )
    if( c == set[--length] ) return 1;
  return 0;
}

static size_t fnmatch__compiler_oparg(
  fnmatch_pattern_t* pattern, char c,
  const char* data,  size_t length,
  const char* delim, size_t ndelim,
  const char* esc,   size_t nesc ) {
  
  size_t i, j, start;
  start = pattern->proglen;
  
  for( i=j=0; (i<length) && !character_in_set( data[i], delim, ndelim ); i++ ) {
    if( data[i] == c && character_in_set(data[i+1], esc, nesc ) ) {
      fnmatch__compiler_append( pattern, data+j, i-j );
      j = ++i;
    }
  }
  fnmatch__compiler_append( pattern, data+j, i-j );
  if( pattern->proglen != start ) {
    printf( "Push oparg %i:%s\n", (int) (pattern->proglen - start), &(pattern->program[start]) );
    pattern->program[start-1] = (char) (pattern->proglen - start);
    fnmatch__compiler_append( pattern, &(pattern->program[start-1]), 1 );
  }

  return i;
}

/**
 * @brief Compile unary operations.
 * Handles ?, *, **, /
 * @param pattern the pattern to write to.
 * @param expr the input string.
 * @return the number of bytes read from the input string.
 */
static size_t fnmatch__compiler_unary( fnmatch_pattern_t* pattern, const char* expr ) {
  if( expr[0] == FNMATCH_ONE ) {
      fnmatch__compiler_opcode( pattern, FNMATCH_OP_ONE );
      return 1;
  } else if( expr[0] == FNMATCH_ANY ) {
    if( expr[1] == FNMATCH_DEEP ) {
      fnmatch__compiler_opcode( pattern, FNMATCH_OP_DEEP );
      return 2;
    } else {
      fnmatch__compiler_opcode( pattern, FNMATCH_OP_ANY );
      return 1;
    }
  } else if( expr[0] == FNMATCH_SEP ) {
    fnmatch__compiler_opcode( pattern, FNMATCH_OP_SEP );
    return 1;
  } else if( expr[0] == '\0' ) {
    fnmatch__compiler_opcode( pattern, FNMATCH_OP_END );
    return 0;
  }
  return 0;
}

/**
 * Compile character sets and ranges.
 * Special characters "[]" must be escaped.
 * @param buf  the buffer to write to.
 * @param expr the input string.
 * @return the number of bytes read from the input string.
 */
static size_t fnmatch__compiler_chars( fnmatch_pattern_t* pattern, const char* expr ) {
  static const char del[2] = { FNMATCH_CHARS_END, '\0' };
  static const char esc[4] = { FNMATCH_CHARS_START, FNMATCH_CHARS_END, FNMATCH_ESCAPE };
  size_t length;
  
  assert( expr[0] == FNMATCH_CHARS_START );
  fnmatch__compiler_opcode( pattern, FNMATCH_OP_CHARS );
  length = fnmatch__compiler_oparg( pattern, FNMATCH_ESCAPE, expr+1, 127, del, 2, esc, 3 );
  assert( expr[length+1] == FNMATCH_CHARS_END );
  return length + 2;
}

/**
 * Compile fixed strings.
 * Special characters "?*[/" must be escaped.
 * @param buf  the buffer to write to.
 * @param expr the input string.
 * @return the number of bytes read from the input string.
 */
static size_t fnmatch__compiler_fixed( fnmatch_pattern_t* pattern, const char* expr ) {
  static const char del[5] = { FNMATCH_ONE, FNMATCH_ANY, FNMATCH_SEP, FNMATCH_CHARS_START, '\0' };
  static const char esc[5] = { FNMATCH_ONE, FNMATCH_ANY, FNMATCH_SEP, FNMATCH_CHARS_START, FNMATCH_ESCAPE };
  size_t length;
  
  fnmatch__compiler_opcode( pattern, FNMATCH_OP_FIXED );
  length = fnmatch__compiler_oparg( pattern, FNMATCH_ESCAPE, expr, 127, del, 5, esc, 5 );
  return length;
}

/**
 * Compile an expression and store the compiled program inside the pattern struct.
 * @param pattern the pattern to compile to.
 * @param expr the expression to compile.
 */
fnmatch_state_t fnmatch_compile( fnmatch_pattern_t* pattern ) {
  const char* expr = pattern->pattern;
  
  size_t length = 0;
  size_t mchars = 0;
  size_t groups = 0;
  size_t parts  = 0;
  
  if( pattern->program ) free( pattern->program );
  pattern->program = NULL;
  pattern->proglen = 0;
  pattern->alloc   = 0;
  pattern->groups  = 0;
  pattern->mchars  = 0;
  pattern->parts   = 0;
  
  do {
    expr += length;
    switch( *expr ) {
      case FNMATCH_SEP:
        length  = fnmatch__compiler_unary( pattern, expr );
        mchars += 1;
        parts  += 1;
        break;
      case FNMATCH_ONE:
        mchars += 1;
      case FNMATCH_ANY:
    /*case FNMATCH_DEEP:*/
        length  = fnmatch__compiler_unary( pattern, expr );
        groups += 1;
        break;
      case FNMATCH_CHARS_START:
        length  = fnmatch__compiler_chars( pattern, expr );
        mchars += 1;
        groups += 1;
        break;
      default:
        length  = fnmatch__compiler_fixed( pattern, expr );
        mchars += length;
        break;
      case '\0':
        length  = fnmatch__compiler_unary( pattern, expr );
        break;
    }
  } while( *expr != '\0' );

  pattern->mchars = mchars;
  pattern->groups = groups;
  pattern->parts  = parts;
  return FNMATCH_CONTINUE;
}
