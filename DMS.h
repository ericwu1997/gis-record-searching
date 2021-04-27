#pragma once
#ifndef DMS_H
#define DMS_H

#include <string>
#include <iostream>
using namespace std;

class DMS
{
private:
    int _degree;
    int _minute;
    int _second;
    char _direction;

public:
    int getDeg();
    int getMin();
    int getSec();
    char getDir();
    DMS(string coordinate);
    ~DMS();
    int toSecond();
};

int DMS::getDeg()
{
    return _degree;
}
int DMS::getMin()
{
    return _minute;
}
int DMS::getSec()
{
    return _second;
}
char DMS::getDir()
{
    return _direction;
}

int DMS::toSecond()
{
    int sign = (_direction == 'W' || _direction == 'S' ? -1 : 1);
    return (_degree * 3600 + _minute * 60 + _second) * sign;
}

DMS::DMS(string coordinate)
{
    if (coordinate.length() == 8)
    {
        _direction = coordinate.at(7);
        _degree = stoi(coordinate.substr(0, 3));
        _minute = stoi(coordinate.substr(3, 2));
        _second = stoi(coordinate.substr(5, 2));
    }
    else if (coordinate.length() == 7)
    {
        _direction = coordinate.at(6);
        _degree = stoi(coordinate.substr(0, 2));
        _minute = stoi(coordinate.substr(2, 2));
        _second = stoi(coordinate.substr(4, 2));
    }
}
DMS::~DMS() {}

#endif