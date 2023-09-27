# Compiler and compiler flags
CC = gcc
CFLAGS = -Wall -pthread

# Source files and object files
SRCS = main.c
OBJS = $(SRCS:.c=.o)

# Header file dependencies
DEPS = unbounded_queue.h newsItem.h bounded_queue.h

# Executable name
EXECUTABLE = ex3.out

# Default target
all: $(EXECUTABLE)

# Rule to build the executable
$(EXECUTABLE): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@

# Rule to build object files from source files
%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c $< -o $@

# Clean target to remove object files and the executable
clean:
	rm -f $(OBJS) $(EXECUTABLE)
