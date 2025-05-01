#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_LINE_SIZE 1024
#define FILE_NAME "wiki_dump.txt"
#define NUM_THREADS 20
#define MAX_LINES 1000000

// Returns the max ASCII value per line
int collect_ascii_values(char *line) {
	int max_value = 0;
    	for (int i = 0; line[i] != '\0'; i++) {
        	if ((unsigned char)line[i] > max_value) {
            	max_value = (unsigned char)line[i];
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

    	// Open file and read all lines into memory
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

	    size_t len = 0;
	    ssize_t read;
	    char *line = NULL;
	    int num_lines = 0;

	    while ((read = getline(&line, &len, file)) != -1 && num_lines < MAX_LINES) {
	        if (read > 0 && line[read - 1] == '\n') {
	            line[--read] = '\0';
	        }

	        lines[num_lines] = strdup(line);
	        if (lines[num_lines] == NULL) {
	            perror("Memory allocation failed");
	            fclose(file);
	            return 1;
	        }
	        num_lines++;
	    }

	    free(line);
	    fclose(file);
	    printf("Read %d lines from file\n", num_lines);

	    // Allocate space for results
	    int *results = malloc(num_lines * sizeof(int));
	    if (results == NULL) {
	        perror("Memory allocation failed");
	        return 1;
	    }

	    // Parallel processing of lines
	    omp_set_num_threads(NUM_THREADS);
	    #pragma omp parallel for schedule(static)
	    for (int i = 0; i < num_lines; i++) {
	        results[i] = collect_ascii_values(lines[i]);
	    }

	    // Print results
	    for (int i = 0; i < num_lines; i++) {
	        printf("%d: %d\n", i, results[i]);
	        free(lines[i]);
	    }

	    // Clean up
    	    free(lines);
	    free(results);

	    // End timing
	    double end_time = omp_get_wtime();
	    printf("Execution time: %.2f seconds\n", end_time - start_time);

    	return 0;
}
