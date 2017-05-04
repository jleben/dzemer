#pragma once

#include "lv2-util/UriMap.hpp"

#include <unordered_map>
#include <string>

namespace dzemer {

using std::string;

class UriMapper : public LV2::UriMap
{
  std::unordered_map<string, id_t> m_map;

  id_t map(const string & uri) override
  {
    static id_t next_id = 1;

    auto & id = m_map[uri];

    if (id == 0)
    {
      id = next_id;
      ++next_id;
    }

    //cout << "Mapped " << uri << " to " << id << endl;

    return id;
  }
};

}
