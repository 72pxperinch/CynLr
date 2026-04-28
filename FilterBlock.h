#pragma once
#include <cstdint>          // For uint8_t
#include <utility>          // For std::pair
#include <queue>            // For storing pairs
#include <vector>           // For storing arrays

// FilterBlock consumes pixel pairs in stream order and outputs filtered pairs when a
// full local neighborhood is available. Filtering is row-local; caller must signal
// row boundaries with endOfRow() so context never leaks across rows.
class FilterBlock {
private:
    // Kernel coefficients for filtering (9 elements)
    static constexpr double KERNEL[9] = {
        0.00025177,      // K-4th
        0.008666992,     // K-3rd
        0.078025818,     // K-2nd
        0.24130249,      // K-1st
        0.343757629,     // K (center)
        0.24130249,      // K+1st
        0.078025818,     // K+2nd
        0.008666992,     // K+3rd
        0.000125885      // K+4th
    };

    // Buffer to store pixel values (we need 9 pixels for filtering)
    // But since we work with pairs, we store pairs
    // We need 5 pairs at a time (10 pixels), but only filter when we have enough
    std::vector<std::pair<uint8_t, uint8_t>> pixelBuffer;

    // Track current cycle number
    int cycleCount;

    // Number of columns in the image (needed to know when rows change)
    int numColumns;

    // Pairs per row (numColumns / 2)
    int pairsPerRow;

    // First and last pair indices (within a row) that can be filtered
    int firstFilterablePairInRow;
    int lastFilterablePairInRow;

    // Current pair index within the current row (0-based)
    int pairIndexInRow;

    // Track if last addPairAndFilter had 6+ pairs (meaning it filtered)
    bool lastCallFiltered;

    // Helper function to perform the convolution on a single pixel
    // given the pixel value and its 4 previous and 4 next values
    double filterPixel(const std::vector<uint8_t>& neighborhood);

public:
    // Constructor
    FilterBlock();

    // Initialize with number of columns (each row is independent)
    // Determines which pairs within each row can be filtered
    void initialize(int numColsPerRow);

    // Add a new pair of pixels to the buffer
    // Returns a pair of filtered values if ready to filter, otherwise {0, 0}
    std::pair<uint8_t, uint8_t> addPairAndFilter(const std::pair<uint8_t, uint8_t>& newPair);

    // Signal end of row (no more pairs for current row)
    // Processes remaining buffered pairs before clearing
    // Returns any remaining filtered pairs
    std::vector<std::pair<uint8_t, uint8_t>> endOfRow();

    // Get the current cycle count
    int getCycleCount() const { return cycleCount; }

    // Get current pair index within row
    int getPairIndexInRow() const { return pairIndexInRow; }

    // Get buffer size
    size_t getBufferSize() const { return pixelBuffer.size(); }

    // Reset the filter block
    void reset();
};
