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

    int next_event(int64_t time)
    {
        // Assuming events are sorted by time;

        if (size == 0)
            return -1;

        int min = 0;
        int max = size - 1;

        if (time <= buffer[min].time)
            return min;
        if (time == buffer[max].time)
            return max;
        if (time > buffer[max].time)
            return -1;

        while(min < max)
        {
            int mid = (max + min) / 2;
            if (time == buffer[mid].time)
            {
                max = min = mid;
            }
            else if (time > buffer[mid].time)
            {
                min = mid + 1;
            }
            else
            {
                max = mid - 1;
            }
        }

        return max;
    }

    size_t size = 0;
    vector<Score_Element> buffer;
};

}
