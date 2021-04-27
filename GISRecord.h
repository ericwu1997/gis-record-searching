#pragma once
#ifndef GIS_RECORD_H
#define GIS_RECORD_h

#include <string>

using namespace std;

class GISRecord
{
private:
    bool valid = true;

public:
    int featureId;       // Feature ID
    string featureName;  // Feature Name
    string featureClass; // Feature Class

    string stateAlpha;   // State Alpha
    string stateNumeric; // State Numeric

    string countryName;    // Country Name
    string countryNumeric; // Country Numeric

    string primaryLAT_DMS;  // Primary Latitude DMS
    string primaryLONG_DMS; // Primary Longitude DMS
    float primaryLAT_DEC;   // Primary Latitude DEC
    float primaryLONG_DEC;  // Primary Longitude DEC

    string sourceLAT_DMS;  // Source Latitude DMS
    string sourceLONG_DMS; // Source Longitude DMS
    float sourceLAT_DEC;   // Source Latitude DEC
    float sourceLONG_DEC;  // Source Longitude DEC

    int elevation_m; // Elevation (meter)
    int elevation_f; // Elevation (feet)

    string mapName;    // Map Name
    string createDate; // Date Created
    string editedDate; // Date Edited

    string str()
    {
        stringstream ss;

        ss << to_string(featureId) << "|";
        ss << featureName << "|";
        ss << featureClass << "|";
        ss << stateAlpha << "|";
        ss << stateNumeric << "|";
        ss << countryName << "|";
        ss << countryNumeric << "|";
        ss << primaryLAT_DMS << "|";
        ss << primaryLONG_DMS << "|";
        ss << to_string(primaryLAT_DEC) << "|";
        ss << to_string(primaryLONG_DEC) << "|";
        ss << sourceLAT_DMS << "|";
        ss << sourceLONG_DMS << "|";
        ss << to_string(sourceLAT_DEC) << "|";
        ss << to_string(sourceLONG_DEC) << "|";
        ss << to_string(elevation_m) << "|";
        ss << to_string(elevation_f) << "|";
        ss << mapName << "|";
        ss << createDate << "|";
        ss << editedDate << "|";

        return ss.str();
    }

    bool isType(string type)
    {
        if (type.compare("structure") == 0)
        {
            return (featureClass.compare("Airport") == 0 ||
                    featureClass.compare("Bridge") == 0 ||
                    featureClass.compare("Building") == 0 ||
                    featureClass.compare("Church") == 0 ||
                    featureClass.compare("Dam") == 0 ||
                    featureClass.compare("Hospital") == 0 ||
                    featureClass.compare("Levee") == 0 ||
                    featureClass.compare("Part") == 0 ||
                    featureClass.compare("Post Office") == 0 ||
                    featureClass.compare("School") == 0 ||
                    featureClass.compare("Tower") == 0 ||
                    featureClass.compare("Tunnel") == 0);
        }
        if (type.compare("water") == 0)
        {
            return (featureClass.compare("Arriti") == 0 ||
                    featureClass.compare("Bay") == 0 ||
                    featureClass.compare("Bend") == 0 ||
                    featureClass.compare("Canal") == 0 ||
                    featureClass.compare("Channel") == 0 ||
                    featureClass.compare("Falls") == 0 ||
                    featureClass.compare("Glacier") == 0 ||
                    featureClass.compare("Gut") == 0 ||
                    featureClass.compare("Harbor") == 0 ||
                    featureClass.compare("Lake") == 0 ||
                    featureClass.compare("Rapids") == 0 ||
                    featureClass.compare("Reservoir") == 0 ||
                    featureClass.compare("Sea") == 0 ||
                    featureClass.compare("Spring") == 0 ||
                    featureClass.compare("Stream") == 0 ||
                    featureClass.compare("Swamp") == 0 ||
                    featureClass.compare("Well") == 0);
        }
        if (type.compare("pop") == 0)
        {
            return (featureClass.compare("Populated Place") == 0);
        }
        return false;
    }

    bool isValid()
    {
        return valid;
    }

    GISRecord(string record);
    GISRecord(){};
    ~GISRecord(){};
};

GISRecord::GISRecord(string record)
{
    vector<string> tokens;
    size_t pos = 0;
    string delimiter = "|";
    while ((pos = record.find(delimiter)) != string::npos)
    {
        tokens.push_back(record.substr(0, pos));
        record.erase(0, pos + delimiter.length());
    }
    tokens.push_back(record);
    if (tokens.size() != 20)
    {
        // cout << "" << record << endl;
    }
    else
    {
        string tmp;

        if ((tmp = tokens[0]) != "")
            featureId = stoi(tmp);
        else
            valid = false;

        featureName = tokens[1];
        featureClass = tokens[2];

        stateAlpha = tokens[3];
        stateNumeric = tokens[4];

        countryName = tokens[5];
        countryNumeric = tokens[6];

        if ((tmp = tokens[7]) != "" && tmp.compare("Unknown") != 0)
            primaryLAT_DMS = tmp;
        else
            valid = false;
        if ((tmp = tokens[8]) != "" && tmp.compare("Unknown") != 0)
            primaryLONG_DMS = tokens[8];
        else
            valid = false;
        if ((tmp = tokens[9]) != "")
            primaryLAT_DEC = stof(tmp);
        if ((tmp = tokens[10]) != "")
            primaryLONG_DEC = stof(tmp);

        sourceLAT_DMS = tokens[11];
        sourceLONG_DMS = tokens[12];
        if ((tmp = tokens[13]) != "" && tmp.compare("Unknown") != 0)
            sourceLAT_DEC = stof(tmp);
        if ((tmp = tokens[14]) != "" && tmp.compare("Unknown") != 0)
            sourceLONG_DEC = stof(tmp);
        if ((tmp = tokens[15]) != "" && tmp.compare("Unknown") != 0)
            elevation_m = stoi(tmp);
        if ((tmp = tokens[16]) != "" && tmp.compare("Unknown") != 0)
            elevation_f = stoi(tmp);
        mapName = tokens[17];
        createDate = tokens[18];
        editedDate = tokens[19];
    }
};

#endif
