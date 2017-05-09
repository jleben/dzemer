#include "score_recorder.hpp"

#include <midi.lv2/midi.h>
#include <jack/midiport.h>
#include <algorithm>
#include <cstdio>

using namespace std;

namespace dzemer {

void Score_Recorder::start(Score * score, jack_port_t * port, jack_nframes_t time)
{
    m_score = score;
    m_port = port;
    score->size = 0;
    m_start_time = time;
    m_write_pos = 0;
}

void Score_Recorder::record(jack_nframes_t time, jack_nframes_t duration)
{
    if (!m_score)
        return;

    auto source = jack_port_get_buffer(m_port, duration);

    auto num_events = jack_midi_get_event_count(source);

    jack_nframes_t recording_time = time - m_start_time;

    for (int i = 0;
         i < (int) num_events && m_write_pos < m_score->buffer.size();
         ++i)
    {
        printf("Rec: Event\n");

        auto & event = m_score->buffer[m_write_pos];

        jack_midi_event_t jack_event;
        int error = jack_midi_event_get(&jack_event, source, i);
        if (error)
            break;

        auto midi_data = jack_event.buffer;
        auto midi_type = lv2_midi_message_type(midi_data);

        switch(midi_type)
        {
        case LV2_MIDI_MSG_NOTE_OFF:
        case LV2_MIDI_MSG_NOTE_ON:
        {
            printf("Rec: Note, size = %lu, %x %u %u\n", jack_event.size,
                   midi_data[0], midi_data[1], midi_data[2]);

            event.time = recording_time + jack_event.time;
            event.type = Score_Element::Midi;
            event.midi.size = jack_event.size;

            std::copy(midi_data, midi_data + jack_event.size, event.midi.data);

            ++m_write_pos;
            break;
        }
        case LV2_MIDI_MSG_CONTROLLER:
        case LV2_MIDI_MSG_BENDER:
        {
            // TODO: Record control event
        }
        default:
            printf("Rec: Unkown type\n");
        }
    }

    m_score->size += m_write_pos;
}

}
