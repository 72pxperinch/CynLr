#include <iostream>
#include "DataGeneration.h"

int main() {
    std::cout << "========================================\n";
    std::cout << "  Data Generation Block - TEST\n";
    std::cout << "========================================\n\n";

    // Create the data generation block
    DataGenerationBlock dataGen;

    // Load CSV file
    const char* filename = "sample_input.csv";
    std::cout << "Loading CSV file: " << filename << "\n\n";

    if (!dataGen.loadCSV(filename)) {
        std::cout << "\nPress Enter to exit...";
        std::cin.get();
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

    std::cout << "========================================\n";
    std::cout << "  Test Complete!\n";
    std::cout << "========================================\n\n";

    std::cout << "Press Enter to exit...";
    std::cin.get();

    return 0;
}