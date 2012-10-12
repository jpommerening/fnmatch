#define _IN_FNMATCH_
#include "fnmatch.h"
#include "internal.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

/**
 * @brief Start an operation. Store the opcode and reserve a byte
 * for parameter length.
 * @param buffer the buffer to write to.
 * @param opcode  the opcode to write.
 */
static void fnmatch__compiler_opcode( buffer_t* buffer, fnmatch_opcode_t opcode ) {
  char word[2] = { 0, 0 };
  word[0] = opcode;
  buffer_append( buffer, &(word[0]), 2 );
  /*printf( "Push opcode %i\n", opcode );*/
}

/**
 * @brief Append an optional argument to the previous operation.
 * @return the number of bytes read.
 */
static size_t fnmatch__compiler_oparg(
  buffer_t* buffer, char c,
  const char* data,  size_t len,
  const char* delim, size_t ndelim,
  const char* esc,   size_t nesc ) {

  size_t read, written;
  size_t start = buffer->length;
  
  read    = buffer_read_escaped( buffer, c, data, len, delim, ndelim, esc, nesc ),
  written = buffer->length - start;
  
  if( written ) {
    /*printf( "Push oparg %i:%s\n", (int) (pattern->proglen - start), &(pattern->program[start]) );*/
    buffer->data[start-1] = (char)written;
    buffer_appendc( buffer, (char)written );
  }
  
  return read;
}

/**
 * @brief Compile unary operations.
 * Handles ?, *, **, /
 * @param buffer the buffer to write to.
 * @param expr the input string.
 * @return the number of bytes read from the input string.
 */
static size_t fnmatch__compiler_unary( buffer_t* buffer, const char* expr ) {
  if( expr[0] == FNMATCH_ONE ) {
      fnmatch__compiler_opcode( buffer, FNMATCH_OP_ONE );
      return 1;
  } else if( expr[0] == FNMATCH_ANY ) {
    if( expr[1] == FNMATCH_DEEP ) {
      fnmatch__compiler_opcode( buffer, FNMATCH_OP_DEEP );
      return 2;
    } else {
      fnmatch__compiler_opcode( buffer, FNMATCH_OP_ANY );
      return 1;
    }
  } else if( expr[0] == FNMATCH_SEP ) {
    fnmatch__compiler_opcode( buffer, FNMATCH_OP_SEP );
    return 1;
  } else if( expr[0] == '\0' ) {
    fnmatch__compiler_opcode( buffer, FNMATCH_OP_END );
    return 0;
  }
  return 0;
}

/**
 * Compile character sets and ranges.
 * Special characters "[]" must be escaped.
 * @param buffer the buffer to write to.
 * @param expr the input string.
 * @return the number of bytes read from the input string.
 */
static size_t fnmatch__compiler_chars( buffer_t* buffer, const char* expr ) {
  static const char del[2] = { FNMATCH_CHARS_END, '\0' };
  static const char esc[4] = { FNMATCH_CHARS_START, FNMATCH_CHARS_END, FNMATCH_ESCAPE };
  size_t read;
  
  assert( expr[0] == FNMATCH_CHARS_START );
  fnmatch__compiler_opcode( buffer, FNMATCH_OP_CHARS );
  read = fnmatch__compiler_oparg( buffer, FNMATCH_ESCAPE, expr+1, 127, del, 2, esc, 3 );
  assert( expr[read+1] == FNMATCH_CHARS_END );
  return read + 2;
}

/**
 * Compile fixed strings.
 * Special characters "?*[/" must be escaped.
 * @param buffer the buffer to write to.
 * @param expr the input string.
 * @return the number of bytes read from the input string.
 */
static size_t fnmatch__compiler_fixed( buffer_t* buffer, const char* expr ) {
  static const char del[5] = { FNMATCH_ONE, FNMATCH_ANY, FNMATCH_SEP, FNMATCH_CHARS_START, '\0' };
  static const char esc[5] = { FNMATCH_ONE, FNMATCH_ANY, FNMATCH_SEP, FNMATCH_CHARS_START, FNMATCH_ESCAPE };
  size_t read;
  
  fnmatch__compiler_opcode( buffer, FNMATCH_OP_FIXED );
  read = fnmatch__compiler_oparg( buffer, FNMATCH_ESCAPE, expr, 127, del, 5, esc, 5 );
  return read;
}

/**
 * Compile an expression and store the compiled program inside the pattern struct.
 * @param expr the expression to compile.
 * @param flags some flags for compilation.
 * @param length an optional pointer to store the length of the pattern.
 * @param stats an optional pointer to store the metrics of the pattern.
 * @return the compiled pattern program.
 */
void* fnmatch_compile( const char* expr, int flags, size_t* length, fnmatch_stats_t* stats ) {
  buffer_t buffer = BUFFER_INIT;
  
  size_t read   = 0;
  size_t mchars = 0;
  size_t groups = 0;
  size_t parts  = 0;
  
  do {
    expr += read;
    switch( *expr ) {
      case FNMATCH_SEP:
        read    = fnmatch__compiler_unary( &buffer, expr );
        mchars += 1;
        parts  += 1;
        break;
      case FNMATCH_ONE:
        mchars += 1;
      case FNMATCH_ANY:
    /*case FNMATCH_DEEP:*/
        read    = fnmatch__compiler_unary( &buffer, expr );
        groups += 1;
        break;
      case FNMATCH_CHARS_START:
        read    = fnmatch__compiler_chars( &buffer, expr );
        mchars += 1;
        groups += 1;
        break;
      default:
        read    = fnmatch__compiler_fixed( &buffer, expr );
        mchars += read;
        break;
      case '\0':
        read    = fnmatch__compiler_unary( &buffer, expr );
        break;
    }
  } while( *expr != '\0' );

  if( stats != NULL ) {
    stats->mchars = mchars;
    stats->groups = groups;
    stats->parts  = parts;
  }
  
  return buffer_detach( &buffer, length );
}
