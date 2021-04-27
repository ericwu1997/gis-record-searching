#pragma once
#ifndef SYSTEM_MANAGER_H
#define SYSTEM_MANAGER_H

#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>

#include "NameIndex.h"
#include "CommandProcessor.h"
#include "CoordinateIndex.h"
#include "Logger.h"
#include "BufferPool.h"
#include "DMS.h"
#include "GISRecord.h"

#define NAMEINDEX_CAPACITY 1024
#define CACHE_CAPACITY 15

using namespace std;

class SystemManager
{
private:
    CommandProcessor *_cp;
    CoordinateIndex *_coordinateIndex;
    NameIndex *_nameIndex;
    Logger _logger;
    BufferPool *_pool;

    string _db_file;
    string _script_file;
    string _log_file;
    long int _db_file_offset;
    long int boundry[4] = {-648000, -324000, 648000, 324000};

    void world(vector<string> argv);
    void import(vector<string> argv);
    void debug(vector<string> argv);
    void what_is(vector<string> argv);
    void what_is_at(vector<string> argv);
    void what_is_in(vector<string> argv);
    void quit();

public:
    void start()
    {
        // Log header information
        _logger << "Begin of GIS Program log:" << endl;
        _logger << "dbFile:	" << _db_file << endl;
        _logger << "script:	" << _script_file << endl;
        _logger << "log:	" << _log_file << endl;
        _logger.addTimeStamp("Start Time: ");

        // Init database, reset if already exist.
        ofstream ofs;
        ofs.open(_db_file, ofstream::trunc | ofstream::out);
        if (!ofs.is_open())
        {
            cout << "Error opening db file: " << _db_file << endl;
            exit(1);
        }
        ofs << "FEATURE_ID|FEATURE_NAME|FEATURE_CLASS|STATE_ALPHA|STATE_NUMERIC|COUNTY_NAME|COUNTY_NUMERIC|PRIMARY_LAT_DMS|PRIM_LONG_DMS|PRIM_LAT_DEC|PRIM_LONG_DEC|SOURCE_LAT_DMS|SOURCE_LONG_DMS|SOURCE_LAT_DEC|SOURCE_LONG_DEC|ELEV_IN_M|ELEV_IN_FT|MAP_NAME|DATE_CREATED|DATE_EDITED" << endl;
        ofs.close();
        ofs.clear();
        ifstream ifs(_db_file, ios::in);
        string line;
        getline(ifs, line); // Inititial offset jump pass the first line
        _db_file_offset = ifs.tellg().operator std::streamoff();

        // When database file is created, init the bufferpool instance
        _pool = new BufferPool(CACHE_CAPACITY, _db_file);

        // Parse script file
        vector<string> script_line_list = _cp->_parseScriptFile(_script_file);
        vector<string> argv;
        Command type;
        int count = 0;
        for (auto &script_line : script_line_list)
        {
            // If is comment, output to log immediately
            if (_cp->isComment(script_line))
                _logger << script_line << endl;
            else
            {
                _logger << "Command " << to_string(++count) << ": " << script_line << endl
                        << endl;
                argv.clear();
                argv = _cp->parseArgs(script_line);
                type = _cp->parseCommandType(argv[0]);
                switch (type)
                {
                case Command::WORLD:
                    world(argv);
                    break;
                case Command::IMPORT:
                    import(argv);
                    break;
                case Command::DEBUG:
                    debug(argv);
                    break;
                case Command::QUIT:
                    quit();
                    return;
                    break;
                case Command::WHAT_IS_AT:
                    what_is_at(argv);
                    break;
                case Command::WHAT_IS:
                    what_is(argv);
                    break;
                case Command::WHAT_IS_IN:
                    what_is_in(argv);
                    break;
                case Command::ERROR:
                    break;
                default:
                    break;
                }
            }
        }

        _logger.addTimeStamp("End time: ");
    }

    void addToLog(string s)
    {
        _logger << s << endl;
    }

    SystemManager(string db_file, string script_file, string log_file)
        : _db_file(db_file), _script_file(script_file), _log_file(log_file)
    {
        _db_file_offset = 0;

        // Init other components
        _logger.open(log_file);                         // Logger class
        _cp = new CommandProcessor();                   // CommandProcessor class
        _nameIndex = new NameIndex(NAMEINDEX_CAPACITY); // NameIndex class
        _coordinateIndex = new CoordinateIndex(boundry[0], boundry[1], boundry[2], boundry[3]);
    };

    ~SystemManager()
    {
        _logger.close();
        free(_cp);
        free(_nameIndex);
        free(_pool);
        free(_coordinateIndex);
    };
};

