#include "test.h"
#include <stdio.h>

void test_finish( test_statistics_t* statistics, const test_item_t* test, test_t result ) {
  switch( result ) {
    case TEST_PASS:
      statistics->pass++;
      printf( "PASS\n" );
      break;
    case TEST_FAIL:
      statistics->fail++;
      printf( "FAIL\n" );
      break;
    case TEST_SKIP:
      statistics->skip++;
      printf( "SKIP\n" );
      break;
    case TEST_WARN:
      statistics->warn++;
      printf( "WARN\n" );
      break;
    case TEST_ERROR:
      statistics->error++;
      printf( "ERROR\n" );
      break;
  }
}

void test_run_void( test_statistics_t* statistics, const test_item_t* test ) {
  test_t result;
  statistics->total++;
  printf( "TEST %s: ", test->name );
  result = (test->callback.void_cb)();
  test_finish( statistics, test, result );
}

void test_run_data( test_statistics_t* statistics, const test_item_t* test, size_t i ) {
  test_t result;
  statistics->total++;
  printf( "TEST %s [%lu/%lu]: ", test->name, (unsigned long) i+1, (unsigned long) test->length );
  result = (test->callback.data_cb)( (test->data) + (i*test->step) );
  test_finish( statistics, test, result );
}

void test_run( test_statistics_t* statistics, const test_item_t* test ) {
  size_t i;
  test_t result;

  if( test->data == NULL ) {
    test_run_void( statistics, test );
  } else {
    for( i=0; i<(test->length); i++ ) {
      test_run_data( statistics, test, i );
    }
  }
}

