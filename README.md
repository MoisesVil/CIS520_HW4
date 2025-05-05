# Project 4: Parallel ASCII Analysis

This repository contains implementations of a parallel program to find the maximum ASCII value in each line of a large text file. The program is implemented using three different parallel programming approaches: pthreads, MPI, and OpenMP.


## Repository Structure

- `/3way-pthread`: pthread implementation
- `/3way-mpi`: MPI implementation  
- `/3way-openmp`: OpenMP implementation
- `design4.pdf`: Design document with performance analysis
- `README.md`: This file

### Setup
Load required modules:
   ```
   module load CMake/3.23.1-GCCcore-11.3.0 foss/2022a OpenMPI/4.1.4-GCC-11.3.0
   ```


### Compiling and Running

Each implementation directory (`3way-pthread`, `3way-mpi`, `3way-openmp`) contains its own Makefile and submission script.

For each directory, you can compile and run the performance tests with a single command:

```bash
cd 3way-pthread
./submit.sh; python analyze_results.py
```

```bash
cd 3way-mpi
./submit.sh; python analyze_results.py
```

```bash
cd 3way-openmp
./submit.sh; python analyze_results.py
```

The results will be stored in the `performance_data` directory, and graphs will be generated in the `plots` directory.
