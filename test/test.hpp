#pragma once

#include <string>
#include <unordered_map>

namespace dzemer {
namespace testing {

using std::string;
using std::unordered_map;

struct Failure
{
  Failure(const string & m): message(m) {}
  string message;
};

class driver
{
public:
  using test_func_t = void(*)();

  int run(int argc, char * argv[]);

  void add_test(const string & name, test_func_t func)
  {
    m_tests[name] = func;
  }

private:
  unordered_map<string,test_func_t> m_tests;
};

inline
void assert(bool flag, const string & message)
{
  if (!flag)
    throw Failure(message);
}

}
}
