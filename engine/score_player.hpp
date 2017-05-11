#pragma once

#include "score.hpp"
#include "../lv2-util/Atom.hpp"
#include "../lv2-util/UriMap.hpp"

#include <jack/jack.h>

namespace dzemer {

class Score_Player
{
public:
    Score_Player(LV2::UriMap & uri_map);
    void start(Score * score, LV2::AtomBuffer * midi_buffer,
               jack_nframes_t time, jack_nframes_t loop_duration = 0);
    void stop();
    void play(jack_nframes_t time, jack_nframes_t duration);

private:
    void play(const Score_Element &, int64_t time, LV2::AtomSequenceSerializer &);
    int64_t distance(int64_t a, int64_t b);

    LV2::UriMap & m_uri_map;
    Score * m_score = nullptr;
    LV2::AtomBuffer * m_midi_buffer = nullptr;
    jack_nframes_t m_start_time = 0;
    jack_nframes_t m_loop_duration = 0;

    int m_next_event_index = 0;
    jack_nframes_t m_next_event_time = 0;
};

}
