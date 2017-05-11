#include "score_player.hpp"

#include "../lv2-util/Midi.hpp"

#include <cstdio>

namespace dzemer {

Score_Player::Score_Player(LV2::UriMap & uri_map):
    m_uri_map(uri_map)
{}

void Score_Player::start(Score *score, LV2::AtomBuffer * midi_buffer,
                         jack_nframes_t time, jack_nframes_t loop_duration)
{
    m_score = score;
    m_midi_buffer = midi_buffer;
    m_start_time = time;
    m_loop_duration = loop_duration;

    m_next_event_index = -1;
    m_next_event_time = 0;
}

void Score_Player::play(jack_nframes_t time, jack_nframes_t duration)
{
    if (!m_score)
        return;

    // Initialize output to empty sequence

    m_midi_buffer->reset();

    LV2::AtomSerializer atom(*m_midi_buffer);
    LV2::AtomSequenceSerializer sequence(atom, m_uri_map);

    if (m_score->size < 1)
        return;

    bool loop = m_loop_duration > 0;

    // FIXME: Allow a small delay

    // Search for next event

    if (m_next_event_time < time || m_next_event_index < 0)
    {
        auto score_time = (time - m_start_time);
        if (loop)
            score_time = score_time % m_loop_duration;

        printf("Play: Searching for next event at %u.\n", score_time);

        m_next_event_index = m_score->next_event(score_time);

        printf("Play: Found %d.\n", m_next_event_index);

        if (m_next_event_index == -1)
        {
            if (loop)
                m_next_event_index = 0;
            else
                return;
        }

        m_next_event_time = time + distance(m_score->buffer[m_next_event_index].time, score_time);
    }

    // Play events in range

    for(;;)
    {
        auto play_time = int64_t(m_next_event_time) - time;

        if (play_time >= duration)
            break;

        auto & event = m_score->buffer[m_next_event_index];

        printf("Play: Event %d/%lu, score time = %ld, real time = %u, play time = %ld\n",
               m_next_event_index, m_score->size,
               event.time,
               m_next_event_time,
               play_time);

        play(event, play_time, sequence);

        ++m_next_event_index;

        if (m_next_event_index >= m_score->size)
        {
            if (loop)
            {
                printf("Play: Looping (End of score).\n");
                m_next_event_index = 0;
            }
            else
            {
                m_next_event_index = -1;
                break;
            }
        }
        else
        {
            if (loop && m_score->buffer[m_next_event_index].time >= m_loop_duration)
            {
                printf("Play: Looping (Next event outside loop).\n");
                m_next_event_index = 0;
            }
        }

        auto & next_event = m_score->buffer[m_next_event_index];

        m_next_event_time += distance(next_event.time, event.time);

        printf("Play: Next at score time %ld, real time %u.\n", next_event.time, m_next_event_time);
    }
}

void Score_Player::play(const Score_Element & event, int64_t time, LV2::AtomSequenceSerializer & sequence)
{
    time = std::max(int64_t(0), time);

    switch(event.type)
    {
    case Score_Element::Midi:
    {
        printf("Play: Midi, size = %d, %x %u %u\n", event.midi.size,
               event.midi.data[0], event.midi.data[1], event.midi.data[2]);

        sequence << [&](LV2::AtomEventSerializer & atom_event)
        {
            atom_event.setFrames(time);

            LV2::AtomMidiSerializer::message
                    (atom_event.body(), event.midi.data, event.midi.size, m_uri_map);
        };
        break;
    }
    default:
        printf("Play: Unsupported type\n");
    }
}

int64_t Score_Player::distance(int64_t a, int64_t b)
{
    // FIXME: allow a and b outside loop range

    auto d = a - b;

    if (d < 0 && m_loop_duration > 0)
        d += m_loop_duration;

    return d;
}

}