void SystemManager::world(vector<string> argv)
{
    if (argv.size() != 5)
    {
        _logger << "world(), Invalid argument count" << endl;
        return;
    }
    _logger.split();
    _logger << "Latitude/longitude values in index entries are shown as signed integers, in total seconds." << endl;
    _logger.split();
    _logger << "            World boundaries are set to:" << endl;

    DMS w_long(argv[1]), e_long(argv[2]), s_lat(argv[3]), n_lat(argv[4]);

    // Reconfigure coordinateIndex with new boundry
    free(_coordinateIndex);
    boundry[0] = w_long.toSecond();
    boundry[1] = s_lat.toSecond();
    boundry[2] = e_long.toSecond();
    boundry[3] = n_lat.toSecond();
    cout << to_string(boundry[0]) << "  " << to_string(boundry[1]) << " " << to_string(boundry[2]) << " " << to_string(boundry[3]) << endl;
    _coordinateIndex = new CoordinateIndex(boundry[0], boundry[1], boundry[2], boundry[3]);

    _logger << "                       " << to_string(n_lat.toSecond()) << endl;
    _logger << "            " << to_string(w_long.toSecond());
    _logger << "              " << to_string(e_long.toSecond()) << endl;
    _logger << "                       " << to_string(s_lat.toSecond()) << endl;
    _logger.split();
}

void SystemManager::import(vector<string> argv)
{
    if (argv.size() != 2)
    {
        _logger << "import(), Invalid argument count" << endl;
        return;
    }

    string line;
    ofstream ofs(_db_file, ios::out | ios::app);
    ifstream ifs(argv[1], ios::in);

    if (!ofs.is_open())
    {
        cout << "Error opening db file: " << _db_file << endl;
        exit(1);
    }
    if (!ifs.is_open())
    {
        cout << "Error opening dataset file: " << argv[1] << endl;
        exit(1);
    }

    // Remove field header line, record offset of the first record
    getline(ifs, line);

    // Append to database file
    while (getline(ifs, line))
    {
        GISRecord record(line);
        if (!record.isValid())
            continue;
        DMS x_dms(record.primaryLONG_DMS);
        DMS y_dms(record.primaryLAT_DMS);
        long int x = x_dms.toSecond(), y = y_dms.toSecond();
        if (x > boundry[0] && y > boundry[1] && boundry[2] > x && boundry[3] > y)
            ofs << line << endl;
    }
    ofs.close();
    ifs.close();

    // Index info before import
    int nameIndex_old_size = _nameIndex->size();
    int coordinateIndex_old_size = _coordinateIndex->size();
    _nameIndex->resetCollisionCount();

    // And entry to both NameIndex and CoordinateIndex
    ifs.clear();
    ifs.open(_db_file, ios::in);
    ifs.seekg(_db_file_offset, ios::beg);
    string featurename_statealpha;
    int avg_name_length = 0;

    while (getline(ifs, line))
    {
        GISRecord record(line);

        // Insert into NameIndex hashtable
        featurename_statealpha = record.featureName + ":" + record.stateAlpha;
        _nameIndex->insert(featurename_statealpha, _db_file_offset);

        // Insert into Coordinate Quadtree
        DMS longtitude(record.primaryLONG_DMS);
        DMS latitude(record.primaryLAT_DMS);
        _coordinateIndex->insert(longtitude.toSecond(), latitude.toSecond(), _db_file_offset);

        _db_file_offset = ifs.tellg().operator std::streamoff();
        avg_name_length = (avg_name_length + record.featureName.length()) / 2;
    }

    _logger << "Imported Features by name: " << to_string(_nameIndex->size() - nameIndex_old_size) << endl;
    _logger << "Longest probe sequence:    " << to_string(_nameIndex->longestProbe()) << endl;
    _logger << "Imported Locations:        " << to_string(_coordinateIndex->size() - coordinateIndex_old_size) << endl;
    _logger << "Average name length:       " << to_string(avg_name_length) << endl;
    _logger.split();
}

void SystemManager::debug(vector<string> argv)
{
    if (argv.size() != 2)
    {
        _logger << "debug(), Invalid argument count" << endl;
        return;
    }
    string option = argv[1];
    if (option.compare("quad") == 0)
    {
        _logger << _coordinateIndex->str() << endl;
        _logger.split();
        return;
    }
    if (option.compare("hash") == 0)
    {
        _logger << _nameIndex->str() << endl;
        _logger.split();
        return;
    }
    if (option.compare("pool") == 0)
    {
        _logger << _pool->str() << endl;
        _logger.split();
        return;
    }
    if (option.compare("world") == 0)
    {
        _logger << "This is an optional command [NOT IMPLEMENTED]" << endl;
        _logger.split();
        return;
    }
    _logger << "debug(), Invalid option: " << option;
    _logger.split();
}

