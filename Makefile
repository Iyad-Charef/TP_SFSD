# Makefile for File Partitioning by Hashing
# SFSD - 2CP - ESI - 2024/2025

CC = gcc
CFLAGS = -Wall -Wextra -g
TARGET = partition_program
OBJS = main.o functions.o

# Default target
all: $(TARGET)

# Link all object files
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) -lm

# Compile main.c
main.o: main.c functions.h
	$(CC) $(CFLAGS) -c main.c
# Compile functions.c
functions.o: functions.c functions.h
	$(CC) $(CFLAGS) -c functions.c

# Clean up compiled files
clean:
	del /Q main.o functions.o $(TARGET) *.dat

# Clean only data files
cleandata:
	del /Q *.dat
# Run the program
run: $(TARGET)
	./$(TARGET)

.PHONY: all clean cleandata run
