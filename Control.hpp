#pragma once

#include "lv2-util/Atom.hpp"
#include "lv2-util/Midi.hpp"
#include "lv2-util/UriMap.hpp"

namespace dzemer {

#if 0
class MidiBuffer
{
public:

  struct AtomTypes
  {
    uint32_t MidiNoteOn;
    uint32_t MidiNoteOff;
  };

  MidiBuffer(size_t size, LV2::UriMap & uri_map):
    m_buffer(size)
  {
  }

  void * data() { return m_buffer.data(); }

  void clear()
  {
    m_buffer.reset();
    // FIXME: Write a null atom?
  }

  void noteOn(int channel, int key, int velocity);

  LV2::AtomBuffer & buffer() { return m_buffer; }

private:
  LV2::AtomBuffer m_buffer;
  AtomTypes m_atom_types;
};
#endif

class MidiStream
{
public:
  MidiStream(LV2::AtomBuffer & buffer, LV2::UriMap & uri_map):
    uri_map(uri_map),
    atom(buffer),
    sequence(atom, uri_map)
  {}

  MidiStream & noteOn(int frames, int channel, int key, int velocity)
  {
    sequence << [&](LV2::AtomEventSerializer & elem)
    {
      elem.setFrames(frames);
      LV2::AtomMidiSerializer::noteOn(elem.body(), channel, key, velocity, uri_map);
    };
  }

  MidiStream & noteOff(int frames, int channel, int key, int velocity)
  {
    sequence << [&](LV2::AtomEventSerializer & elem)
    {
      elem.setFrames(frames);
      LV2::AtomMidiSerializer::noteOff(elem.body(), channel, key, velocity, uri_map);
    };
  }

private:
  LV2::UriMap & uri_map;
  LV2::AtomSerializer atom;
  LV2::AtomSequenceSerializer sequence;
};

}
