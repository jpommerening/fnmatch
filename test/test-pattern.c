#include "fnmatch.h"
#include "test.h"

typedef struct test_pattern_s {
  const char* expr;
  const char* matchstr;
  const char* nomatchstr;
} test_pattern_t;

static const test_pattern_t data[] = {
  { "t*est", "test", "t/est" },
};

test_t test_pattern( test_pattern_t* data ) {
  fnmatch_state_t state;
  fnmatch_pattern_t pattern;
  fnmatch_pattern_init( &pattern );

  state = fnmatch_pattern_compile( &pattern, data->expr );
  ASSERT_EQUALS( state, FNMATCH_CONTINUE, "Could not compile pattern `%s'.\n", data->expr );

  state = fnmatch_pattern_match( &pattern, data->matchstr );
  ASSERT_EQUALS( state, FNMATCH_MATCH, "`%s' did not match ´%s' but should.\n", data->expr, data->matchstr );

  state = fnmatch_pattern_match( &pattern, data->nomatchstr );
  ASSERT_EQUALS( state, FNMATCH_NOMATCH, "`%s' did match `%s' but shouldn't.\n", data->expr, data->nomatchstr );

  PASS;
}

TEST_DATA(test_pattern,data);
