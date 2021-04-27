#pragma once
#ifndef LOGGER_H
#define LOGGER_H

#include <time.h>

class Logger
{
private:
    ofstream outfile;

public:
    void open(string file)
    {
        outfile.open(file, ofstream::trunc | ofstream::out);
    }

    void close()
    {
        outfile.close();
        outfile.clear();
    }

    void split()
    {
        outfile << "------------------------------------------------------------------------------------------" << endl;
    }

    void addDescription(string s)
    {
        outfile << s << endl;
    }

    void addTimeStamp()
    {
        time_t now;
        struct tm *info;
        char buf[256];
        time(&now);
        info = localtime(&now);
        strftime(buf, 256, "%a %b %d %T %Z %Y", info);
        outfile << buf << endl;
    }

    void addTimeStamp(string s)
    {
        outfile << s;
        addTimeStamp();
    }

    ofstream &operator<<(string log)
    {
        outfile << log;
        return outfile;
    }

    Logger(){};
    ~Logger(){};
};

#endif