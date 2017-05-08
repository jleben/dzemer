#pragma once

#include "Node.hpp"
#include "Feature.hpp"

#include <lilv/lilv.h>
#include <lv2core.lv2/lv2.h>
#include <atom.lv2/atom.h>
#include <midi.lv2/midi.h>

#include <string>
#include <vector>

namespace LV2 {

using std::string;
using std::vector;

class World;

class PluginInstance;

class Port
{
  LilvWorld * world = nullptr;
  const LilvPlugin * plugin = nullptr;
  const LilvPort * port = nullptr;

public:

  struct InvalidType {};

  Port()
  {}

  Port(const LilvPort * port, const LilvPlugin * plugin, LilvWorld * world):
    world(world),
    plugin(plugin),
    port(port)
  {}

  operator bool() const
  {
    return port != nullptr;
  }

  string name() const
  {
    Node value(lilv_port_get_name(plugin, port));
    return value.to_string();
  }

  uint32_t index() const
  {
    return lilv_port_get_index(plugin, port);
  }

  bool is_a(const char * uri_string)
  {
    Node uri = lilv_new_uri(world, uri_string);
    return lilv_port_is_a(plugin, port, uri.get());
  }

  bool supports_event(const char * uri_string)
  {
    auto uri = lilv_new_uri(world, uri_string);
    bool value = lilv_port_supports_event(plugin, port, uri);
    lilv_node_free(uri);
    return value;
  }

  bool is_output()
  {
    return is_a(LV2_CORE__OutputPort);
  }

  bool is_input()
  {
    return is_a(LV2_CORE__InputPort);
  }

  bool is_control()
  {
    return is_a(LV2_CORE__ControlPort);
  }

  bool is_audio()
  {
    return is_a(LV2_CORE__AudioPort);
  }

  bool is_midi()
  {
    return is_a(LV2_ATOM__AtomPort) && supports_event(LV2_MIDI__MidiEvent);
  }

  Node predicate(const char * uri_string)
  {
    Node uri(lilv_new_uri(world, uri_string));
    return lilv_port_get(plugin, port, uri.get());
  }
};

class Plugin
{
  LilvWorld * w = nullptr;
  const LilvPlugin * p = nullptr;

public:

  Plugin() {}

  Plugin(const LilvPlugin * p, LilvWorld * w): w(w), p(p) {}

  operator bool() const
  {
    return p != nullptr;
  }

  string name() const
  {
    return lilv_node_as_string(lilv_plugin_get_name(p));
  }

  PluginInstance * instantiate(double sampleRate, const vector<Feature> & features = vector<Feature>());

  int port_count() const
  {
    return lilv_plugin_get_num_ports(p);
  }

  Port port_at(int index) const
  {
    return Port(lilv_plugin_get_port_by_index(p, index), p, w);
  }
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
