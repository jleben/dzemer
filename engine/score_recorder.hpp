#pragma once

#include "score.hpp"

#include <jack/jack.h>

namespace dzemer {

class Score_Recorder
{
public:
    void start(Score * score, jack_port_t *, jack_nframes_t time);
    void stop() { m_score = nullptr; }
    void record(jack_nframes_t time, jack_nframes_t duration);

private:
    Score * m_score = nullptr;
    jack_port_t * m_port = nullptr;
    jack_nframes_t m_start_time = 0;
    int m_write_pos = 0;
};

}
