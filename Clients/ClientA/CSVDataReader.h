#ifndef CSVDATAREADER_H
#define CSVDATAREADER_H

#include "CrashRecord.h"
#include <string>
#include <vector>

class CSVDataReader {
public:
    // Reads the CSV file and returns a vector of CrashRecord objects.
    // It assumes the first line is a header to be skipped.
    std::vector<CrashRecord> readData(const std::string& filename);

private:
    // Splits one CSV line into fields, handling quotes & parentheses
    // so we don’t incorrectly split on internal commas.
    std::vector<std::string> parseCSVLine(const std::string& line);
};

#endif // CSVDATAREADER_H
