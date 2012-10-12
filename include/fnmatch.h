/* -*- Mode: C; tab-width: 2; c-basic-offset: 2 -*- */
/* vim:set softtabstop=2 shiftwidth=2: */
/* 
 * Copyright (c) 2012, Jonas Pommerening <jonas.pommerening@gmail.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met: 
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer. 
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef _FNMATCH_H_
#define _FNMATCH_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <unistd.h>
#include <stddef.h>

#include <buffer.h>

#define FNMATCH_EXTERN extern

typedef enum {
  FNMATCH_NOMATCH,
  FNMATCH_MATCH,
  FNMATCH_PUSH,
  FNMATCH_POP,
  FNMATCH_CONTINUE,
  FNMATCH_STOP,
  FNMATCH_ERROR
} fnmatch_state_t;

typedef enum {
  FNMATCH_OP_FIXED,
  FNMATCH_OP_CHARS,
  FNMATCH_OP_ONE,
  FNMATCH_OP_ANY,
  FNMATCH_OP_DEEP,
  FNMATCH_OP_SEP,
  FNMATCH_OP_END
} fnmatch_opcode_t;
  
typedef struct fnmatch_stats_s   fnmatch_stats_t;
typedef struct fnmatch_pattern_s fnmatch_pattern_t;
typedef struct fnmatch_frame_s   fnmatch_frame_t;
typedef struct fnmatch_context_s fnmatch_context_t;
typedef struct fnmatch_scanner_s fnmatch_scanner_t;
typedef struct fnmatch_match_s   fnmatch_match_t;

typedef fnmatch_state_t (*fnmatch_push_cb)( fnmatch_context_t* ctx, void* info );
typedef fnmatch_state_t (*fnmatch_pop_cb)( fnmatch_context_t* ctx, void* info );
typedef fnmatch_state_t (*fnmatch_match_cb)( fnmatch_context_t* ctx, fnmatch_match_t* match, void* info );

struct fnmatch_stats_s {
  size_t mchars; /* minimum chars to match (sum of all fixed, sep, one, chars) */
  size_t groups; /* number of groups */
  size_t parts;  /* number of parts */
};
  
struct fnmatch_pattern_s {
  char*  pattern; /* original pattern expression */
  char*  program; /* compiled matching program */
  size_t proglen; /* number of bytes inside program */
  
  fnmatch_stats_t stats;
};

struct fnmatch_frame_s {
  size_t opptr;  /* offset of the current operation inside the program buffer */
  size_t offset; /* offset of the currently matched character */
};

struct fnmatch_context_s {
  fnmatch_pattern_t* pattern;
  buffer_t buffer;

  fnmatch_state_t  state;
  fnmatch_opcode_t opcode;
  fnmatch_frame_t  op;   /* current execution context */
  fnmatch_frame_t  any;  /* last occurance of FNMATCH_OP_ANY */
  fnmatch_frame_t  deep; /* last occurance of FNMATCH_OP_DEEP */

  int nmatch;   /* number of matching strings so far */
  int nnomatch; /* number of non-matching strings */
};

struct fnmatch_scanner_s {
  fnmatch_context_t context;
  fnmatch_push_cb   push_cb;
  fnmatch_pop_cb    pop_cb;
  fnmatch_match_cb  match_cb;
};

struct fnmatch_match_s {
  buffer_t buffer; /* ? */
  
  size_t argc;
  char*  argv[1];
};

/* MARK: - Basic pattern matching API *//**
 * @name Basic pattern matching API
 * Use this API if you just want to match simple strings.
 * @{
 */
FNMATCH_EXTERN void fnmatch_pattern_init( fnmatch_pattern_t* pattern );
FNMATCH_EXTERN void fnmatch_pattern_destroy( fnmatch_pattern_t* pattern );
FNMATCH_EXTERN fnmatch_state_t fnmatch_pattern_compile( fnmatch_pattern_t* pattern, const char* expr, int flags );
FNMATCH_EXTERN fnmatch_state_t fnmatch_pattern_match( fnmatch_pattern_t* pattern, const char* str );
FNMATCH_EXTERN fnmatch_state_t fnmatch_pattern_render( fnmatch_pattern_t* pattern, fnmatch_match_t* match );
/** @} */

/* MARK: - Resumeable API *//**
 * @name Resumeable API
 * Use this if you want to match sets (trees) of strings and push new data on demand.
 * This requires you to loop through multiple invocations of the match function until
 * it's done.
 * @{
 */
FNMATCH_EXTERN void fnmatch_context_init( fnmatch_context_t* context, fnmatch_pattern_t* pattern );
FNMATCH_EXTERN void fnmatch_context_destroy( fnmatch_context_t* context );
FNMATCH_EXTERN void fnmatch_context_reset( fnmatch_context_t* context );
FNMATCH_EXTERN fnmatch_state_t fnmatch_context_match( fnmatch_context_t* context );
FNMATCH_EXTERN void fnmatch_context_push( fnmatch_context_t* context, const char* str );
FNMATCH_EXTERN const char * fnmatch_context_pop( fnmatch_context_t* context );
/** @} */

/* MARK: - Callback driven API *//**
 * @name Callback driven API
 * If you don't want to match sets without iterating through a loop, use this API.
 * Interactions like pushing data to the stack are automated with callbacks.
 * @{
 */
FNMATCH_EXTERN void fnmatch_scanner_init( fnmatch_scanner_t* scanner, fnmatch_pattern_t* pattern,
  fnmatch_push_cb push_cb, fnmatch_pop_cb pop_cb, fnmatch_match_cb match_cb );
FNMATCH_EXTERN void fnmatch_scanner_destroy( fnmatch_scanner_t* scanner );
FNMATCH_EXTERN void fnmatch_scanner_reset( fnmatch_scanner_t* scanner );
FNMATCH_EXTERN fnmatch_state_t fnmatch_scanner_match( fnmatch_scanner_t* scanner, void* info );
/** @} */

/* MARK: - POSIX.2 API *//**
 * @name POSIX.2 API
 * This is the simplest possible form. Allows not much control..
 * @{
 */

/* Some systems #define these macros in <unistd.h> so we need to #undef them first */
#undef FNM_PATHNAME
#undef FNM_NOESCAPE
#undef FNM_PERIOD
#undef FNM_NOMATCH

/* The similarity to Linux is coincidental ;) */
#define FNM_PATHNAME (1 << 0)
#define FNM_NOESCAPE (1 << 1)
#define FNM_PERIOD   (1 << 2)

#define	FNM_NOMATCH 1
#define FNM_NOSYS  -1
FNMATCH_EXTERN int fnmatch( const char*, const char*, int );
/** @} */

#ifdef __cplusplus
}
#endif

#endif
