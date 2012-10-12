#include "fnmatch.h"
#include "test.h"
#include <string.h>

TEST( test_context_single ) {
  fnmatch_context_t context;
  fnmatch_pattern_t pattern;
  const char* str;
  
  fnmatch_pattern_init( &pattern );
  ASSERTEQ( FNMATCH_CONTINUE, fnmatch_pattern_compile( &pattern, "test/**.[hc]", 0 ),
            "Failed to compile pattern.\n" );
  fnmatch_context_init( &context, &pattern );

  fnmatch_context_push( &context, "test/test-context.c" );
  ASSERTEQ( FNMATCH_MATCH, fnmatch_context_match( &context ) ); /* test/ */
  ASSERTEQ( FNMATCH_PUSH, fnmatch_context_match( &context ) );
  fnmatch_context_push( &context, NULL );
  ASSERTEQ( FNMATCH_MATCH, fnmatch_context_match( &context ) ); /* test-context.c */
  ASSERTEQ( FNMATCH_POP, fnmatch_context_match( &context ) );
  str = fnmatch_context_pop( &context );
  ASSERTSTREQ( "test-context.c", str, "with str = `%s'\n", str );

  ASSERTEQ( context.nmatch, 1 );
  ASSERTEQ( context.nnomatch, 0 );

  fnmatch_context_destroy( &context );
  fnmatch_pattern_destroy( &pattern );
}

TEST( test_context_multiple ) {
  fnmatch_context_t context;
  fnmatch_pattern_t pattern;
  const char* str;
  
  fnmatch_pattern_init( &pattern );
  ASSERTEQ( FNMATCH_CONTINUE, fnmatch_pattern_compile( &pattern, "test/**.[hc]", 0 ),
            "Failed to compile pattern.\n" );
  fnmatch_context_init( &context, &pattern );

  fnmatch_context_push( &context, "xxx/" );
  ASSERTEQ( FNMATCH_NOMATCH, fnmatch_context_match( &context ) );
  ASSERTEQ( FNMATCH_POP, fnmatch_context_match( &context ) );
  str = fnmatch_context_pop( &context );
  ASSERTSTREQ( "xxx/", str, "with str = `%s'\n", str );
  
  ASSERTEQ( FNMATCH_PUSH, fnmatch_context_match( &context ) );
  fnmatch_context_push( &context, "test/" );
  ASSERTEQ( FNMATCH_MATCH, fnmatch_context_match( &context ) ); /* "test/" */
  
  ASSERTEQ( FNMATCH_PUSH, fnmatch_context_match( &context ) );
  fnmatch_context_push( &context, "foo" );
  ASSERTEQ( FNMATCH_PUSH, fnmatch_context_match( &context ) );
  fnmatch_context_push( &context, "/bar/" );
  ASSERTEQ( FNMATCH_MATCH, fnmatch_context_match( &context ) ); /* "foo/" */
  ASSERTEQ( FNMATCH_MATCH, fnmatch_context_match( &context ) ); /* "bar/" */

  ASSERTEQ( FNMATCH_PUSH, fnmatch_context_match( &context ) );
  fnmatch_context_push( &context, "baz.c" );
  ASSERTEQ( FNMATCH_PUSH, fnmatch_context_match( &context ) );
  fnmatch_context_push( &context, NULL );

  ASSERTEQ( FNMATCH_MATCH, fnmatch_context_match( &context ) ); /* "baz.c" */
  ASSERTEQ( FNMATCH_POP, fnmatch_context_match( &context ) );
  str = fnmatch_context_pop( &context );
  ASSERTSTREQ( "baz.c", str, "with str = `%s'\n", str );
  
  ASSERTEQ( FNMATCH_PUSH, fnmatch_context_match( &context ) );
  fnmatch_context_push( &context, "baz.h" );
  ASSERTEQ( FNMATCH_PUSH, fnmatch_context_match( &context ) );
  fnmatch_context_push( &context, NULL );

  ASSERTEQ( FNMATCH_MATCH, fnmatch_context_match( &context ) ); /* "baz.h" */
  ASSERTEQ( FNMATCH_POP, fnmatch_context_match( &context ) );
  str = fnmatch_context_pop( &context );
  ASSERTSTREQ( "baz.h", str, "with str = `%s'\n", str );

  ASSERTEQ( FNMATCH_PUSH, fnmatch_context_match( &context ) );
  fnmatch_context_push( &context, NULL );
  ASSERTEQ( FNMATCH_NOMATCH, fnmatch_context_match( &context ) );
  ASSERTEQ( FNMATCH_POP, fnmatch_context_match( &context ) );
  str = fnmatch_context_pop( &context );
  ASSERTSTREQ( "bar/", str, "with str = `%s'\n", str );
  
  ASSERTEQ( FNMATCH_PUSH, fnmatch_context_match( &context ) );
  fnmatch_context_push( &context, "bar.h" );
  ASSERTEQ( FNMATCH_PUSH, fnmatch_context_match( &context ) );
  fnmatch_context_push( &context, NULL );
  
  ASSERTEQ( FNMATCH_MATCH, fnmatch_context_match( &context ) ); /* "bar.h" */
  ASSERTEQ( FNMATCH_POP, fnmatch_context_match( &context ) );
  str = fnmatch_context_pop( &context );
  ASSERTSTREQ( "bar.h", str, "with str = `%s'\n", str );
  
  ASSERTEQ( FNMATCH_PUSH, fnmatch_context_match( &context ) );
  fnmatch_context_push( &context, NULL );
  ASSERTEQ( FNMATCH_NOMATCH, fnmatch_context_match( &context ) );
  ASSERTEQ( FNMATCH_POP, fnmatch_context_match( &context ) );
  str = fnmatch_context_pop( &context );
  ASSERTSTREQ( "foo/", str, "with str = `%s'\n", str );

  ASSERTEQ( FNMATCH_PUSH, fnmatch_context_match( &context ) );
  fnmatch_context_push( &context, NULL );
  ASSERTEQ( FNMATCH_NOMATCH, fnmatch_context_match( &context ) );
  ASSERTEQ( FNMATCH_POP, fnmatch_context_match( &context ) );
  str = fnmatch_context_pop( &context );
  ASSERTSTREQ( "test/", str, "with str = `%s'\n", str );
  
  ASSERTEQ( context.nmatch, 3 );
  ASSERTEQ( context.nnomatch, 4 );

  fnmatch_context_destroy( &context );
  fnmatch_pattern_destroy( &pattern );
}