#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define NUM_THREADS 20
#define MAX_LINE_SIZE 1024
#define FILE_NAME "wiki_dump.txt"

typedef struct {
    int start;
    int end;
    char **lines;  // Array of pointers to lines
    int *results;  // Pointer to main results array
} ThreadData;

// Find max ASCII value in a line
int collect_ascii_values(char *line) {
    int max_value = 0;
    for (int i = 0; line[i] != '\0'; i++) {
        if ((unsigned char)line[i] > max_value) {
            max_value = (unsigned char)line[i];
        }
    }
    return max_value;
}

// Thread routine
void *process_lines(void *arg) {
    ThreadData *data = (ThreadData *)arg;
    for (int i = data->start; i < data->end; i++) {
        data->results[i] = collect_ascii_values(data->lines[i]);
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    clock_t start_time = clock();

    char *filename = (argc > 1) ? argv[1] : FILE_NAME;

    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Error opening file");
        return 1;
    }

    // Estimate initial capacity for 1 million lines
    size_t capacity = 1000000;
    size_t num_lines = 0;
    char **lines = malloc(capacity * sizeof(char *));
    if (!lines) {
        perror("Memory allocation failed");
        fclose(file);
        return 1;
    }

    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    // Read all lines into memory
    while ((read = getline(&line, &len, file)) != -1) {
        // Strip newline
        if (read > 0 && line[read - 1] == '\n') {
            line[--read] = '\0';
        }

        // Reallocate if needed
        if (num_lines >= capacity) {
            capacity *= 2;
            lines = realloc(lines, capacity * sizeof(char *));
            if (!lines) {
                perror("Reallocation failed");
                fclose(file);
                return 1;
            }
        }

        // Copy line into array
        lines[num_lines] = strdup(line);
        if (!lines[num_lines]) {
            perror("Line strdup failed");
            fclose(file);
            return 1;
        }
        num_lines++;
    }

    free(line);
    fclose(file);

    printf("Total lines read: %zu\n", num_lines);

    // Allocate result array
    int *results = malloc(num_lines * sizeof(int));
    if (!results) {
        perror("Result array allocation failed");
        return 1;
    }

    pthread_t threads[NUM_THREADS];
    ThreadData thread_data[NUM_THREADS];

    int lines_per_thread = num_lines / NUM_THREADS;
    int remainder = num_lines % NUM_THREADS;
    int start = 0;

    // Create threads
    for (int i = 0; i < NUM_THREADS; i++) {
        int extra = (i < remainder) ? 1 : 0;
        int end = start + lines_per_thread + extra;

        thread_data[i].start = start;
        thread_data[i].end = end;
        thread_data[i].lines = lines;
        thread_data[i].results = results;

        pthread_create(&threads[i], NULL, process_lines, &thread_data[i]);
        start = end;
    }

    // Wait for all threads
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    for (size_t i = 0; i < num_lines; i++) {
        printf("%zu: %d\n", i, results[i]);
    }
    
    for (size_t i = 0; i < num_lines; i++) {
        free(lines[i]);
    }
    free(lines);
    free(results);

    // Timing
    clock_t end_time = clock();
    double duration = (double)(end_time - start_time) / CLOCKS_PER_SEC;
    printf("Execution time: %.2f seconds\n", duration);

    return 0;
}
