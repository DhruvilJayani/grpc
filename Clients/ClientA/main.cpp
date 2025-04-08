#include "CSVDataReader.h"
#include <iostream>

int main() {
    // CSV file path relative to the ClientA folder
    std::string filename = "../Motor_Vehicle_Collisions_-_Crashes_20250123.csv";
    
    CSVDataReader reader;
    std::vector<CrashRecord> records = reader.readData(filename);
    
    if (records.empty()) {
        std::cerr << "No records found or error reading file: " << filename << std::endl;
        return 1;
    }
    
    // Print all records (each record has 29 possible fields).
    for (const auto& record : records) {
        // record.printRecord();
    }
    // std::cout << records.size();
    records[10].printRecord();
    return 0;
}

// clang++ -std=c++11 -Xpreprocessor -fopenmp \
//     -I$(brew --prefix libomp)/include \
//     -L$(brew --prefix libomp)/lib -lomp \
//     main.cpp CrashRecord.cpp CSVDataReader.cpp -o DataParser