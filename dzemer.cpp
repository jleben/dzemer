#include "arguments.hpp"
#include "util.hpp"
#include "Control.hpp"

#include "lv2-util/World.hpp"
#include "lv2-util/Plugin.hpp"
#include "lv2-util/UriMap.hpp"
#include "lv2-util/Atom.hpp"
#include "lv2-util/Midi.hpp"

#include <lilv/lilv.h>

#include <jack/jack.h>

#include <iostream>
#include <unordered_map>
#include <list>
#include <thread>
#include <chrono>

using namespace std;
using namespace dzemer;

class App
{
public:
  App(LV2::Plugin plugin):
    m_plugin(plugin),
    m_midi_buffer(1024)
  {
    // Create JACK client

    jack_status_t status;
    m_client = jack_client_open("dzemer", JackNoStartServer, &status);
    if (!m_client)
    {
      throw std::runtime_error("Failed to create JACK client.");
    }

    jack_set_process_callback (m_client, jack_process, this);

    sample_rate = jack_get_sample_rate(m_client);

    // Instantiate synth

    m_synth = m_plugin.instantiate(sample_rate, {m_uri_map.feature()} );

    if (!m_synth)
    {
      throw std::runtime_error("Failed to instantiate plugin.");
    }

    // Connect synth port buffers

    bool midi_input_connected = false;

    for (int i = 0; i < m_plugin.port_count(); ++i)
    {
      auto port = m_plugin.port_at(i);

      cout << "Connecting port at " << i
           << " index = " << port.index()
           << " name = " << port.name()
           << endl;

      if (port.is_audio() && port.is_output())
      {
        cout << "Audio port." << endl;

        auto jack_port =
            jack_port_register (m_client, port.name().c_str(),
                                JACK_DEFAULT_AUDIO_TYPE,
                                JackPortIsOutput, 0);
        if (!jack_port)
          throw std::runtime_error("Failed to create JACK port.");

        m_out_ports.emplace_back(port.index(), jack_port);
      }
      else if (port.is_midi() && port.is_input() && !midi_input_connected)
      {
        cout << "Midi port." << endl;
        m_synth->connect_port(port.index(), m_midi_buffer.data());
        midi_input_connected = true;
      }
      else if (port.is_control())
      {
        float default_value = port.predicate(LV2_CORE__default).to_float();

        cout << "Control port: Default value = " << default_value << endl;

        m_control_buffers.push_back(default_value);
        m_synth->connect_port(port.index(), &m_control_buffers.back());
      }
      else
      {
        throw std::runtime_error("Can not connect port.");
      }
    }

    // Activate JACK client

    if (jack_activate (m_client)) {
      throw std::runtime_error("Could not activate JACK client.");
    }

    // Connect JACK client ports to system output ports

    auto input_ports = jack_get_ports (m_client, NULL, NULL,
                                       JackPortIsPhysical|JackPortIsInput);

    auto input_port = input_ports;

    for (auto & port : m_out_ports)
    {
      if (!input_ports)
        break;

      cout << "Connecting port "
           << jack_port_name (port.second)
           << " to system port " << *input_ports
           << endl;

      if (jack_connect (m_client, jack_port_name (port.second), *input_port)) {
        cerr << "Could not connect ports." << endl;
      }

      ++input_port;
    }

    jack_free (input_ports);

    // Activate synth

    m_synth->activate();
  }

  ~App()
  {
    m_synth->deactivate();

    jack_client_close(m_client);

    delete m_synth;
  }

private:
  static int jack_process(jack_nframes_t nframes, void *arg)
  {
    return reinterpret_cast<App*>(arg)->process(nframes);
  }

  int process(jack_nframes_t nframes)
  {
    for (auto & port : m_out_ports)
    {
      auto buf = jack_port_get_buffer(port.second, nframes);
      m_synth->connect_port(port.first, buf);
    }

    m_midi_buffer.reset();

    if (elapsed_frames == 0 || elapsed_frames - last_note_on >= sample_rate)
    {
      printf("Note on.\n");

      MidiStream stream(m_midi_buffer, m_uri_map);
      stream.noteOn(0, 0, 60, 100);

      last_note_on = elapsed_frames;
      note_is_on = true;
    }
    else if (note_is_on && elapsed_frames - last_note_on >= sample_rate * 0.5)
    {
        printf("Note off.\n");

        MidiStream stream(m_midi_buffer, m_uri_map);
        stream.noteOff(0, 0, 60, 100);

        note_is_on = false;
    }
    else
    {
      clearControlPort();
    }

    m_synth->run(nframes);

    for (auto & port : m_out_ports)
    {
      auto buf = (jack_default_audio_sample_t*) jack_port_get_buffer(port.second, nframes);
    }

    elapsed_frames += nframes;

    return 0;
  }

  void clearControlPort()
  {
    MidiStream stream(m_midi_buffer, m_uri_map);
  }

  UriMapper m_uri_map;
  LV2::Plugin m_plugin;
  LV2::PluginInstance * m_synth;
  jack_client_t * m_client;

  vector<pair<int, jack_port_t*>> m_out_ports;
  LV2::AtomBuffer m_midi_buffer;
  list<float> m_control_buffers;

  jack_nframes_t sample_rate = 0;
  jack_nframes_t last_note_on =  0;
  bool note_is_on = false;
  jack_nframes_t elapsed_frames = 0;
};

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

  LV2::Plugin plugin = world.pluginForUri(plugin_uri);

  if (!plugin)
  {
    cerr << "Plugin not found." << endl;
    return 1;
  }

  cout << "Plugin name: " << plugin.name() << endl;

  auto app = new App(plugin);

  cerr << "Running..." << endl;

  while(1)
  {
    this_thread::sleep_for(chrono::seconds(1));
  }

  delete app;
}
