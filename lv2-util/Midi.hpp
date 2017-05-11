#pragma once

#include "Atom.hpp"
#include "UriMap.hpp"

#include <midi.lv2/midi.h>

namespace LV2 {

class AtomMidiSerializer
{
public:
    static
    void message(const AtomSerializer & dst, const void * data, int size, UriMap & map)
    {
        dst.atom().type = map[LV2_MIDI__MidiEvent];
        dst.atom().size = size;

        auto pos = dst.buffer().allocate(size);
        memcpy(dst.buffer()[pos], data, size);
    }

  static
  void noteOn(const AtomSerializer & dst, uint8_t channel, uint8_t key, uint8_t velocity, UriMap & map)
  {
    dst.atom().type = map[LV2_MIDI__MidiEvent];
    dst.atom().size = 3;

    uint8_t status = 0;
    status |= 0x90;
    status |= (channel & 0xF);

    //printf("Appending MIDI status: %X\n", status);

    dst.buffer().append(status);

    dst.buffer().append(key);

    dst.buffer().append(velocity);

    //printf("Status: %X\n", dst.buffer().get<uint8_t>(dst.buffer().size() - 3));
  }

  static
  void noteOff(const AtomSerializer & dst, uint8_t channel, uint8_t key, uint8_t velocity, UriMap & map)
  {
    dst.atom().type = map[LV2_MIDI__MidiEvent];
    dst.atom().size = 3;

    uint8_t status = 0;
    status |= 0x80;
    status |= (channel & 0xF);

    dst.buffer().append(status);

    dst.buffer().append(key);

    dst.buffer().append(velocity);
  }
};

}
