#include "fnmatch.h"
#include "test.h"

typedef struct test_context_s {
  const char* expr;
  const char* matchstr;
  const char* nomatchstr;
} test_context_t;

test_t test_context( void ) {
  PASS;
}

TEST_VOID(test_context);
