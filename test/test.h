#ifndef _TEST_H_
#define _TEST_H_

#ifdef __cplusplus
extern "C" {
#endif

#define TEST_EXTERN extern

typedef enum {
  TEST_PASS,
  TEST_FAIL,
  TEST_SKIP,
  TEST_WARN,
  TEST_ERROR,
  TEST_RESULT_MAX
} test_result_t;

typedef struct test_s test_t;
typedef struct test_suite_s test_suite_t;
typedef struct test_context_s test_context_t;
typedef void (*test_void_cb)( test_context_t* context );
typedef void (*test_data_cb)( test_context_t* context, const void* );

/* from gen-tests.sh -> gen-tests.c */
TEST_EXTERN const test_t* tests[];

typedef union {
  test_void_cb void_cb;
  test_data_cb data_cb;
} test_cb;

struct test_s {
  const char* name;
  test_cb callback;
  int datastep;
  int datalength;
  const void* data;
};

struct test_suite_s {
  const char* name;
  const test_t** tests;
};

struct test_context_s {
  const test_suite_t* suite;
  const test_t* test;
  const void* data;
  test_result_t result;
  int total;
  int count[TEST_RESULT_MAX];
};

#define TEST_UID(name) test__ ## name

#define TEST_DATA_SIGN(name, arg) \
  void name(test_context_t* __ctx, arg)
#define TEST_VOID_SIGN(name) \
  void name(test_context_t* __ctx)
#define TEST_STRUCT(name) \
  const test_t TEST_UID(name)

#define TEST_DATA(name, data, arg) \
  TEST_DATA_SIGN(name, arg); \
  TEST_STRUCT(name) = { \
    #name, { .data_cb = (test_data_cb) &name }, \
    (void*) &(data[1]) - (void*) &(data[0]), \
    sizeof(data)/sizeof(data[0]), \
    &(data[0]) \
  }; \
  TEST_DATA_SIGN(name, arg)

#define TEST_VOID(name) \
  TEST_VOID_SIGN(name); \
  TEST_STRUCT(name) = { \
    #name, { .void_cb = (test_void_cb) &name }, \
    0, 0, NULL \
  }; \
  TEST_VOID_SIGN(name)

TEST_EXTERN void test_message( test_context_t* context, const char* file, int line, const char* fmt, ... );
TEST_EXTERN void test_status( test_context_t* context, test_result_t result );
TEST_EXTERN test_result_t test_run( const test_t* test );
TEST_EXTERN test_result_t test_suite_run( const test_suite_t* suite );

#define TEST_MSG(...) test_message(__ctx, __FILE__, __LINE__, __VA_ARGS__)
#define TEST_STATUS(x) test_status(__ctx,x)

#define FAIL(...) do { TEST_MSG(__VA_ARGS__); TEST_STATUS(TEST_FAIL); return; } while( 0 )
#define WARN(...) do { TEST_MSG(__VA_ARGS__); TEST_STATUS(TEST_WARN); } while( 0 )
#define ERROR(...) do { TEST_MSG(__VA_ARGS__); TEST_STATUS(TEST_ERROR); abort(); } while( 0 )

#define ASSERTBASE(x,...) do if( !(x) ) { FAIL(__VA_ARGS__); } while( 0 )
#define ASSERT(x,...) ASSERTBASE(x,"`" #x "' is not true!\n", ## __VA_ARGS__)
#define ASSERTEQ(a,b,...) ASSERTBASE((a) == (b),"`" #a "' does not equal `" #b "'!\n" __VA_ARGS__)
#define ASSERTLT(a,b,...) ASSERTBASE((a) < (b),"`" #a "' is not less than `" #b "'!\n" __VA_ARGS__)
#define ASSERTGT(a,b,...) ASSERTBASE((a) > (b),"`" #a "' is not greater than`" #b "'!\n" __VA_ARGS__)
#define ASSERTSTREQ(a,b,...) ASSERTBASE(strcmp(a,b) == 0,"Strings `" #a "' and `" #b "' are not equal!\n" __VA_ARGS__)
#define ASSERTSTRLT(a,b,...) ASSERTBASE(strcmp(a,b) < 0,"String `" #a "' is not `less than' string `" #b "'!\n" __VA_ARGS__)
#define ASSERTSTRGT(a,b,...) ASSERTBASE(strcmp(a,b) > 0,"String `" #a "' is not `greater than' string `" #b "'!\n" __VA_ARGS__)

#ifdef __cplusplus
}
#endif

#endif
