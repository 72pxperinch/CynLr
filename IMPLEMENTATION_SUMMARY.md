# FilterBlock Implementation - Summary

## ✅ What Was Implemented

A complete **FilterBlock** that performs row-independent noise filtering on pixel pairs.

### Key Implementation Details:

1. **Row Independence** (Your Requirement)
   - Rows are processed completely independently
   - Buffer is CLEARED at end of each row (`endOfRow()`)
   - Next row data NEVER continues from previous row's buffer
   - Each row starts fresh with pair index = 0

2. **Initialization**
   ```cpp
   FilterBlock filter;
   filter.initialize(numColumnsPerRow);  // Pass columns, not total pairs
   ```

3. **Per-Row Filtering**
   ```cpp
   // Within each row:
   // Pairs 1-4: Accumulate in buffer (no output)
   // Pairs 5-(numPairs-4): Output filtered pairs
   // After pair numPairs: End row, clear buffer

   for (each pair in row) {
       auto filtered = filter.addPairAndFilter(pair);
       if (has output) {
           process(filtered);
       }
   }
   filter.endOfRow();  // Clear buffer, reset for next row
   ```

4. **Examples**

   **10 columns (5 pairs per row):**
   - Pairs 1,2,3,4: Buffered, no output
   - Pair 5: ONLY filterable pair → output
   - End row: buffer cleared
   - Result: 1 filtered pair per row

   **18 columns (9 pairs per row):**
   - Pairs 1,2,3,4: Buffered, no output
   - Pairs 5,6,7,8,9: All filterable → output
   - End row: buffer cleared
   - Result: 5 filtered pairs per row

5. **Convolution Kernel**
   - 9 coefficients (K-4 through K+4)
   - Applied to 9 pixels: 4 before + center + 4 after
   - All 9 pixels must be from SAME row
   - Result clamped to 0-255

## 📁 Files Created/Modified

1. **FilterBlock.h** - Class definition
   - `initialize(int numColsPerRow)` - Init with columns per row
   - `addPairAndFilter()` - Process pairs
   - `endOfRow()` - **CRITICAL** - Clear buffer between rows
   - `reset()` - Reset for reuse
   - `flush()` - Get remaining pairs

2. **FilterBlock.cpp** - Full implementation
   - Kernel coefficients stored
   - Per-row pair indexing
   - Row-independent filtering logic
   - Buffer management

3. **CynLr.cpp** - Updated main demonstration
   - Shows how to use FilterBlock with DataGenerationBlock
   - Calls `endOfRow()` between rows
   - Demonstrates output handling

4. **FILTERBLOCK_README.md** - Complete documentation

## 🔧 Usage Pattern

```cpp
DataGenerationBlock dataGen;
dataGen.loadRandom(numRows, numCols);

FilterBlock filter;
filter.initialize(numCols);

int pairsPerRow = (numCols + 1) / 2;
int pairCount = 0;

while (!dataGen.isFinished()) {
    auto pair = dataGen.getNextPair();
    auto filtered = filter.addPairAndFilter(pair);

    pairCount++;

    // Process filtered output if available
    if (filtered.first != 0 || filtered.second != 0) {
        // Use filtered pair
    }

    // End of row?
    if (pairCount >= pairsPerRow) {
        filter.endOfRow();  // ⭐ MUST call this
        pairCount = 0;
    }
}
```

## ✨ Key Points

- ✅ Rows are independent (buffer cleared at end of row)
- ✅ Each row knows its position via `pairIndexInRow`
- ✅ Only pairs with 4 before + 4 after within same row get filtered
- ✅ Memory efficient: buffer ≤ 5 pairs always
- ✅ Build successful, ready to use
- ✅ Proper edge handling (no row continuation)

## 🚨 Important: Don't Forget `endOfRow()`

**Without calling `filter.endOfRow()` between rows:**
- Buffer will carry over to next row (WRONG)
- Filtering will incorrectly use data from previous row (WRONG)

**With `filter.endOfRow()`:**
- Each row filtered independently (CORRECT)
- Buffer cleared between rows (CORRECT)
- Pair index resets (CORRECT)
