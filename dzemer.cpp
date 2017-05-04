#include "arguments.hpp"

#include "lv2-util/World.hpp"
#include "lv2-util/Plugin.hpp"
#include "lv2-util/UriMap.hpp"

#include <lilv/lilv.h>

#include <iostream>
#include <unordered_map>

using namespace std;
using namespace dzemer;

namespace dzemer {

using std::unordered_map;

class UriMapper : public LV2::UriMap
{
  unordered_map<string, id_t> m_map;

  id_t map(const string & uri) override
  {
    static id_t next_id = 1;

    auto & id = m_map[uri];

    if (id == 0)
    {
      id = next_id;
      ++next_id;
    }

    cout << "Mapped " << uri << " to " << id << endl;

    return id;
  }
};

}

int main(int argc, char * argv[])
{
  string plugin_uri;

  arguments args;
  args.add_option("-plugin", plugin_uri);

  args.parse(argc-1, argv+1);

  if (plugin_uri.empty())
  {
    cerr << "Missing argument: Plugin URI (-plugin)" << endl;
    return 1;
  }

  cout << "Plugin URI: " << plugin_uri << endl;

  LV2::World world;

  world.loadAll();

  UriMapper uri_mapper;

  LV2::Plugin plugin = world.pluginForUri(plugin_uri);

  if (!plugin)
  {
    cerr << "Plugin not found." << endl;
    return 1;
  }

  cout << "Plugin name: " << plugin.name() << endl;

  auto instance = plugin.instantiate(44100, {uri_mapper.feature()} );

  if (!instance)
  {
    cerr << "Failed to instantiate." << endl;
    return 1;
  }

  cout << "Instance URI: " << instance->uri() << endl;

  delete instance;
}
