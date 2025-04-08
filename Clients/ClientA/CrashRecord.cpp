#include "CrashRecord.h"
#include <cstdlib>

// Default constructor
CrashRecord::CrashRecord()
    : crashDate(""), crashTime(""), borough(""), zipCode(0),
      latitude(0.0), longitude(0.0), location(""),
      onStreetName(""), crossStreetName(""), offStreetName(""),
      numberOfPersonsInjured(0), numberOfPersonsKilled(0),
      numberOfPedestriansInjured(0), numberOfPedestriansKilled(0),
      numberOfCyclistInjured(0), numberOfCyclistKilled(0),
      numberOfMotoristInjured(0), numberOfMotoristKilled(0),
      contributingFactorVehicle1(""), contributingFactorVehicle2(""),
      contributingFactorVehicle3(""), contributingFactorVehicle4(""),
      contributingFactorVehicle5(""), collisionId(0),
      vehicleTypeCode1(""), vehicleTypeCode2(""), vehicleTypeCode3(""),
      vehicleTypeCode4(""), vehicleTypeCode5("")
{
}

// Constructor that parses CSV fields
CrashRecord::CrashRecord(const std::vector<std::string>& data)
    : CrashRecord() // Initialize everything to default values first
{
    // A helper lambda to safely read a string at 'idx'.
    auto readString = [&](size_t idx) -> std::string {
        if (idx < data.size() && !data[idx].empty()) {
            return data[idx];
        }
        return "";
    };

    // A helper lambda to safely read an int at 'idx'.
    auto readInt = [&](size_t idx) -> int {
        if (idx < data.size() && !data[idx].empty()) {
            try {
                return std::stoi(data[idx]);
            } catch (...) {}
        }
        return 0;
    };

    // A helper lambda to safely read a double at 'idx'.
    auto readDouble = [&](size_t idx) -> double {
        if (idx < data.size() && !data[idx].empty()) {
            try {
                return std::stod(data[idx]);
            } catch (...) {}
        }
        return 0.0;
    };

    // Now assign each field in the order you provided:
    // (Columns: 0..28 -> 29 fields total)

    crashDate  = readString(0);
    crashTime  = readString(1);
    borough    = readString(2);

    zipCode    = readInt(3);

    latitude   = readDouble(4);
    longitude  = readDouble(5);

    location          = readString(6);
    onStreetName      = readString(7);
    crossStreetName   = readString(8);
    offStreetName     = readString(9);

    numberOfPersonsInjured       = readInt(10);
    numberOfPersonsKilled        = readInt(11);
    numberOfPedestriansInjured   = readInt(12);
    numberOfPedestriansKilled    = readInt(13);
    numberOfCyclistInjured       = readInt(14);
    numberOfCyclistKilled        = readInt(15);
    numberOfMotoristInjured      = readInt(16);
    numberOfMotoristKilled       = readInt(17);

    contributingFactorVehicle1   = readString(18);
    contributingFactorVehicle2   = readString(19);
    contributingFactorVehicle3   = readString(20);
    contributingFactorVehicle4   = readString(21);
    contributingFactorVehicle5   = readString(22);

    collisionId = readInt(23);

    vehicleTypeCode1  = readString(24);
    vehicleTypeCode2  = readString(25);
    vehicleTypeCode3  = readString(26);
    vehicleTypeCode4  = readString(27);
    vehicleTypeCode5  = readString(28);
}

void CrashRecord::printRecord() const {
    std::cout << "Crash Date: " << crashDate 
              << ", Crash Time: " << crashTime
              << ", Borough: " << borough
              << ", Zip Code: " << zipCode
              << ", Latitude: " << latitude
              << ", Longitude: " << longitude
              << ", Location: " << location
              << ", On Street: " << onStreetName
              << ", Cross Street: " << crossStreetName
              << ", Off Street: " << offStreetName
              << ", Persons Injured: " << numberOfPersonsInjured
              << ", Persons Killed: " << numberOfPersonsKilled
              << ", Pedestrians Injured: " << numberOfPedestriansInjured
              << ", Pedestrians Killed: " << numberOfPedestriansKilled
              << ", Cyclist Injured: " << numberOfCyclistInjured
              << ", Cyclist Killed: " << numberOfCyclistKilled
              << ", Motorist Injured: " << numberOfMotoristInjured
              << ", Motorist Killed: " << numberOfMotoristKilled
              << ", Factor1: " << contributingFactorVehicle1
              << ", Factor2: " << contributingFactorVehicle2
              << ", Factor3: " << contributingFactorVehicle3
              << ", Factor4: " << contributingFactorVehicle4
              << ", Factor5: " << contributingFactorVehicle5
              << ", Collision ID: " << collisionId
              << ", Vehicle Code1: " << vehicleTypeCode1
              << ", Vehicle Code2: " << vehicleTypeCode2
              << ", Vehicle Code3: " << vehicleTypeCode3
              << ", Vehicle Code4: " << vehicleTypeCode4
              << ", Vehicle Code5: " << vehicleTypeCode5
              << std::endl;
}
