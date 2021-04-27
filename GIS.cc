#include <iostream>
#include "CommandProcessor.h"
#include "SystemManager.h"
#include "GISRecord.h"

using namespace std;

int main(int argc, char *argv[])
{
    if (argc < 4)
    {
        cerr << "Usage: " << argv[0] << " <database file> <command script file> <log file>" << endl;
        return 1;
    }

    SystemManager manager(argv[1], argv[2], argv[3]);
    manager.addToLog("Course Project for COMP 8042");
    manager.addToLog("Student Name: Eric Wu, Student Id: A00961904");
    manager.start();

    return 0;
}