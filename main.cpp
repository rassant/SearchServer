#include "./source/tests/test_speed.h"
#include "./source/tests/test_example_functions.h"
#include "example.h"

int main ( ){
  TestSearchServer ();
  Example ();
  StartSpeedTest();
  return 0;
}
