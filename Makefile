CC      = arm-none-eabi-gcc
MACH    = cortex-m4

# Output directory for object files and final artifacts
BUILD_DIR = build

# FreeRTOS source root
FREERTOS_DIR = ThirdParty/FreeRTOS

# Include paths
INCLUDES = \
    -I . \
    -I $(FREERTOS_DIR)/include \
    -I $(FREERTOS_DIR)/portable/GCC/ARM_CM4F

# Compiler flags
#   -mfpu / -mfloat-abi: enable the hardware FPU on the Cortex-M4F
#   -O0: no optimisation (easiest to debug)
CFLAGS = \
    -c \
    -mcpu=$(MACH) \
    -mthumb \
    -mfpu=fpv4-sp-d16 \
    -mfloat-abi=hard \
    -std=gnu11 \
    -Wall \
    -O0 \
    -g \
    $(INCLUDES)

# Linker flags
LDFLAGS = \
    -nostdlib \
    -T stm32_ls.ld \
    -mfpu=fpv4-sp-d16 \
    -mfloat-abi=hard \
    -Wl,-Map=$(BUILD_DIR)/final.map

# Libraries must follow the object files on the command line.
# Point to the newlib hard-float variant that matches our -mfloat-abi=hard build.
LDLIBS = -L /usr/lib/arm-none-eabi/newlib/thumb/v7e-m+fp/hard -lc -lgcc

# Application sources
APP_SRCS = main.c led.c stm32_startup.c

# FreeRTOS kernel sources
FREERTOS_SRCS = \
    $(FREERTOS_DIR)/tasks.c \
    $(FREERTOS_DIR)/list.c \
    $(FREERTOS_DIR)/queue.c \
    $(FREERTOS_DIR)/portable/GCC/ARM_CM4F/port.c \
    $(FREERTOS_DIR)/portable/MemMang/heap_4.c

ALL_SRCS = $(APP_SRCS) $(FREERTOS_SRCS)

# Place all object files in BUILD_DIR, preserving base name
OBJS = $(addprefix $(BUILD_DIR)/, $(notdir $(ALL_SRCS:.c=.o)))

# Targets
.PHONY: all clean load

all: $(BUILD_DIR)/final.elf

# Link
$(BUILD_DIR)/final.elf: $(OBJS) | $(BUILD_DIR)
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

# Create build directory if it doesn't exist
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Compile rules — one per source location
$(BUILD_DIR)/%.o: %.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $@ $<

$(BUILD_DIR)/%.o: $(FREERTOS_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $@ $<

$(BUILD_DIR)/%.o: $(FREERTOS_DIR)/portable/GCC/ARM_CM4F/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $@ $<

$(BUILD_DIR)/%.o: $(FREERTOS_DIR)/portable/MemMang/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -rf $(BUILD_DIR)

load:
	openocd \
	    -f /usr/share/openocd/scripts/interface/stlink-v2-1.cfg \
	    -f /usr/share/openocd/scripts/target/stm32f4x.cfg
