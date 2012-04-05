#ifndef _TEST_H_
#define _TEST_H_

#include <unistd.h>
#include <stdio.h>

typedef enum {
  TEST_PASS,
  TEST_FAIL,
  TEST_SKIP,
  TEST_WARN,
  TEST_ERROR
} test_t;

#define ASSERT(x,m,...) do if( !(x) ) { fprintf( stderr, "\n%s:%i: Assertion failed:\n  " #x "\n" m, __FILE__, __LINE__, ## __VA_ARGS__ ); FAIL; } while( 0 )
#define ASSERT_EQUALS(a,b,...) ASSERT( (a) == (b), __VA_ARGS__ )
#define PASS return TEST_PASS
#define FAIL return TEST_FAIL

typedef test_t (*test_void_cb)( void );
typedef test_t (*test_data_cb)( const void* );

typedef union {
  test_void_cb void_cb;
  test_data_cb data_cb;
} test_cb;

typedef struct test_item_s {
  test_cb callback;
  const char* name;
  size_t step;
  size_t length;
  const void* data;
} test_item_t;

typedef struct test_statistics_s {
  int total;
  int pass;
  int fail;
  int skip;
  int warn;
  int error;
} test_statistics_t;

#define TEST_UID(function) __ ## function

#define TEST_VOID_NAME(name,function) const test_item_t TEST_UID(function) = { \
  { .void_cb = (test_void_cb) function }, \
  name, \
  0, \
  0, \
  NULL \
}

#define TEST_DATA_NAME(name,function,data) const test_item_t TEST_UID(function) = { \
  { .data_cb = (test_data_cb) function }, \
  name, \
  (void*) &(data[1]) - (void*) &(data[0]), \
  sizeof(data)/sizeof(data[0]), \
  (void*) &(data[0]) \
}

#define TEST_VOID(function) TEST_VOID_NAME(#function,function)
#define TEST_DATA(function,data) TEST_DATA_NAME(#function,function,data)

#endif
