#pragma once
#ifndef COMMAND_PROCESSOR_H
#define COMMAND_PROCESSOR_H

#include <fstream>
#include <vector>
#include <sstream>

using namespace std;

enum Command
{
    WORLD,
    IMPORT,
    DEBUG,
    QUIT,
    WHAT_IS_AT,
    WHAT_IS,
    WHAT_IS_IN,
    ERROR
};

class CommandProcessor
{
private:
public:
    // Parse script file stream
    vector<string> _parseScriptFile(string filename)
    {
        string line;
        vector<string> cmd_list;
        ifstream infile(filename);
        if (!infile.is_open())
        {
            cout << "Error opening script file: " << filename << endl;
            exit(1);
        }
        while (getline(infile, line))
            cmd_list.push_back(line);
        return cmd_list;
    }

    // Parse command enum type
    Command parseCommandType(string s)
    {
        if (s.compare("world") == 0)
            return WORLD;
        if (s.compare("import") == 0)
            return IMPORT;
        if (s.compare("debug") == 0)
            return DEBUG;
        if (s.compare("quit") == 0)
            return QUIT;
        if (s.compare("what_is_at") == 0)
            return WHAT_IS_AT;
        if (s.compare("what_is") == 0)
            return WHAT_IS;
        if (s.compare("what_is_in") == 0)
            return WHAT_IS_IN;
        return ERROR;
    }

    vector<string> parseArgs(string line)
    {
        vector<string> args;
        istringstream iss(line);
        for (string arg; iss >> arg;)
            args.push_back(arg);
        return args;
    }

    bool isComment(string line)
    {
        if (line.length() > 0)
            return (line.at(0) == ';');
        return false;
    }

    // Constructor & Destructor
    CommandProcessor(){};
    ~CommandProcessor(){};
};

#endif