CC = gcc
CFLAGS = -Wall -O3 -pthread
TARGET = pthread_max_ascii

all: $(TARGET)

$(TARGET): pthread.c
	$(CC) $(CFLAGS) -o $(TARGET) pthread.c

clean:
	rm -f $(TARGET) *.o