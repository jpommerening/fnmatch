#include "fnmatch.h"
#include "test.h"
#include <string.h>

typedef struct test_scanner_data_s test_scanner_data_t;
typedef struct test_scanner_count_s test_scanner_count_t;

struct test_scanner_count_s {
  const test_scanner_data_t* data;
  int push, pop, match;
};

struct test_scanner_data_s {
  const char* expr;
  const char* push[20];
  const char* pop[10];
  struct {
    int push, pop, match;
  } count;
};

static const test_scanner_data_t _data[] = {
  { "test/*.[abc]",
    { "test/", "test.a", NULL,
               "test.b", NULL,
               NULL,
      NULL },
    { "test.a", "test.b", "test/" },
    { 7, 4, 3 } },

  { "**.[ch]",
    { "src/", "one.c", NULL,
              "two.c", NULL,
              "internal/", "three.c", NULL,
                           NULL,
              "four.c", NULL,
               NULL,
      "include/", "test.h", NULL,
                  NULL,
      NULL },
    { "one.c", "two.c", "three.c", "internal/", "four.c", "src/",
      "test.h", "include/" },
    { 17, 9, 8 } }
};

static fnmatch_state_t test_push_cb( fnmatch_context_t* context, void* info ) {
  test_scanner_count_t* count = info;
  const test_scanner_data_t* data = count->data;
  
  if( count->push < data->count.push ) {
    fnmatch_context_push( context, data->push[count->push] );
  }
  count->push++;
  return FNMATCH_CONTINUE;
}

static fnmatch_state_t test_pop_cb( fnmatch_context_t* context, void* info ) {
  test_scanner_count_t* count = info;
  const test_scanner_data_t* data = count->data;
  const char* str;
  
  /* note, if we "pop" the empty string from the stack,
   * the scanner will (rightfully) want to continue with a new string */
  if( count->pop < data->count.pop -1 ) {
    str = fnmatch_context_pop( context );
  }
  count->pop++;
  return FNMATCH_CONTINUE;
}

static fnmatch_state_t test_match_cb( fnmatch_context_t* context, fnmatch_match_t* match, void* info ) {
  test_scanner_count_t* count = info;
  
  count->match++;
  return FNMATCH_CONTINUE;
}

TEST( test_scanner, _data, const test_scanner_data_t* data ) {
  fnmatch_pattern_t pattern;
  fnmatch_scanner_t scanner;
  
  test_scanner_count_t count = { data, 0, 0, 0 };
  
  fnmatch_pattern_init( &pattern );
  ASSERTEQ( FNMATCH_CONTINUE, fnmatch_pattern_compile( &pattern, data->expr ) );
  fnmatch_scanner_init( &scanner, &pattern, &test_push_cb, &test_pop_cb, &test_match_cb );
  
  fnmatch_scanner_match( &scanner, &count );
    
  ASSERTEQ( count.push, data->count.push );
  ASSERTEQ( count.pop, data->count.pop );
  ASSERTEQ( count.match, data->count.match );
  
  fnmatch_scanner_destroy( &scanner );
  fnmatch_pattern_destroy( &pattern );
}