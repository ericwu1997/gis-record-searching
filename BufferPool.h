#pragma once
#ifndef BUFFER_POOL_H
#define BUFFER_POOL_H

#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>
#include "GISRecord.h"

using namespace std;

template <typename T>
class LRU
{
protected:
    struct PageFrame
    {
        T record;
        size_t clock_tick;
    };
    vector<PageFrame> page;
    size_t empty_frame_available;
    size_t current_clock;

    virtual bool match(T record_1, T record_2) = 0;
    virtual T fetch(T target) = 0;
    virtual string str() = 0;

public:
    T refer(T record)
    {
        PageFrame tmp;
        tmp.clock_tick = current_clock;
        PageFrame *least_used_frame = &tmp;

        for (auto &frame : page)
        {
            // find the oldest frame
            if (least_used_frame->clock_tick > frame.clock_tick)
                least_used_frame = &frame;

            // check if frame has the target record
            if (match(frame.record, record))
            {
                frame.clock_tick = ++current_clock;
                return frame.record;
            }
        }

        // Target data not in cache, fetch from data source
        T result = fetch(record);
        if (full())
        {
            least_used_frame->record = result;
            least_used_frame->clock_tick = ++current_clock;
        }
        else
        {
            PageFrame newFrame = {result, ++current_clock};
            page.push_back(newFrame);
            empty_frame_available--;
        }
        return result;
    }

    bool full()
    {
        return (empty_frame_available == 0);
    }

    LRU(size_t size)
    {
        page.reserve(size);
        empty_frame_available = size;
        current_clock = 0;
    };
    ~LRU(){};
};

struct Entry
{
    long int offset;
    GISRecord data;
};

// GISRecord Cache
class BufferPool : public LRU<Entry>
{
private:
    ifstream ifile;

    bool match(Entry entry1, Entry entry2)
    {
        return entry1.offset == entry2.offset;
    }

    Entry fetch(Entry entry)
    {
        string line;
        ifile.seekg(entry.offset, ios::beg);
        getline(ifile, line);
        GISRecord record(line);
        entry.data = record;
        return entry;
    }

    struct less_than_tick
    {
        inline bool operator()(const PageFrame &frame1, const PageFrame &frame2)
        {
            return (frame1.clock_tick > frame2.clock_tick);
        }
    };

public:
    GISRecord
    refer(long int offset)
    {
        Entry entry;
        entry.offset = offset;
        return LRU::refer(entry).data;
    }

    string str()
    {
        stringstream ss;
        ss << "MRU" << endl;
        sort(page.begin(), page.end(), less_than_tick());
        for (auto &frame : page)
        {
            ss << "  " << to_string(frame.record.offset) << ": ";
            ss << frame.record.data.str() << endl;
        }
        ss << "LRU" << endl;
        return ss.str();
    }

    BufferPool(size_t size, string db_file) : LRU(size)
    {
        ifile.open(db_file);
        if (!ifile.is_open())
            cout << "Error opening file: " << db_file << endl;
    };

    ~BufferPool()
    {
        ifile.close();
    }
};

#endif