#pragma once

#include <lilv/lilv.h>

#include "Plugin.hpp"

#include <vector>
#include <string>

namespace LV2 {

using std::vector;
using std::string;

class World
{
  LilvWorld * w;

public:
  struct InitializationError {};

  World(): w(lilv_world_new())
  {
    if (!w)
      throw InitializationError();
  }

  ~World()
  {
    lilv_world_free(w);
  }

  World(const World & other) = delete;

  void loadAll()
  {
    lilv_world_load_all(w);
  }

  Plugin pluginForUri(const string & uri)
  {
    auto plugins = lilv_world_get_all_plugins(w);

    auto lilv_uri = lilv_new_uri(w, uri.c_str());

    auto plugin = lilv_plugins_get_by_uri(plugins, lilv_uri);

    lilv_node_free(lilv_uri);

    return Plugin(plugin, w);
  }

  vector<Plugin> allPlugins()
  {
      vector<Plugin> result;

      auto plugins = lilv_world_get_all_plugins(w);

      for(auto i = lilv_plugins_begin(plugins);
          !lilv_plugins_is_end(plugins, i);
          i = lilv_plugins_next(plugins, i))
      {
          result.emplace_back(lilv_plugins_get(plugins, i), w);
      }

      return std::move(result);
  }
};

}
