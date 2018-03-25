#define BOOST_TEST_MODULE VariableOption test
#include <boost/test/unit_test.hpp>
#include "VariableOption.h"

BOOST_AUTO_TEST_CASE(value)
{
  VariableOption vo("name", 50.5, 1, 271, 0.01);
  
  BOOST_CHECK_EQUAL(vo.value(), 50.5);
}
