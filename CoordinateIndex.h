#pragma once
#ifndef COORDINATE_INDEX_H
#define COORDINATE_INDEX_H

#include <vector>
#include <iostream>
#include <sstream>
#include <unistd.h>

using namespace std;

#define MAX_HEIGHT(n, m) ((n > m) ? n : m)

class CoordinateIndex
{
private:
    enum IndexStatus
    {
        EMPTY,
        OCCUPIED,
        SPLITED
    };
    struct Index
    {
        vector<long int> offsets;
        vector<long int> coordinate;
        size_t height = 0;
        vector<Index> childs;    // leaf node
        vector<long int> region; // 2 pair of coordinate to represent a region
        IndexStatus status;
    };
    Index _root;
    vector<Index *> path;
    vector<Index> subset;
    size_t _size;

    bool _insert(long int x, long int y, vector<long int> offsets, Index *index)
    {
        while (true)
        {
            // Attempt to insert data in empty node
            if (index->status == EMPTY)
            {
                // Record traverse trace
                path.push_back(index);
                if (x >= index->region[0] && y >= index->region[1])
                    if (x <= index->region[2] && y <= index->region[3])
                    {
                        index->coordinate = {x, y};
                        for (auto &offset : offsets)
                            index->offsets.push_back(offset);
                        index->status = OCCUPIED;
                        _size++;
                        return true;
                    }
                // out of bound
                return false;
            }

            // Travese into corresponding quadrant
            if (index->status == SPLITED)
            {
                // Record traverse trace
                path.push_back(index);
                int i;
                for (i = 0; i < 4; i++)
                {
                    Index *child = &(index->childs[i]);
                    vector<long int> region = child->region;
                    if (x >= region[0] && y >= region[1])
                        if (x <= region[2] && y <= region[3])
                        {
                            index = child;
                            break;
                        }
                }
                // no corresponding quarant found (out of bound)
                if (i == 4)
                    return false;
            }

            // Split current node to four quadrants
            if (index->status == OCCUPIED)
            {
                // Same coordinate, append offset to collection
                if (index->coordinate[0] == x && index->coordinate[1] == y)
                {
                    for (auto &offset : offsets)
                        index->offsets.push_back(offset);
                    return true;
                }
                else
                {
                    vector<long int> parent_region = index->region;

                    Index temp;
                    temp.status = EMPTY;
                    long int x1 = parent_region[0], x2 = parent_region[2];
                    long int y1 = parent_region[1], y2 = parent_region[3];

                    temp.region = {x1, y1, x1 + (x2 - x1) / 2, y1 + (y2 - y1) / 2};
                    index->childs.push_back(temp);

                    temp.region = {x1 + (x2 - x1) / 2, y1, x2, y1 + (y2 - y1) / 2};
                    index->childs.push_back(temp);

                    temp.region = {x1, y1 + (y2 - y1) / 2, x1 + (x2 - x1) / 2, y2};
                    index->childs.push_back(temp);

                    temp.region = {x1 + (x2 - x1) / 2, y1 + (y2 - y1) / 2, x2, y2};
                    index->childs.push_back(temp);

                    index->status = SPLITED;

                    _size--;
                    _insert(index->coordinate[0], index->coordinate[1], index->offsets, index);
                }
            }
        }
    };

    void updateHeight()
    {
        Index *node;
        // Update node in traverse path from bottom to top
        for (int i = path.size() - 1; i >= 0; i--)
        {
            node = (Index *)path[i];
            if (node->status == SPLITED)
            {
                // Find the child with max height
                size_t max_height = node->childs[0].height;
                for (int j = 1; j < 4; j++)
                {
                    max_height = MAX_HEIGHT(max_height, node->childs[j].height);
                }
                // Assign parent height with (child max height + 1)
                node->height = ++max_height;
            }
        }
    }

