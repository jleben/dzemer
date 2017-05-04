#include "Plugin.hpp"

namespace LV2 {

PluginInstance * Plugin::instantiate(double sampleRate, const vector<Feature> & features)
{
  vector<const Feature*> feature_pointers(features.size() + 1);

  for (int i = 0; i < (int) features.size(); ++i)
    feature_pointers[i] = &features[i];

  feature_pointers.back() = nullptr;

  auto i = lilv_plugin_instantiate(p, sampleRate, feature_pointers.data());

  if (!i)
    return nullptr;

  return new PluginInstance(i);
}

}
