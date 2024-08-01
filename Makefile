# Makefile for fiberpool

FF_DIR = ./FiberFlow

# Compiler
CC := clang

TARGET := aarch64-none-elf

BUILD_DIR := build

# Compiler flags
CFLAGS = \
        -Wall \
		-Wextra \
		-target $(TARGET) \
		-ffreestanding \
		-g \
		-O0 \
		-mstrict-align \
		-I$(FF_DIR) \

# Source files
SRCS := FiberPool.c

STATIC_LIB := libfiberpool.a

# Default rule
all: $(STATIC_LIB)

# Object files (generated from source files)
OBJS := build/fiberpool.o \
        build/fiberflow.o \

# Rule to compile source files into object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/fiberpool.o: ./FiberPool.c Makefile
	$(CC) -c $(CFLAGS) $< -o $@

$(BUILD_DIR)/fiberflow.o: ./FiberFlow/FiberFlow.c Makefile
	$(CC) -c $(CFLAGS) $< -o $@

# Rule to link the target executable
$(STATIC_LIB): $(OBJS)
	ar rcs $@ $(OBJS)

# Phony targets (not files)
.PHONY: all clean