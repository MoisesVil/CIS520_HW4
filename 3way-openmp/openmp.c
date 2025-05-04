#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE_SIZE 1024
#define FILE_NAME "wiki_dump.txt"
#define MAX_LINES 1000000
#define BUFFER_SIZE 65536  // 64KB buffer for output

// Returns the max ASCII value per line
int collect_ascii_values(char *line) {
    int max_value = 0;
    int len = strlen(line);

    #pragma omp simd reduction(max:max_value)
    for (int i = 0; i < len; i++) {
        unsigned char c = (unsigned char)line[i];
        if (c > max_value) {
            max_value = c;
        }
    }

    return max_value;
}


int main(int argc, char *argv[]) {
    double start_time = omp_get_wtime();
    
    char *filename = FILE_NAME;
    if (argc > 1) {
        filename = argv[1];
    }
    
    // Open file and read all lines into memory once
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Error opening file");
        return 1;
    }
    
    // Allocate space for all lines
    char **lines = malloc(MAX_LINES * sizeof(char *));
    if (lines == NULL) {
        perror("Memory allocation failed");
        fclose(file);
        return 1;
    }
    
    // Read all lines into memory sequentially
    char buffer[MAX_LINE_SIZE];
    int line_count = 0;
    
    while (fgets(buffer, MAX_LINE_SIZE, file) != NULL && line_count < MAX_LINES) {
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len-1] == '\n') {
            buffer[len-1] = '\0';
        }
        
        lines[line_count] = strdup(buffer);
        if (lines[line_count] == NULL) {
            perror("Memory allocation failed");
            fclose(file);
            return 1;
        }
        line_count++;
    }
    
    fclose(file);
    printf("Read %d lines from file\n", line_count);
    
    // Allocate array for results
    int *results = malloc(line_count * sizeof(int));
    if (results == NULL) {
        perror("Memory allocation failed");
        return 1;
    }
    
    // Process lines in parallel with OpenMP
    int num_threads = omp_get_max_threads();
    printf("Processing with %d threads\n", num_threads);
    
    // Optimize with static scheduling and chunk size
    // This helps reduce thread management overhead and can improve cache locality
    const int CHUNK_SIZE = 64;  // Try different values: 32, 64, 128
    
    #pragma omp parallel for schedule(static, CHUNK_SIZE)
    for (int i = 0; i < line_count; i++) {
        results[i] = collect_ascii_values(lines[i]);
    }
    
    // Set up buffered output for better performance
    char *output_buffer = malloc(BUFFER_SIZE);
    if (output_buffer == NULL) {
        perror("Output buffer allocation failed");
        exit(1);
    }
    
    // Set stdout to use our buffer
    setvbuf(stdout, output_buffer, _IOFBF, BUFFER_SIZE);
    
    // Print all results in batches
    char line_buffer[128];
    for (int i = 0; i < line_count; i++) {
        // Format the line into a buffer
        int len = snprintf(line_buffer, sizeof(line_buffer), "%d: %d\n", i, results[i]);
        
        // Write the buffer to stdout
        fwrite(line_buffer, 1, len, stdout);
        free(lines[i]);
    }
    
    // Calculate and print execution time
    double end_time = omp_get_wtime();
    double execution_time = end_time - start_time;
    printf("Execution time: %.2f seconds\n", execution_time);
    printf("Processed %d lines with %d threads\n", line_count, num_threads);
    
    // Flush and clean up
    fflush(stdout);
    free(output_buffer);
    free(lines);
    free(results);
    
    return 0;
}