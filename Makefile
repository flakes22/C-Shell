# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -g -Iinclude

# Source files and objects
SRCDIR = src
SOURCES = $(SRCDIR)/main.c $(SRCDIR)/activities.c $(SRCDIR)/executor.c $(SRCDIR)/global.c \
          $(SRCDIR)/hop.c $(SRCDIR)/jobs.c $(SRCDIR)/log.c $(SRCDIR)/ping.c \
          $(SRCDIR)/reveal.c $(SRCDIR)/sequential.c $(SRCDIR)/tokenizer.c \
          $(SRCDIR)/signals.c $(SRCDIR)/fg_bg.c

OBJECTS = $(SOURCES:.c=.o)

# Target executable
TARGET = shell.out

# Default rule
all: $(TARGET)

# Link objects into final executable
$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^

# Compile .c into .o
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean build files
clean:
	rm -f $(OBJECTS) $(TARGET)

# Phony targets
.PHONY: all clean