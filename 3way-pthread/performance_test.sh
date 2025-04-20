#!/bin/bash
# Performance testing script for pthread implementation

# Define test parameters
THREAD_COUNTS=(1 2 4 8 16 20)  # Different thread counts to test
ITERATIONS=3                    # Number of runs per configuration
INPUT_FILE="/homes/dan/625/wiki_dump.txt"
OUTPUT_DIR="performance_data"   # Directory to store results

# Create output directory
mkdir -p $OUTPUT_DIR

# Function to run tests for each configuration
run_tests() {
    thread_count=$1
    
    echo "Testing with $thread_count threads..."
    
    # Create directory for this thread count
    thread_dir="$OUTPUT_DIR/threads_$thread_count"
    mkdir -p $thread_dir
    
    # Modify the code to use this thread count
    sed -i "s/#define NUM_THREADS [0-9]*/#define NUM_THREADS $thread_count/" pthread.c
    
    # Compile
    make clean
    make
    
    # Run multiple iterations
    for i in $(seq 1 $ITERATIONS); do
        echo "  Iteration $i of $ITERATIONS"
        
        # Use /usr/bin/time to capture detailed performance metrics
        output_file="$thread_dir/output_$i.txt"
        stats_file="$thread_dir/stats_$i.txt"
        
        # Run the executable with time command
        /usr/bin/time -v ./pthread_max_ascii $INPUT_FILE > $output_file 2> $stats_file
        
        # Extract key performance metrics and save to a summary file
        echo "Thread count: $thread_count, Iteration: $i" >> "$thread_dir/summary.txt"
        grep "User time" $stats_file >> "$thread_dir/summary.txt"
        grep "System time" $stats_file >> "$thread_dir/summary.txt"
        grep "Elapsed" $stats_file >> "$thread_dir/summary.txt"
        grep "Maximum resident set size" $stats_file >> "$thread_dir/summary.txt"
        echo "----------------------------------------" >> "$thread_dir/summary.txt"
    done
}

# Main execution
echo "Starting performance tests..."
echo "Output will be saved to $OUTPUT_DIR/"

# Run tests for each thread count
for tc in "${THREAD_COUNTS[@]}"; do
    run_tests $tc
done

# Create a simple CSV summary of results
echo "Creating summary CSV..."
summary_file="$OUTPUT_DIR/summary.csv"
echo "Threads,Iteration,User_Time(s),System_Time(s),Elapsed_Time(s),Memory(KB)" > $summary_file

for tc in "${THREAD_COUNTS[@]}"; do
    thread_dir="$OUTPUT_DIR/threads_$tc"
    
    for i in $(seq 1 $ITERATIONS); do
        stats_file="$thread_dir/stats_$i.txt"
        
        # Extract metrics
        user_time=$(grep "User time" $stats_file | awk '{print $4}')
        system_time=$(grep "System time" $stats_file | awk '{print $4}')
        elapsed_time=$(grep "Elapsed" $stats_file | awk '{print $8}')
        memory=$(grep "Maximum resident set size" $stats_file | awk '{print $6}')
        
        # Add to CSV
        echo "$tc,$i,$user_time,$system_time,$elapsed_time,$memory" >> $summary_file
    done
done

echo "Performance testing completed. Results are in $OUTPUT_DIR/"