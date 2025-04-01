# Makefile for the Compiler project

CC = gcc
CFLAGS = -Wall -Wextra -I../MLibc/include -Iinclude
LDFLAGS = -L../MLibc/src -lmlibc

SRC = src/main.c src/lexer.c src/parser.c src/codegen.c
OBJ = $(SRC:.c=.o)

TARGET = compiler

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) -o $(TARGET) $(OBJ) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)