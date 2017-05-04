#pragma once

#include <string>
#include <sstream>
#include <unordered_map>
#include <functional>

namespace dzemer {

using std::string;
using std::istringstream;
using std::unordered_map;
using std::function;

class arguments
{
public:
  struct invalid_syntax
  {
    string text;
  };

  struct invalid_option_error
  {
    invalid_option_error(const string & option): option(option) {}

    string option;
  };

  struct invalid_option_value
  {
    invalid_option_value(const string & o, const string & v): option(o), value(v) {}
    string option;
    string value;
  };

  struct missing_option_value
  {
    string option;
  };

  struct redundant_option_value
  {
    string option;
  };

  using parser_function = function<void(const string &)>;

  template <typename T>
  void add_option(const string & name, T & destination, const string & description = string())
  {
    auto parser = [name, &destination](const string & value)
    {
      if (value.empty())
        throw missing_option_value({name});

      istringstream text_stream(value);
      text_stream >> destination;
      if (!text_stream || text_stream.tellg() < value.size())
        throw invalid_option_value(name, value);
    };

    option_parsers.emplace(name, parser);
  }

  void add_switch(const string & name, bool & destination, bool enable, const string & description = string())
  {
    auto parser = [name, enable, &destination](const string & value)
    {
      if (!value.empty())
        throw redundant_option_value({name});

      destination = enable;
    };

    option_parsers.emplace(name, parser);
  }

  void parse(int argc, char * argv[])
  {
    int i = 0;
    while(i < argc)
    {
      string option = argv[i];

      string name;
      string value;

      auto separator_pos = option.find('=');

      name = option.substr(0,separator_pos);

      if (separator_pos != string::npos)
      {
        value = option.substr(separator_pos+1);
        if (value.empty())
          throw invalid_syntax({option});
      }

      auto parser_it = option_parsers.find(name);
      if (parser_it == option_parsers.end())
        throw invalid_option_error(name);

      auto & parser = parser_it->second;
      parser(value);

      ++i;
    }
  }

private:
  unordered_map<string, parser_function> option_parsers;
};

}
