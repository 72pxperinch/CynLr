# Design Overview Document - Evaluation 1 (Programming Fundamentals - C++)

## Author Note

I come from a background with zero formal computer science and C++ knowledge, and I still tried to execute the idea in a structured and practical way by breaking the problem into clear functional blocks, keeping memory bounded, and validating behavior with runnable tests.

## 1) Goal and Scope

This submission implements the first two blocks from the given process pipeline:

1. Data Generation Block
2. Filter Block (convolution filtering)

The current codebase focuses on:

- Pixel-pair streaming flow
- Row-wise processing
- Memory-aware buffering
- Testability via CSV and random generation modes

Thresholding is discussed in design (where it fits) but not yet implemented in code.

## 2) Technology and Build Context

- Language: C++20
- Toolchain target in project: Visual Studio/MSBuild (v145 toolset)
- Configurations: Win32, x64, ARM64 (Debug/Release)
- Runtime dependencies: none beyond standard C++ library

## 3) High-Level Architecture

The implementation follows a simple pipeline-style, block-oriented design.

### 3.1 Functional Blocks

1. `DataGenerationBlock`
- Owns input data source
- Supports two modes:
  - CSV mode (`loadCSV`)
  - Random mode (`loadRandom`)
- Emits stream as `std::pair<uint8_t, uint8_t>` through `getNextPair`

2. `FilterBlock`
- Accepts incoming pixel pairs
- Maintains a small sliding buffer
- Computes convolution output when sufficient context exists
- Treats each row independently (`endOfRow` clears state)

3. Driver/Integration (`main` in `CynLr.cpp`)
- Collects user inputs (mode, file, random dimensions)
- Validates geometry constraints
- Streams data from source to filter block
- Prints diagnostics and summary counters

### 3.2 Communication Mechanism Between Blocks

Current mechanism is direct in-process method invocation:

- Producer: `DataGenerationBlock::getNextPair()`
- Consumer: `FilterBlock::addPairAndFilter(pair)`

Data unit is a pair of consecutive pixels. The contract between blocks is intentionally minimal and type-stable (`std::pair<uint8_t, uint8_t>`), keeping coupling low.

### 3.3 Why This Pattern

This resembles a Pipes-and-Filters style architecture with explicit row-boundary signaling.

Benefits:

- Easy to reason about data flow
- Easy to insert future blocks (Threshold, Labeling, Tracing)
- Supports streaming behavior without requiring full-image materialization

## 4) Data Generation Block Design

### 4.1 Responsibilities

- Parse and validate CSV rows
- Clamp numeric values into `[0, 255]`
- Auto-detect column count from first valid row
- Support random synthetic source for stress testing
- Emit two consecutive pixels per call in row-major order

### 4.2 Key Behaviors

- Handles empty/blank lines
- Warns on ragged CSV rows but keeps streaming available data
- If total number of pixels is odd, appends a padding zero pixel so pair-stream alignment is preserved
- `reset()` rewinds stream without reloading source

### 4.3 Complexity and Memory

- Loading CSV: `O(total_pixels)`
- Streaming pair output: amortized `O(1)` per call
- Storage: `O(total_pixels)` for loaded input matrix (current implementation is batch-loaded)

## 5) Filter Block Design

### 5.1 Mathematical Model Implemented

1D convolution with 9-tap kernel around each center pixel:

$$
y[k] = \sum_{i=-4}^{4} x[k+i] \cdot h[i]
$$

Kernel coefficients are implemented as constants:

- `0.00025177`
- `0.008666992`
- `0.078025818`
- `0.24130249`
- `0.343757629`
- `0.24130249`
- `0.078025818`
- `0.008666992`
- `0.000125885`

### 5.2 Streaming Strategy

- Input arrives as pixel pairs
- Buffer holds pair history for local context
- Filtering starts only when neighborhood is available
- Output is emitted as pair when calculation is possible

### 5.3 Row Independence

Important behavior implemented:

- Filtering context does not cross rows
- `endOfRow()` flushes final valid case and clears internal buffer
- `pairIndexInRow` resets per row

This enforces strict row-local semantics expected in line-scan style processing.

### 5.4 Complexity and Memory

- Fixed kernel size => constant-time arithmetic per filtered output
- Internal buffer size remains bounded and small
- Memory growth does not scale with number of rows

## 6) Requirement-to-Implementation Mapping

### 6.1 Implemented

1. Switchable source mode
- CSV test mode and random mode are both available.

2. Pixel pair streaming interface
- Data generation emits 2 consecutive elements each call.

