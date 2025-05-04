#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>

#define MAX_LINE_SIZE 1024
#define FILE_NAME "wiki_dump.txt"
#define BUFFER_SIZE 65536  // 64KB buffer for output
#define TOTAL_LINES 1000000

// Returns the max ASCII value per line
int collect_ascii_values(char *line) {
    int max_value = 0;
    int len = strlen(line);
    
    for (int i = 0; i < len; i++) {
        unsigned char c = (unsigned char)line[i];
        if (c > max_value) {
            max_value = c;
        }
    }
    return max_value;
}

// Function for each process to read and process its own chunk
void process_chunk(int start, int end, char *filename, int *results) {
    char buffer[MAX_LINE_SIZE];
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error opening file");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    // Skip to start position
    for (int i = 0; i < start && fgets(buffer, MAX_LINE_SIZE, file) != NULL; i++) {
        // Skip lines until we reach our starting position
    }
    
    // Process assigned lines
    for (int i = 0; i < (end - start) && fgets(buffer, MAX_LINE_SIZE, file) != NULL; i++) {
        // Remove newline if present
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len-1] == '\n') {
            buffer[len-1] = '\0';
        }
        
        // Find max ASCII value
        results[i] = collect_ascii_values(buffer);
    }
    
    fclose(file);
}

int main(int argc, char *argv[]) {
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    double start_time = MPI_Wtime();

    // Set filename from command line or use default
    char *filename = FILE_NAME;
    if (argc > 1) {
        filename = argv[1];
    }
    
    // Each process calculates its chunk
    int lines_per_process = TOTAL_LINES / size;
    int remaining_lines = TOTAL_LINES % size;
    
    int start_line = rank * lines_per_process + (rank < remaining_lines ? rank : remaining_lines);
    int end_line = start_line + lines_per_process + (rank < remaining_lines ? 1 : 0);
    int local_count = end_line - start_line;

    // Allocate memory for local results
    int *local_results = malloc(local_count * sizeof(int));
    if (local_results == NULL) {
        perror("Memory allocation failed");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }
    
    // Each process reads and processes its own chunk directly
    process_chunk(start_line, end_line, filename, local_results);
    
    // Prepare for gathering results
    int *recv_counts = NULL;
    int *displs = NULL;
    
    if (rank == 0) {
        recv_counts = malloc(size * sizeof(int));
        displs = malloc(size * sizeof(int));
        
        if (recv_counts == NULL || displs == NULL) {
            perror("Memory allocation failed");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
    }
    
    // Gather all counts to know how many results to expect from each process
    MPI_Gather(&local_count, 1, MPI_INT, recv_counts, 1, MPI_INT, 0, MPI_COMM_WORLD);
    
    // Setup displacement array on rank 0
    if (rank == 0) {
        displs[0] = 0;
        for (int i = 1; i < size; i++) {
            displs[i] = displs[i-1] + recv_counts[i-1];
        }
    }
    
    // Gather results from all processes
    int *all_results = NULL;
    if (rank == 0) {
        all_results = malloc(TOTAL_LINES * sizeof(int));
        if (all_results == NULL) {
            perror("Memory allocation failed");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
    }
    
    MPI_Gatherv(local_results, local_count, MPI_INT, 
                all_results, recv_counts, displs, MPI_INT, 
                0, MPI_COMM_WORLD);
    
    // Print results from rank 0
    if (rank == 0) {
        // Set up buffered output
        char *output_buffer = malloc(BUFFER_SIZE);
        if (output_buffer == NULL) {
            perror("Output buffer allocation failed");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        
        setvbuf(stdout, output_buffer, _IOFBF, BUFFER_SIZE);
        
        // Print results in batches for better performance
        char line_buffer[128];
        for (int i = 0; i < TOTAL_LINES; i++) {
            int len = snprintf(line_buffer, sizeof(line_buffer), "%d: %d\n", i, all_results[i]);
            fwrite(line_buffer, 1, len, stdout);
        }
        
        // Print timing information
        double end_time = MPI_Wtime();
        printf("Execution time: %.2f seconds\n", end_time - start_time);
        printf("Processed %d lines with %d processes\n", TOTAL_LINES, size);
        
        // Cleanup
        fflush(stdout);
        free(output_buffer);
        free(all_results);
        free(recv_counts);
        free(displs);
    }
    
    // Cleanup and finalize
    free(local_results);
    MPI_Finalize();
    
    return 0;
}