void SystemManager::what_is(vector<string> argv)
{
    if (argv.size() < 3)
    {
        _logger << "what_is(), Invalid argument count" << endl;
        return;
    }

    // Parse featurename and statealpha
    stringstream ss;
    string statealpha = argv[argv.size() - 1];
    ss << argv[1];
    for (int i = 2; i < argv.size() - 1; i++)
        ss << " " << argv[i];
    string featurename = ss.str();
    string featurename_statealpha = featurename + ":" + statealpha;

    vector<long int> offsets = _nameIndex->search(featurename_statealpha);
    if (!offsets.empty())
    {
        GISRecord record;
        for (auto &offset : offsets)
        {
            record = _pool->refer(offset);
            DMS latitude(record.primaryLAT_DMS);
            DMS longtitude(record.primaryLONG_DMS);

            _logger << "  " << to_string(offset) << ":  ";
            _logger << record.countryName << "  ";

            _logger << "(" << to_string(latitude.getDeg()) << "d ";
            _logger << to_string(latitude.getMin()) << "m ";
            _logger << to_string(latitude.getSec()) << "s ";
            _logger << (latitude.getDir() == 'N' ? "North" : "South") << ", ";

            _logger << to_string(longtitude.getDeg()) << "d ";
            _logger << to_string(longtitude.getMin()) << "m ";
            _logger << to_string(longtitude.getSec()) << "s ";
            _logger << (latitude.getDir() == 'S' ? "South" : "West") << ")" << endl;
        }
    }
    else
        _logger << "  No records match \"" << featurename << "\" and \"" << statealpha << "\"" << endl;
    _logger.split();
}

void SystemManager::what_is_at(vector<string> argv)
{
    if (argv.size() < 3)
    {
        _logger << "what_is_at(), Invalid argument count" << endl;
        return;
    }

    vector<long int> results_offsets;
    DMS latitude(argv[1]);
    DMS longitude(argv[2]);
    results_offsets = _coordinateIndex->search(longitude.toSecond(), latitude.toSecond());

    if (results_offsets.empty())
    {
        // No records match 381816Nand 0793700W
        _logger << "  Nothing was found at (\"";
        _logger << to_string(latitude.getDeg()) << "d ";
        _logger << to_string(latitude.getMin()) << "m ";
        _logger << to_string(latitude.getSec()) << "s ";
        _logger << (latitude.getDir() == 'N' ? "North" : "South") << "\", \"";

        _logger << to_string(longitude.getDeg()) << "d ";
        _logger << to_string(longitude.getMin()) << "m ";
        _logger << to_string(longitude.getSec()) << "s ";
        _logger << (longitude.getDir() == 'E' ? "East" : "West") << "\")" << endl;
    }
    else
    {
        _logger << "  The following feature(s) were found at (";
        _logger << to_string(latitude.getDeg()) << "d ";
        _logger << to_string(latitude.getMin()) << "m ";
        _logger << to_string(latitude.getSec()) << "s ";
        _logger << (latitude.getDir() == 'N' ? "North" : "South") << ", ";

        _logger << to_string(longitude.getDeg()) << "d ";
        _logger << to_string(longitude.getMin()) << "m ";
        _logger << to_string(longitude.getSec()) << "s ";
        _logger << (longitude.getDir() == 'E' ? "East" : "West") << ")" << endl;

        GISRecord record;
        for (auto &offset : results_offsets)
        {
            record = _pool->refer(offset);
            _logger << "  " << to_string(offset) << ":  ";
            _logger << "\"" << record.featureName << "\" ";
            _logger << "\"" << record.countryName << "\" ";
            _logger << "\"" << record.stateAlpha << "\"" << endl;
        }
    }
    _logger.split();
}

