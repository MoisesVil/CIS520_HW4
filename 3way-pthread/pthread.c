#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NUM_THREADS 4
#define MAX_LINE_SIZE 1024
#define FILE_NAME "wiki_dump.txt"

// This struct will hold the starting and ending lines that a thread will process
typedef struct
{
	int start;
	int end;
	char *filename;
} ThreadData;

int num_lines = 0; // Will hold the total number of lines within the file
int store_max_values[NUM_THREADS]; // Will store the max ASCII values for each line

// Returns the max ASCII value per line
int collect_ascii_values(char *line) 
{
	int max_value = 0;
	for (int i = 0; line[i] != '\0'; i++)
	{
		if (line[i] > max_value)
		{
			max_value = line[i];
		}
	}
	return max_value;
}

// Function executed by each thread
void *process_lines(void *arg)
{
    ThreadData *data = (ThreadData *)arg;
    char *line = NULL;
    size_t len = 0;
    
    FILE *file = fopen(data->filename, "r");
    if (file == NULL) {
        perror("Error opening file");
        pthread_exit(NULL);
    }
    
    // Move to the starting line for this thread
    int current_line = 0;
    while (current_line < data->start && getline(&line, &len, file) != -1) {
        current_line++;
    }
    
    // Process assigned lines
    int *results = malloc((data->end - data->start) * sizeof(int));
    int i = 0;
    
    while (current_line < data->end && getline(&line, &len, file) != -1) {
        size_t line_len = strlen(line);
        if (line_len > 0 && line[line_len-1] == '\n') {
            line[line_len-1] = '\0';
        }
        
        // Find max ASCII value for this line
        results[i] = collect_ascii_values(line);
        i++;
        current_line++;
    }
    
    free(line);
    fclose(file);
    pthread_exit(results);
}

int main(int argc, char *argv[])
{
    // Command line argument for file path
    char *filename = FILE_NAME;
    if (argc > 1) {
        filename = argv[1];
    }
    
    // Count the number of lines in the file
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error opening file");
        return 1;
    }
    
    char buffer[MAX_LINE_SIZE];
    while (fgets(buffer, MAX_LINE_SIZE, file) != NULL) {
        num_lines++;
    }
    fclose(file);
    
    printf("Total lines in file: %d\n", num_lines);
    
    // Create thread data structures
    pthread_t threads[NUM_THREADS];
    ThreadData thread_data[NUM_THREADS];
    
    // Divide lines among threads
    int lines_per_thread = num_lines / NUM_THREADS;
    int remaining_lines = num_lines % NUM_THREADS;
    
    int start = 0;
    for (int i = 0; i < NUM_THREADS; i++) {
		thread_data[i].start = start;
		thread_data[i].filename = filename;  // ADD THIS LINE
		
		// Add one extra line to some threads if division is not even
		int extra = (i < remaining_lines) ? 1 : 0;
		thread_data[i].end = start + lines_per_thread + extra;
        
        start = thread_data[i].end;
        
        // Create thread
        int rc = pthread_create(&threads[i], NULL, process_lines, &thread_data[i]);
        if (rc) {
            fprintf(stderr, "ERROR: return code from pthread_create() is %d\n", rc);
            return 1;
        }
    }
    
    // Allocate array for all results
    int *all_results = malloc(num_lines * sizeof(int));
    
    // Wait for threads to complete and collect results
    for (int i = 0; i < NUM_THREADS; i++) {
        int *thread_results;
        pthread_join(threads[i], (void **)&thread_results);
        
        // Copy the thread's results to the main results array
        int range_start = thread_data[i].start;
        for (int j = 0; j < (thread_data[i].end - thread_data[i].start); j++) {
            all_results[range_start + j] = thread_results[j];
        }
        
        free(thread_results);
    }
    
    for (int i = 0; i < num_lines; i++) {
        printf("%d: %d\n", i, all_results[i]);
    }
    
    free(all_results);
    return 0;
}
