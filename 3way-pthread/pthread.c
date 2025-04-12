#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NUM_THREADS 4 		// Number of pthreads being used
#define MAX_LINE_SIZE 4096 	// Cap of total number of chars per line
#define MAX_NUM_LINES 1000000 	// Cap of total number of lines in the file

char **num_lines;	// Stores each line from the file
int *results;		// Stores the max ASCII value for each line
int total_lines = 0;	// The number of lines read within the file


// Initialize functions
void load_file(const char *file);
void allocate_in_memory();
void *get_max_ascii_values(void *arg);
void start_threads();
void print();

// Runs the program
int main()
{
	load_file("/homes/dan/625/wiki_dump.txt");
	allocate_in_memory();
	start_threads();
	print();

	for (int i = 0; i < total_lines; i++)
	{
		free(num_lines[i]);
	}
	free(num_lines);
	free(results);
	return 0;
}

// Loads the file into the memory and reads for each line
void load_file(const char *file)
{
	// Attempt to open and read the file
	FILE *f = fopen(file, "r");
	if (!f)
	{
		printf("Error opening file");
		fclose(f);
		exit(1);
	}

	// Allocate memory for the array holding references to each line
	num_lines = malloc(MAX_NUM_LINES * sizeof(char *));
	if (!num_lines)
	{
		printf("Could not allocate memory");
		fclose(f);
		exit(1);
	}

	// Read each line and store it
	char buffer[MAX_LINE_SIZE];
	while (fgets(buffer, MAX_LINE_SIZE, f))
	{
		num_lines[total_lines] = strdup(buffer);	// Allocate memory and copy the line being read
		if (!num_lines[total_lines])
		{
			printf("Error");
			fclose(f);
			exit(1);
		}
		total_lines++;
	}
	fclose(f);
}

// Allocate memory into results (holds max ASCII values for each line)
void allocate_in_memory()
{
	results = malloc(MAX_NUM_LINES * sizeof(int));
	if (!results)
	{
		printf("Could not allocate memory for results");
		exit(1);
	}
}

// Will assign each thread to get the max ASCII value for each line
void *get_max_ascii_values(void *arg)
{
	long thread = (long)arg;
	int num_lines_per_thread = total_lines / NUM_THREADS;
	int start_pos = thread * num_lines_per_thread;
	int end_pos = (thread == NUM_THREADS - 1) ? total_lines : start_pos + num_lines_per_thread;

	// Loop for each line
	for (int i = start_pos; i < end_pos; i++)
	{
		int max_ascii_value = 0;
		// Get the max ASCII value within the line
		for (int j = 0; num_lines[i][j] != '\0'; j++)
		{
			if ((unsigned char)num_lines[i][j] > max_ascii_value)
			{
				max_ascii_value = (unsigned char)num_lines[i][j];
			}
		}
		// Store the max ASCII value in the line
		results[i] = max_ascii_value;
	}
	pthread_exit(NULL);
}

// Creates and joins the pthreads from NUM_THREADS
void start_threads()
{
	pthread_t threads[NUM_THREADS];
	// Create the threads
	for (int i = 0; i < NUM_THREADS; i++)
	{
		pthread_create(&threads[i], NULL, get_max_ascii_values, (void *)(long)i);
	}
	// Join the threads
	for (int i = 0; i < NUM_THREADS; i++)
	{
		pthread_join(threads[i], NULL);
	}
}

// Will print the results for each line with its associated max ASCII value
void print()
{
	for (int i = 0; i < 100; i++)
	{
		printf("%d: %d\n", i, results[i]);
	}
}
