#!/bin/bash
# Performance testing script for MPI implementation

PROCESS_COUNTS=(1 2 4 8 16 20)
ITERATIONS=3
INPUT_FILE="/homes/dan/625/wiki_dump.txt"
OUTPUT_DIR="performance_data"

mkdir -p $OUTPUT_DIR

run_tests() {
    proc_count=$1
    echo "Testing with $proc_count processes..."

    proc_dir="$OUTPUT_DIR/procs_$proc_count"
    mkdir -p $proc_dir

    make clean
    make

    for i in $(seq 1 $ITERATIONS); do
        echo "  Iteration $i of $ITERATIONS"

        output_file="$proc_dir/output_$i.txt"
        stats_file="$proc_dir/stats_$i.txt"

        /usr/bin/time -v mpirun --oversubscribe -np $proc_count ./mpi_max_ascii $INPUT_FILE > $output_file 2> $stats_file

        echo "Process count: $proc_count, Iteration: $i" >> "$proc_dir/summary.txt"
        grep "User time" $stats_file >> "$proc_dir/summary.txt"
        grep "System time" $stats_file >> "$proc_dir/summary.txt"
        grep "Elapsed" $stats_file >> "$proc_dir/summary.txt"
        grep "Maximum resident set size" $stats_file >> "$proc_dir/summary.txt"
        echo "----------------------------------------" >> "$proc_dir/summary.txt"
    done
}

echo "Starting MPI performance tests..."
echo "Results will be in $OUTPUT_DIR/"

for pc in "${PROCESS_COUNTS[@]}"; do
    run_tests $pc
done

summary_file="$OUTPUT_DIR/summary.csv"
echo "Processes,Iteration,User_Time(s),System_Time(s),Elapsed_Time(s),Memory(KB)" > $summary_file

for pc in "${PROCESS_COUNTS[@]}"; do
    proc_dir="$OUTPUT_DIR/procs_$pc"
    for i in $(seq 1 $ITERATIONS); do
        stats_file="$proc_dir/stats_$i.txt"
        user_time=$(grep "User time" $stats_file | awk '{print $4}')
        system_time=$(grep "System time" $stats_file | awk '{print $4}')
        elapsed_time=$(grep "Elapsed" $stats_file | awk '{print $8}')
        memory=$(grep "Maximum resident set size" $stats_file | awk '{print $6}')
        echo "$pc,$i,$user_time,$system_time,$elapsed_time,$memory" >> $summary_file
    done
done

echo "Performance testing complete. See $OUTPUT_DIR/"
