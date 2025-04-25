CC = mpicc
CFLAGS = -O2 -Wall
TARGET = mpi_max_ascii
SRC = mpi.c

all: \$(TARGET)

\$(TARGET): \$(SRC)
	\$(CC) \$(CFLAGS) -o \$(TARGET) \$(SRC)

clean:
	rm -f \$(TARGET) results.txt
