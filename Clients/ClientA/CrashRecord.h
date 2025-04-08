#ifndef CRASHRECORD_H
#define CRASHRECORD_H

#include <string>
#include <vector>
#include <iostream>

class CrashRecord {
public:
    // Default constructor
    CrashRecord();

    // Constructs a CrashRecord from a vector of strings.
    // The order matches the CSV columns you provided.
    // If a field is missing/empty, store 0 (for numbers) or "" (for strings).
    CrashRecord(const std::vector<std::string>& data);

    // Prints out the record's data (for demonstration).
    void printRecord() const;

private:
    // 1) CRASH DATE and 2) CRASH TIME
    std::string crashDate;
    std::string crashTime;

    // 3) BOROUGH
    std::string borough;

    // 4) ZIP CODE
    int zipCode;

    // 5) LATITUDE, 6) LONGITUDE
    double latitude;
    double longitude;

    // 7) LOCATION (e.g. "(40.123, -73.456)")
    std::string location;

    // 8) ON STREET NAME, 9) CROSS STREET NAME, 10) OFF STREET NAME
    std::string onStreetName;
    std::string crossStreetName;
    std::string offStreetName;

    // 11) NUMBER OF PERSONS INJURED
    int numberOfPersonsInjured;

    // 12) NUMBER OF PERSONS KILLED
    int numberOfPersonsKilled;

    // 13) NUMBER OF PEDESTRIANS INJURED
    int numberOfPedestriansInjured;

    // 14) NUMBER OF PEDESTRIANS KILLED
    int numberOfPedestriansKilled;

    // 15) NUMBER OF CYCLIST INJURED
    int numberOfCyclistInjured;

    // 16) NUMBER OF CYCLIST KILLED
    int numberOfCyclistKilled;

    // 17) NUMBER OF MOTORIST INJURED
    int numberOfMotoristInjured;

    // 18) NUMBER OF MOTORIST KILLED
    int numberOfMotoristKilled;

    // 19) CONTRIBUTING FACTOR VEHICLE 1
    // 20) CONTRIBUTING FACTOR VEHICLE 2
    // 21) CONTRIBUTING FACTOR VEHICLE 3
    // 22) CONTRIBUTING FACTOR VEHICLE 4
    // 23) CONTRIBUTING FACTOR VEHICLE 5
    std::string contributingFactorVehicle1;
    std::string contributingFactorVehicle2;
    std::string contributingFactorVehicle3;
    std::string contributingFactorVehicle4;
    std::string contributingFactorVehicle5;

    // 24) COLLISION_ID
    int collisionId;

    // 25) VEHICLE TYPE CODE 1
    // 26) VEHICLE TYPE CODE 2
    // 27) VEHICLE TYPE CODE 3
    // 28) VEHICLE TYPE CODE 4
    // 29) VEHICLE TYPE CODE 5
    std::string vehicleTypeCode1;
    std::string vehicleTypeCode2;
    std::string vehicleTypeCode3;
    std::string vehicleTypeCode4;
    std::string vehicleTypeCode5;
};

#endif // CRASHRECORD_H
