#include "arguments.hpp"
#include "engine/engine.hpp"
#include "gui/main_window.hpp"

#include "lv2-util/World.hpp"
#include "lv2-util/Plugin.hpp"

#include <lilv/lilv.h>

#include <jack/jack.h>

#include <QApplication>

#include <iostream>
#include <thread>
#include <chrono>

using namespace std;
using namespace dzemer;

int main(int argc, char * argv[])
{
  string plugin_uri;

  arguments args;
  args.add_option("-plugin", plugin_uri);

  args.parse(argc-1, argv+1);

  LV2::World world;
  world.loadAll();

  auto engine = new Engine();

  Engine::Options options;
  options.audioOutputCount = 2;

  engine->start(options);

  if (!plugin_uri.empty())
  {

      cout << "Plugin URI: " << plugin_uri << endl;

      LV2::Plugin plugin = world.pluginForUri(plugin_uri);

      if (plugin)
      {
          cout << "Plugin name: " << plugin.name() << endl;

          engine->addSynth(plugin);
      }
      else
      {
        cerr << "Plugin not found." << endl;
        return 1;
      }
  }

  QApplication app(argc, argv);

  auto main_win = new MainWindow(engine, world);
  main_win->show();

  auto status = app.exec();

  delete main_win;

  delete engine;

  return status;
}
