#pragma once
#include <cstdint>      // For uint8_t
#include <utility>      // For std::pair
#include <fstream>      // For file reading
#include <vector>       // For storing CSV data
#include <string>       // For strings

// DataGenerationBlock owns the input dataset and exposes it as a sequential stream of pixel pairs.
// Source can be CSV or synthetic random data, but both are consumed through getNextPair().
class DataGenerationBlock {
private:
    // CSV data storage
    std::vector<std::vector<uint8_t>> csvData;  // 2D array of pixels

    // Current position tracking
    size_t currentRow;      // Which row we're reading
    size_t currentCol;      // Which column we're reading

    // Configuration
    int numColumns;         // Number of columns (m) - auto-detected
    int numRows;            // Number of rows in CSV

    // Helper function to parse one line of CSV
    std::vector<uint8_t> parseLine(const std::string& line);

public:
    // Constructor
    DataGenerationBlock();

    // Load CSV file and auto-detect columns
    bool loadCSV(const char* filename);

    // Create random data with given rows and columns
    bool loadRandom(int rows, int cols);

    // Get the next 2 pixels
    std::pair<uint8_t, uint8_t> getNextPair();

    // Get detected number of columns
    int getNumColumns() const { return numColumns; }

    // Get total number of rows
    int getNumRows() const { return numRows; }

    // Reset to beginning of CSV
    void reset();

    // Check if we've processed all data
    bool isFinished() const;
};

