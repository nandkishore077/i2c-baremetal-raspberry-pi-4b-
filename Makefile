CROSS   = aarch64-none-elf-
CC      = $(CROSS)gcc
LD      = $(CROSS)ld
OBJCOPY = $(CROSS)objcopy
SIZE    = $(CROSS)size

CFLAGS  = -ffreestanding -nostdlib -nostartfiles -O2 -Wall -Wextra -mcpu=cortex-a72
LDFLAGS = -nostdlib -T link.ld

TARGET  = kernel8
OBJECTS = start.o i2c-driver.o i2c-app.o

all: $(TARGET).img

# Startup assembly
start.o: start.S
	$(CC) -ffreestanding -nostdlib -mcpu=cortex-a72 -c $< -o $@

# I2C driver implementation
i2c-driver.o: i2c-driver.c i2c-driver.h
	$(CC) $(CFLAGS) -c $< -o $@

# I2C test application
i2c-app.o: i2c-app.c i2c-driver.h
	$(CC) $(CFLAGS) -c $< -o $@

# Link
$(TARGET).elf: $(OBJECTS) link.ld
	$(LD) $(LDFLAGS) $(OBJECTS) -o $@

# Convert to raw binary
$(TARGET).img: $(TARGET).elf
	$(OBJCOPY) -O binary $< $@
	@echo ""
	@echo "=== Build Complete ==="
	@$(SIZE) $(TARGET).elf
	@echo "Output: $(TARGET).img ($$(wc -c < $(TARGET).img) bytes)"

clean:
	rm -f $(OBJECTS) $(TARGET).elf $(TARGET).img

.PHONY: all clean
