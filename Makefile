# Makefile for fiberpool

FIBERPOOL_PATH ?= ./

# Compiler
CC := clang

TARGET := aarch64-none-elf

BUILD_DIR ?= build

# Compiler flags
CFLAGS = \
        -Wall \
		-Wextra \
		-target $(TARGET) \
		-ffreestanding \
		-g \
		-O0 \
		-mstrict-align \
		-I$(FIBERPOOL_PATH) \

STATIC_LIB := libfiberpool.a

# Default rule
all: $(BUILD_DIR)/$(STATIC_LIB)

# Object files (generated from source files)
OBJS := $(BUILD_DIR)/fiberpool.o \
		$(BUILD_DIR)/fiberflow.o \

$(BUILD_DIR)/fiberpool.o: $(FIBERPOOL_PATH)/FiberPool.c $(FIBERPOOL_PATH)/Makefile
	$(CC) -c $(CFLAGS) $< -o $@

$(BUILD_DIR)/fiberflow.o: $(FIBERPOOL_PATH)/FiberFlow/FiberFlow.c $(FIBERPOOL_PATH)/Makefile
	$(CC) -c $(CFLAGS) $< -o $@

# Rule to link the target executable
$(BUILD_DIR)/$(STATIC_LIB): $(OBJS)
	ar rcs $@ $(OBJS)

# Phony targets (not files)
.PHONY: all clean