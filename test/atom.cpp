#include "test.hpp"

#include "../lv2-util/Atom.hpp"
#include "../lv2-util/Midi.hpp"
#include "../util.hpp"

#include <iostream>

using namespace dzemer::testing;
using namespace std;

void hello()
{
  cout << "Hello." << endl;
}

void sequence()
{
  using namespace LV2;

  dzemer::UriMapper uri_map;

  AtomBuffer buffer(512);
  AtomSerializer atom(buffer);
  AtomSequenceSerializer sequence(atom, uri_map);
  sequence << [&](AtomEventSerializer & event){
    event.setFrames(111);
    AtomIntSerializer i(event.body(), 12345, uri_map);
  };
  sequence << [&](AtomEventSerializer & event){
    event.setFrames(555);
    AtomMidiSerializer::noteOn(event.body(), 5, 61, 78, uri_map);
  };

  unsigned char * d = (unsigned char*) buffer[0];

  {
    auto atom = (LV2_Atom*)d;
    assert(atom->type == uri_map[LV2_ATOM__Sequence], "Atom type is sequence.");

    auto seq = (LV2_Atom_Sequence*)d;
    auto seq_size = seq->atom.size;

    int pos = 0;
    {
      assert(pos + sizeof(LV2_Atom_Event) < seq_size, "Event 1 fits into sequence.");

      LV2_Atom_Event * event = (LV2_Atom_Event*)(d + sizeof(LV2_Atom_Sequence) + pos);

      assert(event->time.frames == 111, "Event frames = 111");
      assert(event->body.type == uri_map[LV2_ATOM__Int], "Event data is int.");

      LV2_Atom_Int * data = (LV2_Atom_Int*) &event->body;

      assert(data->body == 12345, "Event data is 12345");

      pos += sizeof(LV2_Atom_Event) + lv2_atom_pad_size(event->body.size);
    }
    {
      assert(pos + sizeof(LV2_Atom_Event) < seq_size, "Event 2 fits into sequence.");

      LV2_Atom_Event * event = (LV2_Atom_Event*)(d + sizeof(LV2_Atom_Sequence) + pos);

      assert(event->time.frames == 555, "Event frames = 555");
      assert(event->body.type == uri_map[LV2_MIDI__MidiEvent], "Event data type is MIDI.");
      assert(event->body.size == 3, "Event data size is 3.");

      uint8_t * msg = (uint8_t*) ((char*) event + sizeof(LV2_Atom_Event));

      assert(lv2_midi_is_voice_message(msg), "MIDI message is voice message.");
      assert(lv2_midi_message_type(msg) == LV2_MIDI_MSG_NOTE_ON, "MIDI message type is note-on.");

      int channel = msg[0] & 0xF;
      int key = msg[1];
      int velocity = msg[2];

      assert(channel == 5, "Channel = 5.");
      assert(key == 61, "Key = 61.");
      assert(velocity == 78, "Velocity = 78");

      pos += sizeof(LV2_Atom_Event) + lv2_atom_pad_size(event->body.size);
    }
  }
}

int main(int argc, char * argv[])
{
  driver d;

  d.add_test("hello", &hello);
  d.add_test("sequence", sequence);

  return d.run(argc, argv);
}
