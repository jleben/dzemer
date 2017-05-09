#include "score_player.hpp"

#include "../lv2-util/Midi.hpp"

#include <cstdio>

namespace dzemer {

Score_Player::Score_Player(LV2::UriMap & uri_map):
    m_uri_map(uri_map)
{}

void Score_Player::start(Score *score, LV2::AtomBuffer * midi_buffer, jack_nframes_t time)
{
    m_score = score;
    m_midi_buffer = midi_buffer;
    m_start_time = time;
    m_play_pos = 0;
}

void Score_Player::play(jack_nframes_t time, jack_nframes_t duration)
{
    if (!m_score)
        return;

    // Initialize output to empty sequence

    m_midi_buffer->reset();

    LV2::AtomSerializer atom(*m_midi_buffer);
    LV2::AtomSequenceSerializer sequence(atom, m_uri_map);

    if (m_play_pos >= m_score->size)
        return;

    auto score_start_time = time - m_start_time;
    auto score_end_time = score_start_time + duration;

    // FIXME: Skip missed events

    // Play events in range
    for(; m_play_pos < m_score->size; ++m_play_pos)
    {
        auto & event = m_score->buffer[m_play_pos];

        if (event.time >= score_end_time)
            break;

        auto event_play_time = event.time - score_start_time;

        printf("Play: Event, time = %ld\n", event_play_time);

        switch(event.type)
        {
        case Score_Element::Midi:
        {
            printf("Play: Midi, size = %d, %x %u %u\n", event.midi.size,
                   event.midi.data[0], event.midi.data[1], event.midi.data[2]);

            sequence << [&](LV2::AtomEventSerializer & atom_event)
            {
                atom_event.setFrames(event_play_time);

                LV2::AtomMidiSerializer::message
                        (atom_event.body(), event.midi.data, event.midi.size, m_uri_map);
            };
            break;
        }
        default:
            printf("Play: Unsupported type\n");
        }
    }
}

}
