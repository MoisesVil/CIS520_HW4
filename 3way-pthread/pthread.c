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