3. Convolution filtering
- 9-element window and provided coefficients are implemented.

4. Row-level modularity
- Filter buffer reset is explicit at row boundaries.

5. Input validation and safe fallbacks
- Invalid file handling, value clamping, odd-pixel padding, geometry checks.

### 6.2 Partially Implemented / Design Intent Present

1. Parallel process execution under process time `T`
- Current code is functionally pipelined but runs in a single thread in sequence.
- The design can be extended to true parallel block execution using lock-free SPSC queues.

2. Threshold block behavior
- Expected threshold logic location is immediately after filter output.
- Actual compare-to-threshold binary conversion is not yet in code.

3. Throughput target `<100ns` and explicit `T` compliance measurement
- Functional flow is implemented; micro-benchmark harness is not yet integrated.

4. Maximum memory `<= m`
- Filter internal memory is bounded and small.
- Generator currently stores full dataset in memory for testability, so strict `<= m` global memory target is not yet satisfied.

## 7) Modularity and Scalability Assessment

### 7.1 Current Modularity

- Source block and filter block are class-separated
- Public APIs are clean and block-specific
- Driver is thin orchestration logic

### 7.2 How It Scales to More Blocks

Pipeline can grow with minimal block rewrites by standardizing block I/O contracts:

Proposed contracts:

- `PixelPair` input/output between generation and filtering
- `FilteredPair` + metadata between filtering and thresholding
- `BinaryPair` between thresholding and labeling

Future block insertion points:

1. `ThresholdBlock` after `FilterBlock`
2. `LabelBlock` after threshold
3. `TraceComputeBlock` after labeling

### 7.3 Suggested Architecture Upgrade (for true process model)

- One thread per block
- SPSC ring buffers between adjacent blocks
- Time-budget instrumentation per block (`start`, `end`, cycle-time)
- Backpressure policy when consumer lags

This preserves existing block APIs while introducing real-time pipeline execution.

## 8) Timing and Memory Profiling Strategy (Evaluation-Friendly)

The code currently prints functional counters. To satisfy evaluation profiling expectations, the recommended measurable plan is:

1. Per-module timing
- Wrap each block call with high-resolution timestamp
- Track min/avg/max and p95/p99 per cycle

2. Throughput
- Measure delta between successive output pixels
- Verify `<100ns` only in Release + optimized build on target hardware

3. Memory
- DataGeneration: report source storage bytes
- FilterBlock: report max buffer bytes observed
- Total process memory: sample peak resident memory per run

4. Report format
- Separate table for each module: `calls`, `avg_ns`, `max_ns`, `peak_bytes`

## 9) Unit Test Strategy

### 9.1 DataGeneration Tests

1. CSV load success and geometry detection
2. Invalid file path handling
3. Value clamping behavior (`<0`, `>255`)
4. Odd pixel count padding behavior
5. Reset and deterministic replay behavior

### 9.2 FilterBlock Tests

1. No output before enough context
2. Correct output count per row size
3. End-of-row flush correctness
4. Buffer cleared between rows (no cross-row contamination)
5. Kernel arithmetic correctness with deterministic input vectors

### 9.3 Integration Tests

1. CSV -> stream -> filter full path
2. Random -> stream -> filter full path
3. Multiple rows with fixed expected filtered outputs

## 10) Known Gaps and Honest Limitations

1. Threshold conversion block is not coded yet
2. True parallel timing model with `T` is not implemented yet
3. Profiling harness for strict nanosecond SLA is not yet integrated
4. Memory target `<= m` is not met globally when full CSV is loaded

These are identified clearly so evaluation can separate implemented functionality from pending work.

## 11) Execution Instructions (Current Project)

1. Open solution/project in Visual Studio 2017+ (recommended newer VS).
2. Build in Release configuration.
3. Run executable.
4. Choose mode:
   - CSV mode: provide file name (default `sample_input.csv`)
   - Random mode: provide rows and even columns >= 10
5. Observe pair stream, filter stage output, and summary counters.

## 12) Deliverables Included in Repository

1. Source code for data generation and filtering blocks
2. Integration driver executable entry (`main`)
3. Sample input CSV
4. Filter block documentation
5. Implementation summary
6. This design overview document

## 13) Evaluation Summary

This implementation demonstrates:

- Functional decomposition into modular blocks
- Streaming-style pair processing
- Correct use of 9-tap convolution logic
- Row-isolated filtering behavior
- A practical base architecture that can be scaled to full multi-block pipeline

It intentionally favors clarity and correctness of core flow first, with a clean path to evolve into strict real-time, threaded, and fully profiled production behavior.