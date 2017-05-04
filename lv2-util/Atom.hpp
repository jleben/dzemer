#pragma once

#include "UriMap.hpp"

#include <atom.lv2/atom.h>
#include <atom.lv2/util.h>

#include <vector>

namespace LV2 {

using std::vector;

class AtomBuffer
{
public:
  struct OutOfMemory {};

  AtomBuffer(size_t size):
    m_data(new unsigned char [size]),
    m_size(size)
  {}

  ~AtomBuffer()
  {
    delete[] m_data;
  }

  template <typename T>
  size_t append(const T & value)
  {
    auto pos = allocate(sizeof(T));
    get<T>(pos) = value;
    return pos;
  }

  template <typename T>
  size_t allocate()
  {
    return allocate(sizeof(T));
  }

  size_t allocate(size_t count)
  {
    if (m_pos + count > m_size)
      throw OutOfMemory();

    auto pos = m_pos;
    m_pos += count;

    return pos;
  }

  void reset()
  {
    m_pos = 0;
  }

  void * operator[](size_t pos) { return m_data + pos; }

  template <typename T>
  T & get(size_t pos)
  {
    return *reinterpret_cast<T*>(m_data + pos);
  }

  size_t position(void * ptr) { return reinterpret_cast<unsigned char*>(ptr) - m_data; }

  size_t capacity() const { return m_size; }

  size_t size() const { return m_pos; }

  void * data() const { return m_data; }

private:
  unsigned char * m_data = nullptr;
  size_t m_size = 0;
  size_t m_pos = 0;
};

class AtomSerializer
{
  AtomBuffer & m_buffer;
  size_t m_pos;

public:
  AtomSerializer(AtomBuffer & buffer, size_t pos): m_buffer(buffer), m_pos(pos) {}

  AtomSerializer(AtomBuffer & buffer): m_buffer(buffer)
  {
    m_pos = m_buffer.allocate<LV2_Atom>();
  }

  size_t pos() const { return m_pos; }

  LV2_Atom & atom() const { return m_buffer.get<LV2_Atom>(m_pos); }

  AtomBuffer & buffer() const { return m_buffer; }
};

class AtomEventSerializer
{
  AtomBuffer & m_buffer;
  size_t m_pos;

  LV2_Atom_Event & event() { return m_buffer.get<LV2_Atom_Event>(m_pos); }

public:

  uint32_t size()
  {
    return sizeof(LV2_Atom_Event) + event().body.size;
  }

  AtomEventSerializer(AtomBuffer & buf): m_buffer(buf)
  {
    m_pos = m_buffer.allocate<LV2_Atom_Event>();
  }

  void setFrames(int64_t frames)
  {
    event().time.frames = frames;
  }

  void setBeats(double beats)
  {
    event().time.beats = beats;
  }

  AtomSerializer body()
  {
    return AtomSerializer(m_buffer, m_buffer.position(&event().body));
  }
};

class AtomSequenceSerializer
{
  AtomBuffer & m_buffer;
  size_t m_pos;

  LV2_Atom_Sequence & sequence() { return m_buffer.get<LV2_Atom_Sequence>(m_pos); }

public:

  AtomSequenceSerializer(const AtomSerializer & atom, UriMap & map):
    m_buffer(atom.buffer()),
    m_pos(atom.pos())
  {
    m_buffer.allocate<LV2_Atom_Sequence_Body>();

    sequence().atom.size = sizeof(LV2_Atom_Sequence_Body);
    sequence().atom.type = map[LV2_ATOM__Sequence];
    sequence().body.unit = 0;
    sequence().body.pad = 0;
  }

  void setUnit(uint32_t unit)
  {
    sequence().body.unit = unit;
  }

  template <typename F>
  AtomSequenceSerializer & operator<<(F f)
  {
    AtomEventSerializer event_serializer(m_buffer);

    f(event_serializer);

    auto padded_size = lv2_atom_pad_size(event_serializer.size());
    auto padding_size =  padded_size - event_serializer.size();
    if (padding_size > 0)
      m_buffer.allocate(padding_size);

    sequence().atom.size += padded_size;

    return *this;
  }
};

class AtomIntSerializer
{
public:
  AtomIntSerializer(const AtomSerializer & atom, int32_t value, UriMap & map)
  {
    atom.atom().size = sizeof(int32_t);
    atom.atom().type = map[LV2_ATOM__Int];
    atom.buffer().append(value);
  }
};

}
