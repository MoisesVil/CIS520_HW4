#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>

#define MAX_LINE_SIZE 1024
#define FILE_NAME "wiki_dump.txt"
#define BUFFER_SIZE 65536  // 64KB buffer for output

// Global variables
int num_lines = 1000000; // We know this file has exactly 1M lines

// Returns the max ASCII value per line
int collect_ascii_values(char *line)
{
    int max_value = 0;
    for (int i = 0; line[i] != '\0'; i++)
    {
        if ((unsigned char)line[i] > max_value)
        {
            max_value = (unsigned char)line[i];
        }
    }
    return max_value;
}

// Function for each process to process its file chunk
void process_lines(int start, int end, char *filename, int *results)
{
    char buffer[MAX_LINE_SIZE];
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error opening file");
        MPI_Abort(MPI_COMM_WORLD, 1);  // Abort if file can't be opened
    }

    // Move to the starting line for this process
    long current_line = 0;
    while (current_line < start && fgets(buffer, MAX_LINE_SIZE, file) != NULL) {
        current_line++;
    }

    // Process lines from start to end or EOF, whichever comes first
    int i = 0;
    while (i < (end - start) && fgets(buffer, MAX_LINE_SIZE, file) != NULL) {
        // Skip empty lines or just newlines
        if (strlen(buffer) <= 1) {
            continue;
        }

        size_t line_len = strlen(buffer);
        if (line_len > 0 && buffer[line_len-1] == '\n') {
            buffer[line_len-1] = '\0';
        }

        // Find max ASCII value for this line
        results[i] = collect_ascii_values(buffer);
        i++;
        current_line++;
    }

    fclose(file);
}

int main(int argc, char *argv[])
{
    // Initialize MPI
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Start timing
    double start_time = MPI_Wtime();

    // Command line argument for file path
    char *filename = FILE_NAME;
    if (argc > 1) {
        filename = argv[1];
    }

    if (rank == 0) {
        printf("Working with %d lines\n", num_lines);
    }

    // Calculate how many lines each process will process
    int lines_per_process = num_lines / size;
    int remaining_lines = num_lines % size;
    int start = rank * lines_per_process + (rank < remaining_lines ? rank : remaining_lines);
    int end = start + lines_per_process + (rank < remaining_lines ? 1 : 0);
    int local_count = end - start;

    // Allocate an array for this process's results
    int *results = malloc(local_count * sizeof(int));
    if (results == NULL) {
        perror("Memory allocation failed");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    // Process lines
    process_lines(start, end, filename, results);

    // Arrays for gathering results
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

    // Gather information about how many elements each process will send
    MPI_Gather(&local_count, 1, MPI_INT, recv_counts, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // Setup displacement array for gatherv
    if (rank == 0) {
        displs[0] = 0;
        for (int i = 1; i < size; i++) {
            displs[i] = displs[i-1] + recv_counts[i-1];
        }
    }

    // Gather results from all processes with variable lengths
    int *all_results = NULL;
    if (rank == 0) {
        all_results = malloc(num_lines * sizeof(int));
        if (all_results == NULL) {
            perror("Memory allocation failed");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
    }

    MPI_Gatherv(results, local_count, MPI_INT, all_results, recv_counts, displs, MPI_INT, 0, MPI_COMM_WORLD);

    // Root process prints results - with efficient output
    if (rank == 0) {
        // Set up buffered output for better performance
        char *output_buffer = malloc(BUFFER_SIZE);
        if (output_buffer == NULL) {
            perror("Output buffer allocation failed");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        
        // Set stdout to use our buffer
        setvbuf(stdout, output_buffer, _IOFBF, BUFFER_SIZE);
        
        // Print all results in batches
        char line_buffer[128];
        for (int i = 0; i < num_lines; i++) {
            // Format the line into a buffer
            int len = snprintf(line_buffer, sizeof(line_buffer), "%d: %d\n", i, all_results[i]);
            
            // Write the buffer to stdout
            fwrite(line_buffer, 1, len, stdout);
        }

        // Calculate and print execution time
        double end_time = MPI_Wtime();
        double execution_time = end_time - start_time;
        printf("Execution time: %.2f seconds\n", execution_time);
        printf("Processed %d lines with %d processes\n", num_lines, size);
        
        // Flush and clean up
        fflush(stdout);
        free(output_buffer);
        
        // Free allocated memory
        free(all_results);
        free(recv_counts);
        free(displs);
    }

    // Cleanup
    free(results);
    MPI_Finalize();

    return 0;
}