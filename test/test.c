#include "test.h"
#include <stdio.h>
#include <stdarg.h>
#include <sys/time.h>

struct test_context_s {
  const test_suite_t* suite;
  const test_t* test;
  const void* data;
  test_result_t result;
  FILE* log;
  struct timeval start, finish;
  int msg, total, count[TEST_RESULT_MAX];
};

void test_status( test_context_t* context, test_result_t result ) {
  context->result = (result > context->result) ? result : context->result;
}

void test_message( test_context_t* context, const char* file, int line, const char* fmt, ... ) {
  const test_t* test = context->test;
  
  if( fmt[0] == '\0' ) return;
  
  if( context->msg == 0 )
    fprintf( context->log, " ...\n" );
  
  if( test ) {
    fprintf( context->log, "%s:%d: %s: ", file, line, test->name );
  } else {
    fprintf( context->log, "%s:%d: ", file, line );
  }
  
  context->msg++;
  
  va_list vargs;
  va_start(vargs, fmt);
  vfprintf( context->log, fmt, vargs );
  va_end(vargs);
}

static void test__start( test_context_t* context ) {
  const test_t* test = context->test;
/*const test_suite_t* suite = context->suite;*/
  const void* data = context->data;
  
  if( test ) {
    fprintf( context->log, "TEST %s", test->name );
    
    if( data ) {
      fprintf( context->log, " [%d/%d]",
        (int) ((data - test->data) / test->datastep) + 1,
        test->datalength );
    }
    
    fprintf( context->log, ":" );
  }
  
  gettimeofday( &(context->start), NULL );
}

static void test__finish( test_context_t* context ) {
  static const char* str[TEST_RESULT_MAX] = {
    "PASS",
    "FAIL",
    "SKIP",
    "WARN",
    "ERROR"
  };
  double diff;
  
  gettimeofday( &(context->finish), NULL );
  
  diff = ((context->finish.tv_sec - context->start.tv_sec) * 1000)
       + ((context->finish.tv_usec - context->start.tv_usec) * 0.001);
  
  context->total++;
  context->count[context->result]++;
  if( context->msg == 0 ) {
    fprintf( context->log, " %s (%.3lfms)\n", str[context->result], diff );
  } else {
    context->msg = 0;
    fprintf( context->log, " ... %s (%.3lfms)\n", str[context->result], diff );
  }
}

static void test__run_void( test_context_t* context ) {
  const test_t* test = context->test;

  test__start( context );
  (test->callback.void_cb)( context );
  test__finish( context );
}

static void test__run_data( test_context_t* context, int i ) {
  const test_t* test = context->test;
   
  if( i == -1 ) {
    for( i=0; i<(test->datalength); i++ ) {
      test__run_data( context, i );
    }
  } else {
    context->data = (test->data) + (i*test->datastep);
    context->result = TEST_PASS;
    test__start( context );
    (test->callback.data_cb)( context, context->data );
    test__finish( context );
    context->data = NULL;
  }
}

static void test__run( test_context_t* context ) {
  int i;

  if( context->test->datastep == 0 ) {
    test__run_void( context );
  } else {
    test__run_data( context, -1 );
  }
  return;
}

test_result_t test_run( const test_t* test ) {
  test_context_t context = {
    NULL,
    test,
    NULL,
    TEST_PASS,
    stdout,
    { 0 }, { 0 },
    0
  };
  
  test__run( &context );
  return context.result;
}

test_result_t test_suite_run( const test_suite_t* suite ) {
  test_context_t context = {
    suite,
    NULL,
    NULL,
    TEST_PASS,
    stdout,
    { 0 }, { 0 },
    0
  };
  int i;
  
  for( i=0; suite->tests[i] != NULL; i++ ) {
    context.test = suite->tests[i];
    context.result = TEST_PASS;
    test__run( &context );
  }
  
  fprintf( context.log, "RESULTS: %i/%i passed, %i failed, %i skipped\n",
    context.count[TEST_PASS],
    context.total,
    context.count[TEST_FAIL],
    context.count[TEST_SKIP]
  );
  
  if( context.count[TEST_FAIL] || context.count[TEST_SKIP] ) {
    context.result = TEST_FAIL;
  } else if( context.count[TEST_WARN] ) {
    fprintf( context.log, "WARNINGS: %i\n", context.count[TEST_WARN] );
    context.result = TEST_WARN;
  } else if( context.count[TEST_ERROR] ) {
    fprintf( context.log, "ERRORS: %i\n", context.count[TEST_ERROR] );
    context.result = TEST_ERROR;
  } else {
    context.result = TEST_PASS;
  }
  return context.result;
}

