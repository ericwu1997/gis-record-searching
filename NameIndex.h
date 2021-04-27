#pragma once
#ifndef NAME_INDEX_H
#define NAME_INDEX_H

#include <vector>
#include <string>
#include <time.h>
#include <unistd.h>

using namespace std;

struct Index
{
    unsigned int key;
    string featurename_statealpha;
    vector<long int> record_offsets;
};

class NameIndex
{
private:
    enum BucketStatus
    {
        EMPTY,
        OCCUPIED,
        DELETED
    };

    int _highestCollisionCount;
    int _size;
    int _capacity;

    vector<Index> _buckets;
    vector<BucketStatus> _status;

    void expandAndRehash()
    {
        _capacity *= 2;

        vector<Index> tmp_buckets = _buckets;
        vector<BucketStatus> tmp_status = _status;

        Index empty;
        _buckets.resize(_capacity, empty);
        _status.assign(_capacity, EMPTY);

        for (int i = 0; i < tmp_buckets.size(); i++)
            if (tmp_status[i] == OCCUPIED)
                _insert(tmp_buckets[i]);
    };

    unsigned int ELFHash(string str)
    {
        unsigned int hash = 0;
        unsigned int x = 0;
        unsigned int i = 0;
        unsigned int len = str.length();

        for (i = 0; i < len; i++)
        {
            hash = (hash << 4) + (str[i]);
            if ((x = hash & 0xF0000000L) != 0)
                hash ^= (x >> 24);
            hash &= ~x;
        }
        return hash;
    }

    // For rehash only
    void _insert(Index entry)
    {
        unsigned int key = entry.key, i = 0, index = key % _capacity;
        int collisionCunt = 0;
        while (_status[index] == OCCUPIED)
        {
            collisionCunt++, i++;
            index = (index + ((i * i + i) / 2)) % _capacity;
        }
        // Add index to hashtable
        _status[index] = OCCUPIED;
        _buckets[index] = entry;

        // Update highest collision count
        if (collisionCunt > _highestCollisionCount)
            _highestCollisionCount = collisionCunt;
    }

public:
    void insert(string featurename_statealpha, long int offset)
    {
        if (static_cast<double>(_size) / _capacity >= 0.70)
        {
            expandAndRehash();
        }

        unsigned int key = ELFHash(featurename_statealpha), i = 0, index = key % _capacity;
        int collisionCunt = 0;

        while (_status[index] == OCCUPIED)
        {
            if (_buckets[index].key == key)
            {
                bool existed = false;
                for (auto &offset_exist : _buckets[index].record_offsets)
                {
                    // Offset already exist, dont add
                    if (existed = (offset_exist == offset))
                        break;
                }
                if (!existed)
                {
                    _buckets[index].record_offsets.push_back(offset);
                    return;
                }
            }
            collisionCunt++, i++;
            // quadratic probing
            index = (index + ((i * i + i) / 2)) % _capacity;
        }

        // Create new index entry
        Index tmp;
        tmp.key = key;
        tmp.record_offsets.push_back(offset);
        tmp.featurename_statealpha = featurename_statealpha;

        // Add index to hashtable
        _status[index] = OCCUPIED;
        _buckets[index] = tmp;
        _size++;

        // Update highest collision count
        if (collisionCunt > _highestCollisionCount)
            _highestCollisionCount = collisionCunt;
    }

    vector<long int> search(string featurename_statealpha)
    {
        unsigned int key = ELFHash(featurename_statealpha), i = 0, index = key % _capacity;
        while (_status[index] == OCCUPIED)
        {
            if (_buckets[index].key == key)
            {
                return _buckets[index].record_offsets;
            }
            i++;
            // quadratic probing
            index = (index + ((i * i + i) / 2)) % _capacity;
        }
        return {};
    }

    int size()
    {
        return _size;
    }

    int longestProbe()
    {
        return _highestCollisionCount;
    }

    void resetCollisionCount()
    {
        _highestCollisionCount = 0;
    }

    string str()
    {
        stringstream ss;
        size_t i = 0;

        ss << "Format of display is " << endl;
        ss << "Slot number: data record" << endl;
        ss << "Current table size is 1024" << endl;
        ss << "Number of elements in table is " << to_string(_size) << endl;
        ss << endl;

        for (auto &s : _status)
        {
            if (s == NameIndex::BucketStatus::OCCUPIED)
            {
                Index index = _buckets[i];
                vector<long int> offsets = index.record_offsets;
                ss << "  " << to_string(i) << ": ";
                ss << "[" << index.featurename_statealpha << ", ";
                ss << "[";
                ss << to_string(offsets[0]);
                if (offsets.size() > 1)
                {
                    int j = 1;
                    for (; offsets.size() > j; j++)
                        ss << ", " << offsets[j];
                }
                ss << "]]" << endl;
            }
            i++;
        }
        return ss.str();
    }

    NameIndex(size_t capacity) : _capacity(capacity)
    {
        _highestCollisionCount = 0;
        _size = 0;
        _buckets.resize(_capacity);
        _status.resize(_capacity);
    };
    NameIndex() {}
    ~NameIndex() {}
};

#endif