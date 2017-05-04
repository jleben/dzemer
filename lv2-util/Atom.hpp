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
    data.resize(data.size() + count);
    return data.size() - count;
  }

  void * operator[](size_t pos) { return data.data() + pos; }

  template <typename T>
  T & get(size_t pos)
  {
    return *reinterpret_cast<T*>(data.data() + pos);
  }

  size_t position(void * ptr) { return reinterpret_cast<unsigned char*>(ptr) - data.data(); }

  size_t size() const { return data.size(); }

private:
  vector<unsigned char> data;
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

public:

  LV2_Atom_Event & event() { return m_buffer.get<LV2_Atom_Event>(m_pos); }

  uint32_t size()
  {
    return sizeof(LV2_Atom_Event) + event().body.size;
  }

  AtomEventSerializer(AtomBuffer & buf): m_buffer(buf)
  {
    m_pos = m_buffer.allocate<LV2_Atom_Event>();
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

public:

  LV2_Atom_Sequence & sequence() { return m_buffer.get<LV2_Atom_Sequence>(m_pos); }

  AtomSequenceSerializer(const AtomSerializer & atom, UriMap & map):
    m_buffer(atom.buffer()),
    m_pos(atom.pos())
  {
    m_buffer.allocate<LV2_Atom_Sequence_Body>();

    sequence().atom.size = sizeof(LV2_Atom_Sequence_Body);
    sequence().atom.type = map[LV2_ATOM__Sequence];
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
