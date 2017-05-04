#pragma once

#include "Feature.hpp"

#include <lilv/lilv.h>

#include <string>
#include <vector>

namespace LV2 {

using std::string;
using std::vector;

class World;

class PluginInstance;

class Plugin
{
  const LilvPlugin * p = nullptr;

public:

  Plugin() {}

  Plugin(const LilvPlugin * p): p(p) {}

  operator bool()
  {
    return p != nullptr;
  }

  string name() const
  {
    return lilv_node_as_string(lilv_plugin_get_name(p));
  }

  PluginInstance * instantiate(double sampleRate, const vector<Feature> & features = vector<Feature>());
};

class PluginInstance
{
  LilvInstance * i;

public:
  PluginInstance(LilvInstance * i): i(i) {}

  ~PluginInstance()
  {
    lilv_instance_free(i);
  }

  PluginInstance(const PluginInstance &) = delete;

  string uri()
  {
    return lilv_instance_get_uri(i);
  }
};

}
