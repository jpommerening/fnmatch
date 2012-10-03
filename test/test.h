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

typedef void (*test_any_cb)();
typedef void (*test_void_cb)( test_context_t* context );
typedef void (*test_data_cb)( test_context_t* context, const void* );

/* from gen-tests.sh -> gen-tests.c */
TEST_EXTERN const test_t* tests[];

typedef union {
  test_any_cb  any_cb;
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

#define TESTCTX __ctx
#define TEST_UID(name) test__ ## name
#define TEST_STRUCT(name) \
  const test_t TEST_UID(name)

/* Crazy preprocessor macros ahead:
 *   We trick the preprocessor into telling us if TEST() was supplied
 *   with additional parameters.
 *   Depending on that, we place different stuff into the struct.
 */

/* If additional parameters are given in `...' expand them
 * using the function-like macro `a', otherwise insert `b'. */
#define IF_ARGS(a,b,...) _IF_ARGS(__VA_ARGS__) ? b : a(__VA_ARGS__ ((void*)0)) /* dummy arg*/
#define _IF_ARGS(...)    (sizeof(#__VA_ARGS__) == 1)

/* Initializers for test structs with test data. */
#define _TEST_DATASTEP(data, ...) \
  sizeof(data[0])
#define _TEST_DATALENGTH(data, ...) \
  sizeof(data)/sizeof(data[0])
#define _TEST_DATAPTR(data, ...) \
  &(data[0])

/* Generate the signature of a test function.
 * Skip the first variadic parameter and use the rest inside the declaration. */
#define TEST_SIGN(name,...) \
  _TEST_SIGN(name,__VA_ARGS__)
#define _TEST_SIGN(name,d,...) \
  static void name( test_context_t* TESTCTX, ## __VA_ARGS__ )

/**
 * Declare test functions with this macro.
 * @param name the name of the function to declare.
 * @param [data] an array containing test data to drive the test.
 * @param [arg] the parameter that is used by the function to accept data.
 */
#define TEST(name,...) \
  TEST_SIGN(name,## __VA_ARGS__); \
  TEST_STRUCT(name) = { \
    #name, &name, \
    IF_ARGS(_TEST_DATASTEP,0,## __VA_ARGS__), \
    IF_ARGS(_TEST_DATALENGTH,0,## __VA_ARGS__), \
    IF_ARGS(_TEST_DATAPTR,NULL,## __VA_ARGS__), \
  }; \
  TEST_SIGN(name,## __VA_ARGS__)

TEST_EXTERN void test_message( test_context_t* context, const char* file, int line, const char* fmt, ... );
TEST_EXTERN void test_status( test_context_t* context, test_result_t result );
TEST_EXTERN test_result_t test_run( const test_t* test );
TEST_EXTERN test_result_t test_suite_run( const test_suite_t* suite );

#define MSG(...) test_message(TESTCTX, __FILE__, __LINE__, "" __VA_ARGS__)
#define RESULT(x) test_status(TESTCTX,x)

#define FAIL(...) do { MSG(__VA_ARGS__); RESULT(TEST_FAIL); return; } while( 0 )
#define WARN(...) do { MSG(__VA_ARGS__); RESULT(TEST_WARN); } while( 0 )
#define ERROR(...) do { MSG(__VA_ARGS__); RESULT(TEST_ERROR); abort(); } while( 0 )

#define ASSERTBASE(x,...) do if( !(x) ) { FAIL(__VA_ARGS__); } while( 0 )
#define ASSERT(x,...) ASSERTBASE(x,"`" #x "' is not true!\n" __VA_ARGS__)
#define ASSERTEQ(a,b,...) ASSERTBASE((a) == (b),"`" #a "' does not equal `" #b "'!\n" __VA_ARGS__)
#define ASSERTLT(a,b,...) ASSERTBASE((a) < (b),"`" #a "' is not less than `" #b "'!\n" __VA_ARGS__)
#define ASSERTGT(a,b,...) ASSERTBASE((a) > (b),"`" #a "' is not greater than`" #b "'!\n" __VA_ARGS__)
#define ASSERTSTRCMP(a,op,b) ((a==b) || (a!=NULL) && (b!=NULL) && strcmp(a,b) op 0)
#define ASSERTSTREQ(a,b,...) ASSERTBASE(ASSERTSTRCMP(a,==,b),"Strings `" #a "' and `" #b "' are not equal!\n" __VA_ARGS__)
#define ASSERTSTRLT(a,b,...) ASSERTBASE(ASSERTSTRCMP(a,<,b),"String `" #a "' is not `less than' string `" #b "'!\n" __VA_ARGS__)
#define ASSERTSTRGT(a,b,...) ASSERTBASE(ASSERTSTRCMP(a,>,b),"String `" #a "' is not `greater than' string `" #b "'!\n" __VA_ARGS__)

#ifdef __cplusplus
}
#endif

#endif