void SystemManager::what_is_in(vector<string> argv)
{
    string verbose_option = "-long";
    string filter_option = "-filter";
    string filter_str;
    bool verbose = false, filter = false;
    if (argv.size() > 5)
        if (argv.size() == 6)
        {
            if (verbose_option.compare(argv[1]) == 0)
                verbose = true;
            else
            {
                _logger << "what_is_in(), Invalid option: " << argv[1] << endl;
                return;
            }
        }
        else if (argv.size() == 7)
        {
            if (filter_option.compare(argv[1]) == 0)
            {
                filter = true;
            }
            else
            {
                _logger << "what_is_in(), Invalid option: " << argv[1] << endl;
                return;
            }
        }
        else
        {
            _logger << "what_is_in(), Invalid argument count" << endl;
            return;
        }

    if (verbose)
    {
        argv[1] = argv[2];
        argv[2] = argv[3];
        argv[3] = argv[4];
        argv[4] = argv[5];
    }
    else if (filter)
    {
        filter_str = argv[2];
        argv[1] = argv[3];
        argv[2] = argv[4];
        argv[3] = argv[5];
        argv[4] = argv[6];
    }

    DMS latitude(argv[1]);
    DMS longitude(argv[2]);

    long int latitude_range = stol(argv[3]);
    long int longtitude_range = stol(argv[4]);
    long int latitude_sec = latitude.toSecond();
    long int longtitude_sec = longitude.toSecond();

    vector<long int> result = _coordinateIndex->findSubset(
        longtitude_sec - longtitude_range, latitude_sec - latitude_range,
        longtitude_sec + longtitude_range, latitude_sec + latitude_range);

    if (result.empty())
        _logger << "  Nothing was found in (\"";
    else
    {
        if (filter)
            _logger << "  The following features matching your criteria were found in (";
        else
            _logger << "  The following " << to_string(result.size()) << " feature(s) were found in (";
    }
    _logger << to_string(latitude.getDeg()) << "d ";
    _logger << to_string(latitude.getMin()) << "m ";
    _logger << to_string(latitude.getSec()) << "s ";
    _logger << (latitude.getDir() == 'N' ? "North" : "South");
    _logger << " +/- " << to_string(latitude_range) << ", ";

    _logger << to_string(longitude.getDeg()) << "d ";
    _logger << to_string(longitude.getMin()) << "m ";
    _logger << to_string(longitude.getSec()) << "s ";
    _logger << (longitude.getDir() == 'E' ? "East" : "West");
    _logger << " +/- " << to_string(longtitude_range) << ")\"" << endl;

    _logger << "" << endl;

    int feature_match = 0; // for showing number of result match when filter applied
    GISRecord record;
    for (auto &offset : result)
    {
        record = _pool->refer(offset);

        // Apply filter if option specified, ignore result if type not match
        if (filter && !record.isType(filter_str))
            continue;
        feature_match++;

        // Verbose format
        if (verbose)
        {
            _logger << "  Feature ID   : " << to_string(record.featureId) << endl;
            _logger << "  Feature Name : " << record.featureName << endl;
            _logger << "  Feature Cat  : " << record.featureClass << endl;
            _logger << "  State        : " << record.stateAlpha << endl;
            _logger << "  County       : " << record.countryName << endl;

            _logger << "  Longitude    : ";
            _logger << to_string(longitude.getDeg()) << "d ";
            _logger << to_string(longitude.getMin()) << "m ";
            _logger << to_string(longitude.getSec()) << "s ";
            _logger << (longitude.getDir() == 'E' ? "East" : "West") << endl;

            _logger << "  Latitude     : ";
            _logger << to_string(latitude.getDeg()) << "d ";
            _logger << to_string(latitude.getMin()) << "m ";
            _logger << to_string(latitude.getSec()) << "s ";
            _logger << (latitude.getDir() == 'N' ? "North" : "South") << endl;

            _logger << "  Elev in ft   : " << to_string(record.elevation_f) << endl;
            _logger << "  USGS Quad    : " << record.mapName << endl;

            _logger << "  Date created : " << record.createDate << endl;
            _logger << "" << endl;
        }
        else
        {
            // Normal format
            DMS latitude(record.primaryLAT_DMS);
            DMS longitude(record.primaryLONG_DMS);
            _logger << "  " << to_string(offset) << ":  ";
            _logger << "\"" << record.featureName << "\"  ";
            _logger << "\"" << record.stateAlpha << "\"  \"(";

            _logger << to_string(latitude.getDeg()) << "d ";
            _logger << to_string(latitude.getMin()) << "m ";
            _logger << to_string(latitude.getSec()) << "s ";
            _logger << (latitude.getDir() == 'N' ? "North" : "South");

            _logger << ", ";

            _logger << to_string(longitude.getDeg()) << "d ";
            _logger << to_string(longitude.getMin()) << "m ";
            _logger << to_string(longitude.getSec()) << "s ";
            _logger << (longitude.getDir() == 'E' ? "East" : "West");

            _logger << ")\"" << endl;
        }
    }
    if (filter)
    {
        _logger << "" << endl;
        _logger << "  There were " << to_string(feature_match) << " features of type structure." << endl;
    }
    _logger.split();
}

void SystemManager::quit()
{
    _logger << "Terminating execution of commands." << endl;
    _logger.split();
    _logger.addTimeStamp("End time: ");
}

#endif
