#include "CSVDataReader.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <omp.h>

std::vector<std::string> CSVDataReader::parseCSVLine(const std::string& line) {
    std::vector<std::string> result;
    std::string currentField;
    bool inQuotes = false;
    bool inParentheses = false;

    for (size_t i = 0; i < line.size(); ++i) {
        char c = line[i];
        // Toggle quotes if not escaped.
        if (c == '"' && (i == 0 || line[i - 1] != '\\')) {
            inQuotes = !inQuotes;
        }
        else if (c == '(') {
            inParentheses = true;
        }
        else if (c == ')') {
            inParentheses = false;
        }
        else if (c == ',' && !inQuotes && !inParentheses) {
            result.push_back(currentField);
            currentField.clear();
            continue;
        }
        currentField += c;
    }
    // Add the last field
    result.push_back(currentField);
    return result;
}

std::vector<CrashRecord> CSVDataReader::readData(const std::string& filename) {
    std::vector<CrashRecord> records;
    std::ifstream file(filename);
    
    if (!file.is_open()) {
        std::cerr << "Error opening file: " << filename << std::endl;
        return records;
    }
    
    std::string line;
    // Skip header line (assuming first line is column names).
    if (!std::getline(file, line)) {
        std::cerr << "Error reading header from file." << std::endl;
        return records;
    }
    
    // Read all lines into a vector
    std::vector<std::string> lines;
    while (std::getline(file, line)) {
        if (!line.empty()) {
            lines.push_back(line);
        }
    }
    file.close();

    // Resize once to avoid repeated allocations
    records.resize(lines.size());

    // Parallel parse (remove #pragma statements if you aren’t using OpenMP)
    #pragma omp parallel for
    for (size_t i = 0; i < lines.size(); ++i) {
        std::vector<std::string> data = parseCSVLine(lines[i]);
        records[i] = CrashRecord(data);
    }
    return records;
}
