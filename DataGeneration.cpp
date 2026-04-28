#include "DataGeneration.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <random>

// Constructor - initialize everything
DataGenerationBlock::DataGenerationBlock()
    : currentRow(0), currentCol(0), numColumns(0), numRows(0) {
}

// Generate random data
bool DataGenerationBlock::loadRandom(int rows, int cols) {
    if (rows <= 0 || cols <= 0) {
        std::cerr << "ERROR: rows and cols must be > 0\n";
        return false;
    }

    csvData.clear();
    csvData.reserve(rows);

    // Setup random generator for 0-255
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 255);

    for (int r = 0; r < rows; ++r) {
        std::vector<uint8_t> row;
        row.reserve(cols);

        for (int c = 0; c < cols; ++c) {
            row.push_back(static_cast<uint8_t>(dis(gen)));
        }

        csvData.push_back(std::move(row));
    }

    numRows = rows;
    numColumns = cols;
    currentRow = 0;
    currentCol = 0;

    std::cout << "Generated random data:\n";
    std::cout << "  - Rows: " << numRows << "\n";
    std::cout << "  - Columns: " << numColumns << "\n";
    std::cout << "  - Total pixels: " << (numRows * numColumns) << "\n";
    std::cout << "  - Total pairs: " << ((numRows * numColumns + 1) / 2) << "\n\n";

    return true;
}

// Parse one line of CSV into a vector of uint8_t values
std::vector<uint8_t> DataGenerationBlock::parseLine(const std::string& line) {
    std::vector<uint8_t> row;
    std::stringstream ss(line);
    std::string value;

    // Split by comma
    while (std::getline(ss, value, ',')) {
        // Remove whitespace
        value.erase(0, value.find_first_not_of(" \t\r\n"));
        value.erase(value.find_last_not_of(" \t\r\n") + 1);

        if (!value.empty()) {
            int num = std::stoi(value);

            // Validate range (0-255 for uint8_t)
            if (num < 0 || num > 255) {
                std::cerr << "Warning: Value " << num << " out of range. Clamping to 0-255.\n";
                num = std::max(0, std::min(255, num));
            }

            row.push_back(static_cast<uint8_t>(num));
        }
    }

    return row;
}

// Load CSV file
bool DataGenerationBlock::loadCSV(const char* filename) {
    std::ifstream file(filename);

    if (!file.is_open()) {
        std::cerr << "ERROR: Could not open file: " << filename << "\n";
        std::cerr << "Make sure the file exists in the same folder as your .exe\n";
        return false;
    }

    std::string line;
    csvData.clear();
    int lineNumber = 0;

    // Read each line
    while (std::getline(file, line)) {
        lineNumber++;

        // Skip empty lines
        if (line.empty() || line.find_first_not_of(" \t\r\n") == std::string::npos) {
            continue;
        }

        std::vector<uint8_t> row = parseLine(line);

        if (row.empty()) {
            std::cerr << "Warning: Line " << lineNumber << " is empty or invalid.\n";
            continue;
        }

        // Detect number of columns from first row
        if (csvData.empty()) {
            numColumns = row.size();
            std::cout << "Auto-detected columns (m): " << numColumns << "\n";
        }

        // Warning if row has different number of columns
        if (row.size() != (size_t)numColumns) {
            std::cerr << "Warning: Line " << lineNumber << " has " << row.size()
                << " columns (expected " << numColumns << ")\n";
        }

        csvData.push_back(row);
    }

    file.close();

    // Final validation
    if (csvData.empty()) {
        std::cerr << "ERROR: No valid data found in CSV file.\n";
        return false;
    }

    numRows = csvData.size();

    std::cout << "Successfully loaded CSV:\n";
    std::cout << "  - Rows: " << numRows << "\n";
    std::cout << "  - Columns: " << numColumns << "\n";
    std::cout << "  - Total pixels: " << (numRows * numColumns) << "\n";
    std::cout << "  - Total pairs: " << ((numRows * numColumns + 1) / 2) << "\n\n";

    return true;
}

// Get next pair of pixels
std::pair<uint8_t, uint8_t> DataGenerationBlock::getNextPair() {
    uint8_t pixel1 = 0;
    uint8_t pixel2 = 0;

    // Safety check
    if (csvData.empty()) {
        std::cerr << "ERROR: No CSV data loaded!\n";
        return {0, 0};
    }

    // Skip any empty rows (defensive)
    while (currentRow < csvData.size() && currentCol >= csvData[currentRow].size()) {
        currentRow++;
        currentCol = 0;
    }

    if (currentRow >= csvData.size()) {
        return {0, 0};
    }

    const auto& row = csvData[currentRow];
    size_t rowSize = row.size();

    // First pixel is always from the current position
    pixel1 = row[currentCol];

    // If there's a next pixel in the same row, use it. Otherwise pad with 0
    if (currentCol + 1 < rowSize) {
        pixel2 = row[currentCol + 1];
        currentCol += 2;

        if (currentCol >= rowSize) {
            // move to next row
            currentRow++;
            currentCol = 0;
        }
    } else {
        // Last element in the row. If number of columns is odd, pad with 0
        if (numColumns % 2 == 1) {
            pixel2 = 0;
        } else {
            // Fallback: pad with 0
            pixel2 = 0;
        }

        // Move to next row after padding
        currentRow++;
        currentCol = 0;
    }

    return {pixel1, pixel2};
}

// Reset to beginning
void DataGenerationBlock::reset() {
    currentRow = 0;
    currentCol = 0;
}

// Check if finished
bool DataGenerationBlock::isFinished() const {
    return currentRow >= csvData.size();
}