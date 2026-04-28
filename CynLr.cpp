#include <iostream>
#include "DataGeneration.h"
#include "FilterBlock.h"

int main(int argc, char* argv[]) {
    std::cout << "========================================\n";
    std::cout << "  Data Generation Block - TEST\n";
    std::cout << "========================================\n\n";

    // Create the data generation block
    DataGenerationBlock dataGen;

    // Let user choose between CSV or Random mode
    std::cout << "Choose input mode:\n";
    std::cout << "  1) CSV file\n";
    std::cout << "  2) Random mode\n";
    std::cout << "Enter choice (1 or 2, press Enter for 1): ";

    std::string choice;
    std::getline(std::cin, choice);

    if (choice.empty()) choice = "1";

    if (choice == "1") {
        // CSV mode
        std::string defaultFile = "sample_input.csv";
        if (argc > 1) {
            defaultFile = argv[1];
        }

        std::string filename;
        std::cout << "Enter CSV filename to load (press Enter to use '" << defaultFile << "'): ";
        std::getline(std::cin, filename);

        if (filename.empty()) {
            filename = defaultFile;
        }

        // Try loading; allow retry on failure
        while (!dataGen.loadCSV(filename.c_str())) {
            std::cout << "Failed to load '" << filename << "'.\n";
            std::cout << "Enter another filename or press Enter to cancel: ";
            std::getline(std::cin, filename);

            if (filename.empty()) {
                std::cout << "Canceled. Press Enter to exit...";
                std::cin.get();
                return 1;
            }
        }

        std::cout << "Loading CSV file: " << filename << "\n\n";
    } else if (choice == "2") {
        // Random mode
        int rows = 0, cols = 0;

        while (rows <= 0) {
            std::cout << "Enter number of rows (>0): ";
            std::string s;
            std::getline(std::cin, s);
            try { rows = std::stoi(s); } catch (...) { rows = 0; }
            if (rows <= 0) std::cout << "Invalid value.\n";
        }

        while (cols <= 0) {
            std::cout << "Enter number of columns (>0): ";
            std::string s;
            std::getline(std::cin, s);
            try { cols = std::stoi(s); } catch (...) { cols = 0; }
            if (cols <= 0) std::cout << "Invalid value.\n";
        }

        if (!dataGen.loadRandom(rows, cols)) {
            std::cout << "Failed to generate random data. Exiting.\n";
            return 1;
        }
    } else {
        std::cout << "Invalid choice. Exiting.\n";
        return 1;
    }

    // Display detected parameters
    std::cout << "Detected parameters:\n";
    std::cout << "  m (columns): " << dataGen.getNumColumns() << "\n";
    std::cout << "  rows: " << dataGen.getNumRows() << "\n\n";

    // Test: Get first 20 pairs
    std::cout << "========================================\n";
    std::cout << "  First 20 Pixel Pairs:\n";
    std::cout << "========================================\n\n";

    std::cout << "Pair#  | Pixel1 | Pixel2\n";
    std::cout << "-------+--------+--------\n";

    for (int i = 0; i < 20 && !dataGen.isFinished(); i++) {
        auto [p1, p2] = dataGen.getNextPair();

        printf("%5d  | %6d | %6d\n", i + 1, p1, p2);
    }

    std::cout << "\n========================================\n";
    std::cout << "  Testing Reset Functionality\n";
    std::cout << "========================================\n\n";

    // Reset and get first pair again
    dataGen.reset();
    auto [first1, first2] = dataGen.getNextPair();

    std::cout << "After reset, first pair: [" << (int)first1 << ", " << (int)first2 << "]\n";

    // Process ALL pixels and count them
    std::cout << "\n========================================\n";
    std::cout << "  Processing All Pixels\n";
    std::cout << "========================================\n\n";

    dataGen.reset();
    int pairCount = 0;
    int pixelCount = 0;

    while (!dataGen.isFinished()) {
        auto [p1, p2] = dataGen.getNextPair();
        pairCount++;

        if (p1 != 0) pixelCount++;  // Count non-zero pixels
        if (p2 != 0) pixelCount++;
    }

    std::cout << "Total pairs processed: " << pairCount << "\n";
    std::cout << "Total pixels counted: " << pixelCount << "\n\n";

    // Now test the Filter Block
    std::cout << "========================================\n";
    std::cout << "  Filter Block - TEST\n";
    std::cout << "========================================\n\n";

    FilterBlock filter;

    // Initialize filter with number of columns (per row is independent)
    filter.initialize(dataGen.getNumColumns());

    dataGen.reset();

    std::cout << "Processing pairs through FilterBlock...\n";
    std::cout << "Row | Pair | DataGen Pair | Filter Output | Buffer Size\n";
    std::cout << "----+------+--------------+---------------+-------------\n";

    int currentRow = 0;
    int pairNum = 0;
    std::vector<std::pair<uint8_t, uint8_t>> filteredResults;

    while (!dataGen.isFinished()) {
        pairNum++;
        auto dataPair = dataGen.getNextPair();
        auto [dp1, dp2] = dataPair;

        // Add pair to filter and get result if available
        auto filteredPair = filter.addPairAndFilter(dataPair);
        auto [fp1, fp2] = filteredPair;

        // Check if we're at end of row
        if (pairNum % (dataGen.getNumColumns() / 2 + 1) == 0 || 
            (dataGen.getNumColumns() % 2 == 1 && pairNum % ((dataGen.getNumColumns() + 1) / 2) == 0)) {
            filter.endOfRow();

            printf("%4d | (end of row) | -----------   | ------------- | (cleared)\n", currentRow);

            if (pairNum < 20) {
                currentRow++;
                pairNum = 0;
            } else {
                std::cout << "  ... (continuing) ...\n";
                break;
            }
        } else if (pairNum <= 15) {
            // Print first 15 pairs per row
            if (fp1 != 0 || fp2 != 0) {
                printf("%4d | %4d | [%3d, %3d]   | [%3d, %3d]    | %11zu\n",
                    currentRow, pairNum, dp1, dp2, fp1, fp2, filter.getBufferSize());
                filteredResults.push_back(filteredPair);
            } else {
                printf("%4d | %4d | [%3d, %3d]   | (buffering)   | %11zu\n",
                    currentRow, pairNum, dp1, dp2, filter.getBufferSize());
            }
        }
    }

    // Continue processing remaining pairs without detailed output
    while (!dataGen.isFinished()) {
        pairNum++;
        auto dataPair = dataGen.getNextPair();
        auto filteredPair = filter.addPairAndFilter(dataPair);

        if (filteredPair.first != 0 || filteredPair.second != 0) {
            filteredResults.push_back(filteredPair);
        }

        // Check if we're at end of row
        if (pairNum % ((dataGen.getNumColumns() + 1) / 2) == 0) {
            filter.endOfRow();
            currentRow++;
            pairNum = 0;
        }
    }

    std::cout << "\nTotal filtered pairs produced: " << filteredResults.size() << "\n";
    std::cout << "Columns per row: " << dataGen.getNumColumns() << "\n";
    std::cout << "Pairs per row: " << ((dataGen.getNumColumns() + 1) / 2) << "\n";
    std::cout << "Filter block cycles: " << filter.getCycleCount() << "\n";
    std::cout << "Final buffer size: " << filter.getBufferSize() << "\n\n";

    std::cout << "========================================\n";
    std::cout << "  Test Complete!\n";
    std::cout << "========================================\n\n";

    std::cout << "Press Enter to exit...";
    std::cin.get();

    return 0;
}