    void _str(stringstream &ss, Index node, vector<bool> parent_last_child)
    {
        stringstream s("");
        for (int i = 0; i < parent_last_child.size() - 1; i++)
        {
            if (parent_last_child[i])
                ss << "    ";
            else
                ss << "|   ";
        }
        ss << "|_@_" << endl;
        vector<Index> childs = (vector<Index>)node.childs;
        for (int i = 0; i < parent_last_child.size(); i++)
        {
            if (parent_last_child[i])
                s << "    ";
            else
                s << "|   ";
        }
        string spacing = s.str();

        for (int j = 0; j < 4; j++)
        {
            if (childs[j].status == EMPTY)
                ss << spacing << "|__ [EMPTY]" << endl;
            else if (childs[j].status == OCCUPIED)
            {
                ss << spacing << "|__ ";
                ss << "[(" << to_string(childs[j].coordinate[0]);
                ss << ", " << to_string(childs[j].coordinate[1]);
                ss << ", [ ";
                for (auto &offset : childs[j].offsets)
                    ss << to_string(offset) << " ";
                ss << "]" << endl;
            }
            else
            {
                vector<bool> tmp = parent_last_child;
                tmp.push_back((j == 3));
                _str(ss, childs[j], tmp);
            }
        }
        return;
    }

    void _returnAllIndex(Index index)
    {
        if (index.status == SPLITED)
        {
            _returnAllIndex(index.childs[0]);
            _returnAllIndex(index.childs[1]);
            _returnAllIndex(index.childs[2]);
            _returnAllIndex(index.childs[3]);
        }
        else if (index.status == OCCUPIED)
            subset.push_back(index);
    }

public:
    void insert(long int x, long int y, long int data)
    {
        bool check = _insert(x, y, {data}, &_root);
        updateHeight();
        path.clear();
    };

    vector<long int> search(long int x, long int y)
    {
        Index node = _root;
        while (true)
        {
            if (node.status == OCCUPIED)
            {
                if (x == node.coordinate[0] && y == node.coordinate[1])
                    return node.offsets;
                return {};
            }
            if (node.status == EMPTY)
            {
                // Specified entry not found
                return {};
            }
            do
            {
                int i = 0;
                for (i = 0; i < 4; i++)
                {
                    vector<long int> region = node.childs[i].region;
                    if (x >= region[0] && y >= region[1])
                        if (x <= region[2] && y <= region[3])
                        {
                            // Type casting (Node) here is necessary
                            node = (Index)node.childs[i];
                            break;
                        }
                }
            } while (node.status == SPLITED);
        }
        cout << "ERROR" << endl;
        return {};
    }

    size_t size()
    {
        return _size;
    }

    string str()
    {
        stringstream ss;
        if (_root.status != SPLITED)
        {
            if (_root.status == OCCUPIED)
            {
                ss << "|_@_\n"
                   << "    |__";
                for (auto &offset : _root.offsets)
                    ss << to_string(offset) << "  ";
            }
            else
                ss << "|_@_\n"
                   << "    |__ [EMPTY]";
        }
        else
            _str(ss, _root, {true});
        return ss.str();
    }

    vector<long int> findSubset(long int min_x, long int min_y, long int max_x, long int max_y)
    {
        Index index = _root, parent, child;

        // Find the smallest quadrant containing the searching area
        int i;
        while (index.status == SPLITED && i != 4)
        {
            // Check all four quadrants
            for (i = 0; i < 4; i++)
            {
                child = index.childs[i];
                if (min_x >= child.region[0] && min_y >= child.region[1])
                    if (max_x <= child.region[2] && max_y <= child.region[3])
                    {
                        index = child;
                        break;
                    }
            }
        }

        // Check all index in the acquired quadrant
        subset.clear();
        _returnAllIndex(index);
        vector<long int> results;
        // cout << to_string(index.region[0]) << ", " << to_string(index.region[1]) << endl;
        // cout << to_string(subset.size()) << endl;
        for (auto &location : subset)
        {
            if (location.coordinate[0] >= min_x && location.coordinate[1] >= min_y)
                if (max_x >= location.coordinate[0] && max_y >= location.coordinate[1])
                {
                    for (auto &offset : location.offsets)
                        results.push_back(offset);
                }
        }
        subset.clear();
        return results;
    }

    CoordinateIndex(long int x_min, long int y_min, long int x_max, long int y_max)
    {
        _size = 0;
        _root.height = 0;
        _root.status = EMPTY;
        _root.region = {x_min, y_min, x_max, y_max};
    };
    ~CoordinateIndex(){};
};

#endif