#pragma once

#include <vector>
#include <cstdint>
#include <cstddef>

namespace dzemer {

using std::vector;
using std::int64_t;
using std::size_t;

struct Score_Element
{
    enum Type {
        Midi,
        Control
    };

    int64_t time;

    Type type;

    struct ControlData
    {
        int port;
        float value;
    };

    struct MidiData
    {
        int size;
        unsigned char data[3];
    };

    union
    {
        ControlData control;
        MidiData midi;
    };
};

struct Score
{
    Score(size_t capacity):
        buffer(capacity)
    {}

    size_t size = 0;
    vector<Score_Element> buffer;
};

}
