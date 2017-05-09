#pragma once

#include "score.hpp"
#include "score_recorder.hpp"
#include "score_player.hpp"
#include "../lv2-util/Plugin.hpp"
#include "../lv2-util/Atom.hpp"
#include "../lv2-util/Midi.hpp"
#include "../util.hpp"

#include <jack/jack.h>

#include <mutex>
#include <vector>
#include <list>

namespace dzemer {

using std::mutex;
using std::vector;
using std::list;

class Bleeper
{
public:
    Bleeper(LV2::AtomBuffer & buffer, LV2::UriMap & uri_map, int period);
    void run(int nframes);
private:
    void clearBuffer();

    LV2::AtomBuffer & m_buffer;
    LV2::UriMap & m_uri_map;
    jack_nframes_t m_period;
    jack_nframes_t last_note_on =  0;
    bool note_is_on = false;
    jack_nframes_t elapsed_frames = 0;
};

class Engine
{
public:
    struct Options
    {
        int audioOutputCount = 2;
    };

    Engine() {}
    ~Engine();

    void addSynth(LV2::Plugin & plugin);
    void removeSynth(int index);
    vector<LV2::Plugin> synths();
    void selectActiveSynth(int index);

    void start(const Options &);
    void stop();
    bool isActive() const { return m_runtime != nullptr; }

    //void startRecording();
    //void stopRecording();

private:

    vector<LV2::Plugin> m_synths;

    struct Synth
    {
        LV2::Plugin plugin;
    };

    struct Runtime
    {
        Runtime(const Engine::Options &);
        ~Runtime();

        struct Synth
        {
            Synth(LV2::UriMap & uri_map):
                // FIXME: Adjust buffer capacities
                midi_in_buffer(1024),
                score(100),
                score_player(uri_map)
            {}

            ~Synth();

            LV2::PluginInstance * plugin_instance;
            vector<float> control_buffers;
            vector<vector<float>> audio_out_buffers;
            int midi_in_port_index = -1;
            LV2::AtomBuffer midi_in_buffer;

            Score score;
            Score_Player score_player;
            Score_Recorder score_recorder;
        };

        // Thread-safe

        double sampleRate() const { return jack.sample_rate; }
        int bufferSize() const { return jack.buffer_size; }
        // FIXME: update buffer size when it changes in JACK

        void addSynth(LV2::Plugin &);
        void removeSynth(int index);

        void setActiveSynth(int index);

    private:

        UriMapper uri_mapper;

        std::mutex m_mutex;

        struct {
            jack_client_t * client = nullptr;
            jack_port_t * midi_in_port = nullptr;
            vector<jack_port_t*> audio_out_ports;
            double sample_rate = 0;
            int buffer_size = 0;
        } jack;

        vector<Synth*> m_synths;
        int activeSynth = -1;

        LV2::AtomBuffer midi_buffer { 1024 };

        Bleeper m_bleeper { midi_buffer, uri_mapper, 44100 };

        int process(jack_nframes_t nframes);

        static int jack_process_cb(jack_nframes_t nframes, void *arg)
        {
          return reinterpret_cast<Runtime*>(arg)->process(nframes);
        }
    };

    Runtime * m_runtime = nullptr;
};

}
