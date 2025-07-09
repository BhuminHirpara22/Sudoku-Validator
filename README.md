# Sudoku Validator - Parallel Computing Performance Analysis

## Project Overview

This project implements and compares different parallel synchronization techniques for Sudoku grid validation. It demonstrates the performance characteristics of various synchronization primitives in a multi-threaded environment by validating Sudoku puzzles using different approaches.

## Implemented Algorithms

The project includes four different implementations:

1. **Sequential** (`sequential.cpp`) - Single-threaded baseline implementation
2. **Test-and-Set (TAS)** (`TAS.cpp`) - Uses atomic_flag with test-and-set for mutual exclusion
3. **Compare-and-Swap (CAS)** (`CAS.cpp`) - Uses atomic<bool> with compare-and-exchange operations
4. **Bounded CAS** (`BoundedCAS.cpp`) - Uses bounded waiting with compare-and-exchange and thread queuing

## Features

- **Sudoku Validation**: Validates rows, columns, and sub-grids of NxN Sudoku puzzles (where N is a perfect square)
- **Performance Metrics**: Measures and logs:
  - Total execution time
  - Average time to enter critical section
  - Average time to exit critical section  
  - Worst-case entry and exit times
- **Thread Synchronization**: Demonstrates different mutual exclusion techniques
- **Detailed Logging**: Timestamped logs of thread activities for analysis
- **Experimental Framework**: Python script for automated testing and performance comparison

## Project Structure

```
├── sequential.cpp      # Sequential implementation (baseline)
├── TAS.cpp            # Test-and-Set implementation
├── CAS.cpp            # Compare-and-Swap implementation
├── BoundedCAS.cpp     # Bounded CAS with fair scheduling
├── gen.py             # Test generation and benchmarking script
├── Report.pdf         # Detailed analysis report
└── exp*_*.png         # Performance comparison graphs
```

## Input Format

The input file `input.txt` contains:
```
k n taskInc
sudoku_grid
```

Where:
- `k` = Number of threads
- `n` = Grid size (must be a perfect square: 4, 9, 16, 25, etc.)
- `taskInc` = Number of tasks each thread processes at once
- `sudoku_grid` = n lines of n space-separated integers (1 to n)

## Compilation and Usage

### Compile the programs:
```bash
g++ -o sequential sequential.cpp -pthread
g++ -o TAS TAS.cpp -pthread
g++ -o CAS CAS.cpp -pthread  
g++ -o BoundedCAS BoundedCAS.cpp -pthread
```

### Run individual programs:
```bash
./sequential    # or ./TAS or ./CAS or ./BoundedCAS
```

### Generate test data and run experiments:
```bash
python3 gen.py
```

## How It Works

### Sudoku Validation Process
1. **Row Validation**: Check each row for duplicate numbers
2. **Column Validation**: Check each column for duplicate numbers  
3. **Sub-grid Validation**: Check each sqrt(n)×sqrt(n) sub-grid for duplicates

### Parallel Implementation
- Tasks are distributed among threads dynamically
- Critical sections protect shared variables (task counter, validity flag)
- Different synchronization primitives provide different performance characteristics
- Early termination when invalid sudoku is detected

### Synchronization Techniques

1. **Test-and-Set (TAS)**: Simple spinning lock using atomic_flag
2. **Compare-and-Swap (CAS)**: Lock-free synchronization with atomic operations
3. **Bounded CAS**: Fair scheduling that prevents thread starvation

## Performance Analysis

The project includes comprehensive performance analysis with:
- Execution time comparisons across different thread counts
- Critical section entry/exit time measurements
- Scalability analysis with varying workloads
- Visual performance graphs (exp*.png files)

## Key Concepts Demonstrated

- **Mutual Exclusion**: Protecting shared resources in concurrent environments
- **Lock-free Programming**: Using atomic operations for synchronization
- **Thread Fairness**: Preventing starvation in multi-threaded systems
- **Performance Measurement**: Benchmarking parallel algorithms
- **Critical Section Analysis**: Understanding synchronization overhead

## Dependencies

- C++11 or later (for atomic operations and threading)
- pthread library
- Python 3 with matplotlib (for graph generation)
- Linux/Unix environment (for compilation and execution)

## Output

Each program generates `output.txt` containing:
- Timestamped thread activity logs
- Validation results (valid/invalid sudoku)
- Performance metrics (execution times, critical section statistics)

## Author

This project demonstrates parallel computing concepts and synchronization primitives through practical Sudoku validation, providing insights into the trade-offs between different concurrent programming approaches.