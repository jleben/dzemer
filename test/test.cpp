#include "test.hpp"

#include <iostream>

using namespace std;

namespace dzemer {
namespace testing {

int driver::run(int argc, char *argv[])
{
  if (argc < 2)
  {
    cerr << "Expected arguments: <test name>" << endl;
    return 1;
  }

  string test_name = argv[1];

  auto test = m_tests[test_name];

  if (!test)
  {
    cerr << "No such test: " << test_name << endl;
    return 1;
  }

  try {
    test();
  } catch (dzemer::testing::Failure & failure) {
    cerr << "Failed: " << failure.message << endl;
    return 1;
  }

  cerr << "OK." << endl;
  return 0;
}

}
}
