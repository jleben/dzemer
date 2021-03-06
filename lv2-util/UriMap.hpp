#pragma once

#include "Feature.hpp"

#include <urid.lv2/urid.h>

#include <string>

namespace LV2 {

using std::string;

class UriMap
{
public:

  using id_t = LV2_URID;

  UriMap()
  {
    urid.handle = this;
    urid.map = &static_map;
  }

  virtual id_t map(const string & uri) = 0;

  id_t operator[](const string & uri) { return map(uri); }

  Feature feature()
  {
    LV2_Feature f;
    f.URI = LV2_URID__map;
    f.data = &urid;

    return f;
  }

private:

  static LV2_URID static_map(LV2_URID_Map_Handle handle, const char * uri)
  {
    UriMap * me = reinterpret_cast<UriMap*>(handle);
    return me->map(uri);
  }

  LV2_URID_Map urid;
};

}
