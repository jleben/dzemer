#include "engine.hpp"
#include "../Control.hpp"

#include <iostream>

using namespace std;

namespace dzemer
{

// FIXME: error handling

Engine::~Engine()
{
    stop();
}

void Engine::start(const Options & options)
{
    if (isActive())
        return;

    m_runtime = new Runtime(options);
}

void Engine::stop()
{
    delete m_runtime;
    m_runtime = nullptr;
}

void Engine::addSynth(LV2::Plugin & synth)
{
    m_synths.push_back(synth);

    if (m_runtime)
    {
        m_runtime->addSynth(synth);
    }
}

void Engine::removeSynth(int index)
{
    if (index < 0 || index >= m_synths.size())
        return;

    m_synths.erase(m_synths.begin() + index);

    if (m_runtime)
    {
        m_runtime->removeSynth(index);
    }
}

vector<LV2::Plugin> Engine::synths()
{
    return m_synths;
}

void Engine::selectActiveSynth(int index)
{
    if (m_runtime)
    {
        m_runtime->setActiveSynth(index);
    }
}



Engine::Runtime::Runtime(const Engine::Options & options)
{
    // Create JACK client

    jack_status_t status;

    jack.client = jack_client_open("dzemer", JackNoStartServer, &status);
    if (!jack.client)
    {
      throw std::runtime_error("Failed to create JACK client.");
    }

    jack.sample_rate = jack_get_sample_rate(jack.client);
    jack.buffer_size = jack_get_buffer_size(jack.client);

    // Set callbacks

    jack_set_process_callback (jack.client, jack_process_cb, this);

    // Create ports

    for (int i = 0; i < options.audioOutputCount; ++i)
    {
        auto port =
                jack_port_register (jack.client, ("out" + to_string(i)).c_str(),
                                    JACK_DEFAULT_AUDIO_TYPE,
                                    JackPortIsOutput, 0);
        if (!port)
            throw std::runtime_error("Failed to create JACK port.");

        jack.audio_out_ports.push_back(port);
    }

    {
        auto port =
                jack_port_register (jack.client, "in",
                                    JACK_DEFAULT_MIDI_TYPE,
                                    JackPortIsInput, 0);

        if (!port)
            throw std::runtime_error("Failed to create JACK port.");

        jack.midi_in_port = port;
    }

    // Activate JACK client

    if (jack_activate (jack.client)) {
      throw std::runtime_error("Could not activate JACK client.");
    }

    // Connect JACK client ports to system output ports

    auto input_ports = jack_get_ports (jack.client, NULL, NULL,
                                       JackPortIsPhysical|JackPortIsInput);

    auto input_port = input_ports;

    for (auto & port : jack.audio_out_ports)
    {
      if (!input_ports)
        break;

      cout << "Connecting port "
           << jack_port_name (port)
           << " to system port " << *input_ports
           << endl;

      if (jack_connect (jack.client, jack_port_name (port), *input_port)) {
        cerr << "Could not connect ports." << endl;
      }

      ++input_port;
    }

    jack_free (input_ports);
}

Engine::Runtime::~Runtime()
{
    jack_client_close(jack.client);

    for(auto & synth : m_synths)
    {
        delete synth;
    }
}

void
Engine::Runtime::addSynth(LV2::Plugin & plugin)
{
    auto synth = new Synth;

    synth->plugin_instance = plugin.instantiate(jack.sample_rate, {uri_mapper.feature()} );

    if (!synth->plugin_instance)
        throw std::runtime_error("Could not instantiate synth.");

    vector<int> audio_out_ports;
    vector<int> control_ports;

    for (int i = 0; i < plugin.port_count(); ++i)
    {
      auto port = plugin.port_at(i);

      cout << "Port at " << i
           << " index = " << port.index()
           << " name = " << port.name()
           << endl;

      if (port.is_audio() && port.is_output())
      {
          cout << "Audio output port." << endl;

          audio_out_ports.push_back(port.index());
      }
      else if (port.is_midi() && port.is_input())
      {
          cout << "Midi input port." << endl;
          synth->midi_in_port_index = port.index();
      }
      else if (port.is_control())
      {
        cout << "Control port." << endl;
        control_ports.push_back(port.index());
      }
      else
      {
        throw std::runtime_error("Unexpected port type.");
      }
    }

    synth->audio_out_buffers.resize(audio_out_ports.size());
    for(int port_idx : audio_out_ports)
    {
        cout << "Connecting audio out port " << port_idx << endl;
        auto & buffer = synth->audio_out_buffers[port_idx];
        buffer.resize(jack.buffer_size);
        synth->plugin_instance->connect_port(port_idx, buffer.data());
    }

    synth->control_buffers.resize(control_ports.size());
    for(int port_idx : control_ports)
    {
        cout << "Connecting control port " << port_idx << endl;
        auto port = plugin.port_at(port_idx);
        float default_value = port.predicate(LV2_CORE__default).to_float();

        synth->control_buffers[port_idx] = default_value;
        synth->plugin_instance->connect_port(port.index(), &synth->control_buffers[port_idx]);
    }

    {
        std::lock_guard<mutex> lock(m_mutex);
        m_synths.push_back(synth);
    }
}

void
Engine::Runtime::removeSynth(int index)
{
    Synth * synth;

    {
        std::lock_guard<mutex> lock(m_mutex);

        if (index < 0 || index >= m_synths.size())
            return;

        synth = m_synths[index];

        m_synths.erase(m_synths.begin() + index);

        if (index == activeSynth)
            activeSynth = -1;
    }

    delete synth;
}

void Engine::Runtime::setActiveSynth(int index)
{
    std::lock_guard<mutex> lock(m_mutex);

    if (index < 0 || index >= m_synths.size())
        return;

    activeSynth = index;
}

int Engine::Runtime::process(jack_nframes_t nframes)
{
    std::lock_guard<mutex> lock(m_mutex);

    m_bleeper.run(nframes);

    for (auto synth : m_synths)
    {
        if (synth->midi_in_port_index >= 0)
            synth->plugin_instance->connect_port(synth->midi_in_port_index, midi_buffer.data());

        synth->plugin_instance->run(nframes);

        if (synth->midi_in_port_index >= 0)
            synth->plugin_instance->connect_port(synth->midi_in_port_index, nullptr);
    }

    for (int port_idx = 0; port_idx < jack.audio_out_ports.size(); ++port_idx)
    {
        auto port = jack.audio_out_ports[port_idx];
        auto buf = (jack_default_audio_sample_t*) jack_port_get_buffer(port, nframes);

        for(int f = 0; f < nframes; ++f)
        {
            buf[f] = 0;
        }

        for (auto synth : m_synths)
        {
            if (port_idx >= synth->audio_out_buffers.size())
                continue;

            for(int f = 0; f < nframes; ++f)
            {
                buf[f] += synth->audio_out_buffers[port_idx][f];
            }
        }
    }
}

Engine::Runtime::Synth::~Synth()
{
    delete plugin_instance;
}




Bleeper::Bleeper(LV2::AtomBuffer & buffer, LV2::UriMap & uri_map, int period):
    m_buffer(buffer),
    m_uri_map(uri_map),
    m_period(period)
{

}

void Bleeper::run(int nframes)
{
    m_buffer.reset();

    if (elapsed_frames == 0 || elapsed_frames - last_note_on >= m_period)
    {
      printf("Note on.\n");

      MidiStream stream(m_buffer, m_uri_map);
      stream.noteOn(0, 0, 60, 100);

      last_note_on = elapsed_frames;
      note_is_on = true;
    }
    else if (note_is_on && elapsed_frames - last_note_on >= m_period * 0.5)
    {
        printf("Note off.\n");

        MidiStream stream(m_buffer, m_uri_map);
        stream.noteOff(0, 0, 60, 100);

        note_is_on = false;
    }
    else
    {
      clearBuffer();
    }

    elapsed_frames += nframes;
}

void Bleeper::clearBuffer()
{
  MidiStream stream(m_buffer, m_uri_map);
}

}
