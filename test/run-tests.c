#include "test.h"

/* from gen-tests.sh -> gen-tests.c */
extern const test_item_t* tests[];

void test_run( test_statistics_t* statistics, const test_item_t* test );

int main( int argc, char* argv[] ) {
  int i;
  test_statistics_t statistics = { 0, 0, 0, 0, 0, 0 };

  for( i=0; tests[i] != NULL; i++ ) {
    test_run( &statistics, tests[i] );
  }

  printf( "RESULTS: %i/%i passed, %i failed, %i skipped\n",
    statistics.pass,
    statistics.total,
    statistics.fail,
    statistics.skip
  );
  return 0;
}
