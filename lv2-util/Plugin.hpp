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
  using port_index_t = uint32_t;
  using sample_count_t = uint32_t;

  PluginInstance(LilvInstance * i): i(i) {}

  ~PluginInstance()
  {
    lilv_instance_free(i);
  }

  PluginInstance(const PluginInstance &) = delete;

  void connect_port(port_index_t index, void * data)
  {
    lilv_instance_connect_port(i, index, data);
  }

  void activate()
  {
    lilv_instance_activate(i);
  }

  void deactivate()
  {
    lilv_instance_deactivate(i);
  }

  void run(sample_count_t sample_count)
  {
    lilv_instance_run(i, sample_count);
  }

  string uri()
  {
    return lilv_instance_get_uri(i);
  }
};

}
