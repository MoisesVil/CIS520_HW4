CC = gcc
CFLAGS = -Wall -O3 -fopenmp
TARGET = openmp_max_ascii

all: $(TARGET)

$(TARGET): openmp.c
	$(CC) $(CFLAGS) -o $(TARGET) openmp.c

clean:
	rm -f $(TARGET) *.o
