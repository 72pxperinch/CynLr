#include "FilterBlock.h"
#include <cmath>
#include <iostream>
#include <algorithm>

// Static constant initialization
constexpr double FilterBlock::KERNEL[9];

// Constructor
FilterBlock::FilterBlock()
    : cycleCount(0), numColumns(0), pairsPerRow(0), 
      firstFilterablePairInRow(-1), lastFilterablePairInRow(-1), pairIndexInRow(0), 
      lastCallFiltered(false) {
    pixelBuffer.clear();
}

// Initialize with number of columns (each row is independent)
void FilterBlock::initialize(int numColsPerRow) {
    numColumns = numColsPerRow;
    pairsPerRow = (numColumns + 1) / 2;  // m/2 pairs per row
    pairIndexInRow = 0;

    // Within each row, to filter a pair at index i, we need:
    // - 2 pairs before it (indices i-2 to i-1) = 4 pixels
    // - 1 pair (the pair itself) = 2 pixels
    // - 2 pairs after it (indices i+1 to i+2) = 4 pixels
    // Total = 5 pairs needed
    // So i must be in range [2, pairsPerRow-3]

    firstFilterablePairInRow = 2;  // 3rd pair in row (0-based index 2)
    lastFilterablePairInRow = pairsPerRow - 3;  // Last filterable pair in row

    if (firstFilterablePairInRow > lastFilterablePairInRow) {
        // Not enough pairs in a row to filter any
        std::cout << "WARNING: Columns per row (" << numColumns << ") results in " 
                  << pairsPerRow << " pairs.\n";
        std::cout << "         Need at least 5 pairs per row to filter any.\n";
        firstFilterablePairInRow = -1;
        lastFilterablePairInRow = -1;
    } else {
        int filterablePairsPerRow = lastFilterablePairInRow - firstFilterablePairInRow + 1;
        std::cout << "FilterBlock initialized:\n";
        std::cout << "  - Columns per row: " << numColumns << "\n";
        std::cout << "  - Pairs per row: " << pairsPerRow << "\n";
        std::cout << "  - Filterable pairs per row: " << filterablePairsPerRow
                  << " (pairs " << (firstFilterablePairInRow + 1) << " to " 
                  << (lastFilterablePairInRow + 1) << ")\n";
    }
}

// Helper function to perform convolution on a single pixel
double FilterBlock::filterPixel(const std::vector<uint8_t>& neighborhood) {
    // neighborhood should have exactly 9 elements
    // Element at index 4 is the center pixel to be filtered
    if (neighborhood.size() != 9) {
        std::cerr << "ERROR: neighborhood size is " << neighborhood.size() 
                  << ", expected 9\n";
        return 0.0;
    }

    double result = 0.0;
    for (int i = 0; i < 9; ++i) {
        result += static_cast<double>(neighborhood[i]) * KERNEL[i];
    }

    return result;
}

// Add a new pair of pixels and filter if we have enough data
std::pair<uint8_t, uint8_t> FilterBlock::addPairAndFilter(const std::pair<uint8_t, uint8_t>& newPair) {
    cycleCount++;
    int pairIndex = pairIndexInRow++;  // Get current index in row, then increment

    // Add the new pair to buffer
    pixelBuffer.push_back(newPair);

    lastCallFiltered = false;  // Assume no filtering happens

    // If we haven't initialized, we can't process anything
    if (firstFilterablePairInRow < 0) {
        return {0, 0};
    }

    // We need at least 6 pairs in buffer to safely filter and rotate
    if (pixelBuffer.size() < 6) {
        // Keep accumulating data
        return {0, 0};
    }

    // Now we have 6+ pairs in buffer. We can safely filter the 3rd pair and rotate
    lastCallFiltered = true;  // Mark that we're filtering

    // Extract all pixels from buffer
    std::vector<uint8_t> allPixels;
    for (const auto& pair : pixelBuffer) {
        allPixels.push_back(pair.first);
        allPixels.push_back(pair.second);
    }

    // Filter the 3rd pair in buffer (pixels at indices 4 and 5)
    std::vector<uint8_t> neighborhood1(allPixels.begin(), allPixels.begin() + 9);
    uint8_t filteredPixel1 = static_cast<uint8_t>(
        std::max(0.0, std::min(255.0, filterPixel(neighborhood1)))
    );

    std::vector<uint8_t> neighborhood2(allPixels.begin() + 1, allPixels.begin() + 10);
    uint8_t filteredPixel2 = static_cast<uint8_t>(
        std::max(0.0, std::min(255.0, filterPixel(neighborhood2)))
    );

    // Remove the first pair and keep 5+ pairs for the next cycle
    pixelBuffer.erase(pixelBuffer.begin());

    return {filteredPixel1, filteredPixel2};
}



// Signal end of row
// Processes the final buffered pair if not yet processed, then clears for next row
std::vector<std::pair<uint8_t, uint8_t>> FilterBlock::endOfRow() {
    std::vector<std::pair<uint8_t, uint8_t>> result;

    // Only filter in endOfRow if the last addPairAndFilter() call didn't already filter
    // This happens when we have exactly 5 pairs and no 6th pair came
    if (pixelBuffer.size() == 5 && !lastCallFiltered) {
        std::vector<uint8_t> allPixels;
        for (const auto& pair : pixelBuffer) {
            allPixels.push_back(pair.first);
            allPixels.push_back(pair.second);
        }

        // Filter the 3rd pair in buffer (pixels at indices 4 and 5)
        std::vector<uint8_t> neighborhood1(allPixels.begin(), allPixels.begin() + 9);
        uint8_t filteredPixel1 = static_cast<uint8_t>(
            std::max(0.0, std::min(255.0, filterPixel(neighborhood1)))
        );

        std::vector<uint8_t> neighborhood2(allPixels.begin() + 1, allPixels.begin() + 10);
        uint8_t filteredPixel2 = static_cast<uint8_t>(
            std::max(0.0, std::min(255.0, filterPixel(neighborhood2)))
        );

        result.push_back({filteredPixel1, filteredPixel2});
    }

    // Clear buffer for next row
    pixelBuffer.clear();
    pairIndexInRow = 0;
    lastCallFiltered = false;
    return result;
}

// Reset the filter block (for reuse within same image)
// Note: Keep initialization - numColumns, pairsPerRow, filterable range persist
void FilterBlock::reset() {
    cycleCount = 0;
    pairIndexInRow = 0;
    pixelBuffer.clear();
}
