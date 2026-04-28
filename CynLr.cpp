 #include <iostream>
#include <string>
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
    std::cout << "  3) Exit\n";
    std::cout << "Enter choice (1, 2, or 3, press Enter for 1): ";

    std::string choice;
    std::getline(std::cin, choice);

    if (choice.empty()) choice = "1";

    if (choice == "3") {
        std::cout << "Exiting application. Goodbye!\n";
        return 0;
    } else if (choice == "1") {
        // CSV mode
        std::string defaultFile = "sample_input.csv";
        if (argc > 1) {
            defaultFile = argv[1];
        }

        std::string filename;
        bool validCols = false;

        while (!validCols) {
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

            // Check if columns meet minimum requirement
            if (dataGen.getNumColumns() < 10) {
                std::cout << "CSV file has " << dataGen.getNumColumns() << " columns, but at least 10 are required.\n";
                std::cout << "Options:\n";
                std::cout << "  1) Try another CSV file\n";
                std::cout << "  2) Switch to Random mode\n";
                std::cout << "  3) Exit application\n";
                std::cout << "Enter choice (1, 2, or 3): ";

                std::string modeChoice;
                std::getline(std::cin, modeChoice);

                if (modeChoice == "2") {
                    std::cout << "\nSwitching to Random mode...\n\n";
                    choice = "2";
                    break;
                } else if (modeChoice == "3") {
                    std::cout << "Exiting application. Goodbye!\n";
                    return 0;
                }
                // If "1" or invalid, loop continues to try another file
                dataGen.reset();
                std::cout << "\n";
            } else if (dataGen.getNumColumns() % 2 != 0) {
                std::cout << "CSV file has " << dataGen.getNumColumns() << " columns, but must be even.\n";
                std::cout << "Options:\n";
                std::cout << "  1) Try another CSV file\n";
                std::cout << "  2) Switch to Random mode\n";
                std::cout << "  3) Exit application\n";
                std::cout << "Enter choice (1, 2, or 3): ";

                std::string modeChoice;
                std::getline(std::cin, modeChoice);

                if (modeChoice == "2") {
                    std::cout << "\nSwitching to Random mode...\n\n";
                    choice = "2";
                    break;
                } else if (modeChoice == "3") {
                    std::cout << "Exiting application. Goodbye!\n";
                    return 0;
                }
                // If "1" or invalid, loop continues to try another file
                dataGen.reset();
                std::cout << "\n";
            } else {
                validCols = true;
            }
        }

        if (choice != "2") {
            std::cout << "Loading CSV file: " << filename << "\n\n";
        }
    }

    if (choice == "2") {
        // Random mode
        int rows = 0, cols = 0;

        while (rows <= 0) {
            std::cout << "Enter number of rows (>0): ";
            std::string s;
            std::getline(std::cin, s);
            try { rows = std::stoi(s); } catch (...) { rows = 0; }
            if (rows <= 0) std::cout << "Invalid value.\n";
        }

        while (cols < 10 || cols % 2 != 0) {
            std::cout << "Enter number of columns (must be even and >= 10): ";
            std::string s;
            std::getline(std::cin, s);
            try { cols = std::stoi(s); } catch (...) { cols = 0; }
            if (cols < 10) {
                std::cout << "Columns must be at least 10.\n";
            } else if (cols % 2 != 0) {
                std::cout << "Columns must be even.\n";
            }
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
    std::cout << "Row | Cycle | Pair | DataGen Pair | Filter Output | Buffer Size\n";
    std::cout << "----+-------+------+--------------+---------------+-------------\n";

    int currentRow = 0;
    int pairNum = 0;
    int cycleNum = 0;
    int pairsPerRow = (dataGen.getNumColumns() + 1) / 2;
    std::vector<std::pair<uint8_t, uint8_t>> filteredResults;
    bool showDetailedOutput = true;

    while (!dataGen.isFinished()) {
        pairNum++;
        cycleNum++;
        auto dataPair = dataGen.getNextPair();
        auto [dp1, dp2] = dataPair;

        // Add pair to filter and get result if available
        auto filteredPair = filter.addPairAndFilter(dataPair);
        auto [fp1, fp2] = filteredPair;

        // Display the pair if we're still showing detailed output
        if (showDetailedOutput && pairNum <= 15) {
            if (fp1 != 0 || fp2 != 0) {
                printf("%4d | %5d | %4d | [%3d, %3d]   | [%3d, %3d]    | %11zu\n",
                    currentRow, cycleNum, pairNum, dp1, dp2, fp1, fp2, filter.getBufferSize());
                filteredResults.push_back(filteredPair);
            } else {
                printf("%4d | %5d | %4d | [%3d, %3d]   | (buffering)   | %11zu\n",
                    currentRow, cycleNum, pairNum, dp1, dp2, filter.getBufferSize());
            }
        } else if (showDetailedOutput && pairNum == 15) {
            // Transition point - show ellipsis but still process
            std::cout << "----+-------+------+--------------+---------------+------------ ...\n";
            showDetailedOutput = false;
        }

        // Always collect pairs (even if not displaying)
        if (fp1 != 0 || fp2 != 0) {
            filteredResults.push_back(filteredPair);
        }

        // Check if we're at end of row
        if (pairNum == pairsPerRow) {
            // Capture endOfRow results
            auto endRowResults = filter.endOfRow();

            // Display endOfRow output if still in detailed mode
            if (showDetailedOutput) {
                for (size_t i = 0; i < endRowResults.size(); i++) {
                    cycleNum++;
                    auto [efp1, efp2] = endRowResults[i];
                    printf("%4d | %5d | (END)| -----------   | [%3d, %3d]    | (cleared)\n",
                        currentRow, cycleNum, efp1, efp2);
                    filteredResults.push_back(endRowResults[i]);
                }
            } else {
                // Still collect results but don't display
                for (const auto& result : endRowResults) {
                    cycleNum++;
                    filteredResults.push_back(result);
                }
            }

            currentRow++;
            pairNum = 0;
            cycleNum = 0;
        }
    }

    // Process any remaining pairs after loop ends
    while (!dataGen.isFinished()) {
        pairNum++;
        cycleNum++;
        auto dataPair = dataGen.getNextPair();
        auto filteredPair = filter.addPairAndFilter(dataPair);

        if (filteredPair.first != 0 || filteredPair.second != 0) {
            filteredResults.push_back(filteredPair);
        }

        // Check if we're at end of row
        if (pairNum == pairsPerRow) {
            auto endRowResults = filter.endOfRow();
            for (const auto& result : endRowResults) {
                cycleNum++;
                filteredResults.push_back(result);
            }
            currentRow++;
            pairNum = 0;
            cycleNum = 0;
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

    // Main menu loop
    while (true) {
        std::cout << "Options:\n";
        std::cout << "  1) Run another test with different data\n";
        std::cout << "  2) Exit application\n";
        std::cout << "Enter choice (1 or 2): ";

        std::string menuChoice;
        std::getline(std::cin, menuChoice);

        if (menuChoice == "1") {
            std::cout << "\nRestarting application...\n\n";
            // Restart by returning to main and re-running (will be handled by calling code)
            // For now, we'll just exit and let user restart manually
            std::cout << "Please restart the application to run another test.\n";
            break;
        } else if (menuChoice == "2") {
            std::cout << "\nExiting application. Goodbye!\n";
            return 0;
        } else {
            std::cout << "Invalid choice. Please enter 1 or 2.\n\n";
        }
    }

    return 0;
}