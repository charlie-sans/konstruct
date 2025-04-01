# Makefile for building the MLibc library

CC = tcc
CFLAGS = -Wall -Wextra -Iinclude -fPIC
AR = ar
ARFLAGS = rcs

# Source files
SRC = src/memory.c src/stdio.c src/string.c
OBJ = $(SRC:.c=.o)

# Output library
LIBRARY = libMLibc.a

# Default target
all: $(LIBRARY)

# Build the static library
$(LIBRARY): $(OBJ)
	$(AR) $(ARFLAGS) $@ $^

# Compile source files to object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up build artifacts
clean:
	rm -f $(OBJ) $(LIBRARY)