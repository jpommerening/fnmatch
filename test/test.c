#include "test.h"
#include <stdio.h>
#include <stdarg.h>

void test_status( test_context_t* context, test_result_t result ) {
  context->result = (result > context->result) ? result : context->result;
}

void test_message( test_context_t* context, const char* file, int line, const char* fmt, ... ) {
  const test_t* test = context->test;
  FILE* fp = stdout;
  
  if( fmt[0] == '\0' ) return;
  
  if( test ) {
    fprintf( fp, "%s:%d: %s: ", file, line, test->name );
  } else {
    fprintf( fp, "%s:%d: ", file, line );
  }
  
  va_list vargs;
  va_start(vargs, fmt);
  vfprintf( fp, fmt, vargs );
  va_end(vargs);
}

static void test__start( test_context_t* context ) {
  const test_t* test = context->test;
/*const test_suite_t* suite = context->suite;*/
  const void* data = context->data;

  if( test ) {
    printf( "TEST %s", test->name );
    
    if( data ) {
      printf( " [%d/%d]",
        (int) ((data - test->data) / test->datastep) + 1,
        test->datalength );
    }
    
    printf( ":" );
  }
}

static void test__finish( test_context_t* context ) {
  static const char* str[TEST_RESULT_MAX] = {
    "PASS",
    "FAIL",
    "SKIP",
    "WARN",
    "ERROR"
  };
  context->total++;
  context->count[context->result]++;
  printf( " %s\n", str[context->result] );
}

static void test__run_void( test_context_t* context ) {
  const test_t* test = context->test;

  test__start( context );
  (test->callback.void_cb)( context );
  test__finish( context );
}

static void test__run_data( test_context_t* context ) {
  const test_t* test = context->test;
  int i;
  
  for( i=0; i<(test->datalength); i++ ) {
    context->data = (test->data) + (i*test->datastep);
    context->result = TEST_PASS;
    test__start( context );
    (test->callback.data_cb)( context, context->data );
    test__finish( context );
    context->data = NULL;
  }
}

static void test__run( test_context_t* context ) {
  if( context->test->datastep == 0 ) {
    test__run_void( context );
  } else {
    test__run_data( context );
  }
  return;
}

test_result_t test_run( const test_t* test ) {
  test_context_t context = {
    NULL,
    test,
    NULL,
    TEST_PASS
  };
  
  test__run( &context );
  return context.result;
}

test_result_t test_suite_run( const test_suite_t* suite ) {
  test_context_t context = {
    suite,
    NULL,
    NULL,
    TEST_PASS
  };
  int i;
  
  for( i=0; suite->tests[i] != NULL; i++ ) {
    context.test = suite->tests[i];
    context.result = TEST_PASS;
    test__run( &context );
  }
  
  printf( "RESULTS: %i/%i passed, %i failed, %i skipped\n",
    context.count[TEST_PASS],
    context.total,
    context.count[TEST_FAIL],
    context.count[TEST_SKIP]
  );
  
  if( context.count[TEST_FAIL] || context.count[TEST_SKIP] ) {
    context.result = TEST_FAIL;
  } else if( context.count[TEST_WARN] ) {
    printf( "WARNINGS: %i\n", context.count[TEST_WARN] );
    context.result = TEST_WARN;
  } else if( context.count[TEST_ERROR] ) {
    printf( "ERRORS: %i\n", context.count[TEST_ERROR] );
    context.result = TEST_ERROR;
  } else {
    context.result = TEST_PASS;
  }
  return context.result;
}

