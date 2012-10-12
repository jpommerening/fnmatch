#include "fnmatch.h"
#include "test.h"

typedef struct test_pattern_s {
  const char* expr;
  const char* matchstr;
  const char* nomatchstr;
} test_pattern_t;

static const test_pattern_t _data[] = {

  /* fixed string */
  { "test", "test", "test " },

  /* question mark */
  { "tes?", "test", "tes" },

  /* asterisk */
  { "t*t", "tt", "tta" },
  { "t*t", "tatt", "t" },

  /* brackets */
  { "[qrstu]es[a-z]", "test", "best" },

  /* separators */
  { "t*/est", "test/est", "test" },

  /* end */
  { "test/*t", "test/est", "test/es" },

  /* "deep" matching asterisk */
  { "**.[ch]", "test/test.c", "test.d" },
  { "**/test/*.c", "this/test/is/a/test/file.c", "this/test/" },

  /* escaping -- remember: one backslash is for the C compiler */
  { "t\\?st", "t?st", "test" },
  { "t\\*st", "t*st", "test" },
  { "t\\[e]st", "t[e]st", "test" },
  { "t\\**st", "t*est", "t*e/st" },
  { "t\\*\\*t", "t**t", "t***t" },

  /* TBD: An escaped slash makes "t/st" one fixed string, but what shall happen next? */
  /*      The behaviour depends on how much data is available... */
  { "t\\/st", "t/st", "t\\/st" },
};

TEST( test_pattern, _data, const test_pattern_t* data ) {
  fnmatch_state_t state;
  fnmatch_pattern_t pattern;
  fnmatch_pattern_init( &pattern );
  
/*MSG("%s, %s, %s", data->expr, data->matchstr, data->nomatchstr);*/
  
  state = fnmatch_pattern_compile( &pattern, data->expr, 0 );
  ASSERTEQ( state, FNMATCH_CONTINUE, "Could not compile pattern `%s'.\n", data->expr );

  state = fnmatch_pattern_match( &pattern, data->matchstr );
  ASSERTEQ( state, FNMATCH_MATCH, "`%s' did not match %s' but should.\n", data->expr, data->matchstr );

  state = fnmatch_pattern_match( &pattern, data->nomatchstr );
  ASSERTEQ( state, FNMATCH_NOMATCH, "`%s' did match `%s' but shouldn't.\n", data->expr, data->nomatchstr );
}
