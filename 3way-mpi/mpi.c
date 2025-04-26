#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>
#include <time.h>

#define MAX_LINES 1000000
#define MAX_LINE_SIZE 1024
#define FILE_NAME "wiki_dump.txt"

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

int main(int argc, char *argv[])
{
    int rank, size;
    double start_time, end_time;
    char *filename = FILE_NAME;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    start_time = MPI_Wtime();

    if (argc > 1) {
        filename = argv[1];
    }

    char **local_lines = NULL;
    int *results = NULL;
    int num_lines = 0;
    int local_count = 0;

    if (rank == 0) {
        // Rank 0 reads the whole file
        FILE *file = fopen(filename, "r");
        if (!file) {
            perror("Error opening file");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }

        local_lines = malloc(MAX_LINES * sizeof(char *));
        if (!local_lines) {
            perror("Could not malloc");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }

        char buffer[MAX_LINE_SIZE];
        // Modified file reading loop with safety checks
        while (1) {
            if (fgets(buffer, MAX_LINE_SIZE, file) == NULL) {
                // Handle EOF or error
                break;
            }
            
            size_t len = strlen(buffer);
            if (len > 0 && buffer[len - 1] == '\n') {
                buffer[len - 1] = '\0';
            }
            
            local_lines[num_lines] = strdup(buffer);
            if (local_lines[num_lines] == NULL) {
                perror("strdup failed");
                MPI_Abort(MPI_COMM_WORLD, 1);
            }
            
            num_lines++;
            if (num_lines >= MAX_LINES) {
                fprintf(stderr, "Warning: Reached maximum line limit (%d)\n", MAX_LINES);
                break;
            }
        }
        fclose(file);

        // Check if we hit the maximum lines limit
        if (num_lines >= MAX_LINES) {
            fprintf(stderr, "File has more than maximum allowed lines (%d)\n", MAX_LINES);
            fprintf(stderr, "Processing will continue with first %d lines only\n", MAX_LINES);
        }

        printf("Total lines: %d\n", num_lines);
    }
    // Moved outside: all ranks must call this
    MPI_Bcast(&num_lines, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Barrier(MPI_COMM_WORLD);

    if (rank == 0) {
        // Split and send lines
        int base = num_lines / size;
        int rem = num_lines % size;

        for (int i = 1; i < size; i++) {
            int count = base + (i < rem ? 1 : 0);
            MPI_Send(&count, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
            for (int j = 0; j < count; j++) {
                int line_idx = i * base + (i < rem ? i : rem) + j;
                int len = strlen(local_lines[line_idx]) + 1;
                MPI_Send(&len, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
                MPI_Send(local_lines[line_idx], len, MPI_CHAR, i, 0, MPI_COMM_WORLD);
            }
        }

        // Local work for rank 0
        local_count = base + (0 < rem ? 1 : 0);  // <-- no 'int' here, just assignment
        results = malloc(local_count * sizeof(int));
        if (!results) {
            perror("Could not malloc");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }

        for (int i = 0; i < local_count; i++) {
            results[i] = collect_ascii_values(local_lines[i]);
        }
    } else {
        // Receive line count from root
        int received_count;
        MPI_Recv(&received_count, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        local_count = received_count;  // Save it here

        local_lines = malloc(local_count * sizeof(char *));
        if (!local_lines) {
            perror("Could not malloc");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }

        results = malloc(local_count * sizeof(int));
        if (!results) {
            perror("Could not malloc");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }

        for (int i = 0; i < local_count; i++) {
            int len;
            MPI_Recv(&len, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            local_lines[i] = malloc(len);
            if (!local_lines[i]) {
                perror("Could not malloc line buffer");
                MPI_Abort(MPI_COMM_WORLD, 1);
            }
            MPI_Recv(local_lines[i], len, MPI_CHAR, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }

        for (int i = 0; i < local_count; i++) {
            results[i] = collect_ascii_values(local_lines[i]);
        }
    }

    // Gather results at root
    int *recvcounts = NULL, *displs = NULL, *all_results = NULL;

    if (rank == 0) {
        recvcounts = malloc(size * sizeof(int));
        displs = malloc(size * sizeof(int));
        if (!recvcounts || !displs) {
            perror("Could not malloc recvcounts or displs");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }

        int base = num_lines / size;
        int rem = num_lines % size;

        for (int i = 0; i < size; i++) {
            recvcounts[i] = base + (i < rem ? 1 : 0);
            displs[i] = (i == 0) ? 0 : displs[i-1] + recvcounts[i-1];
        }

        all_results = malloc(num_lines * sizeof(int));
        if (!all_results) {
            perror("Could not malloc all_results");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
    }

    MPI_Gatherv(results, local_count, MPI_INT,
                all_results, recvcounts, displs, MPI_INT,
                0, MPI_COMM_WORLD);

    if (rank == 0) {
        printf("Sample of results (first 10 lines):\n");
        for (int i = 0; i < 10 && i < num_lines; i++) {
            printf("%d: %d\n", i, all_results[i]);
        }
        end_time = MPI_Wtime();
        printf("Execution time: %.2f seconds\n", end_time - start_time);

        free(all_results);
        free(recvcounts);
        free(displs);

        for (int i = 0; i < num_lines; i++) {
            free(local_lines[i]);
        }
        free(local_lines);
    } else {
        for (int i = 0; i < local_count; i++) {
            free(local_lines[i]);
        }
        free(local_lines);
    }

    free(results);

    MPI_Finalize();
    return 0;
}
