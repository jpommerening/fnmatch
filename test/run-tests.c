#include "test.h"

int main( int argc, char* argv[] ) {
  test_suite_t suite = {
    "default",
    &(tests[0]),
  };
  
  return (test_suite_run( &suite ) == TEST_PASS) ? 0 : 1;
}
