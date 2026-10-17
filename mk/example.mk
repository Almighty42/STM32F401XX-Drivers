# mk/example.mk: shared build rules for example firmware.
#
# An example's Makefile sets:
#   TARGET   output name, e.g. button_blink
#   SOURCES  the example's own .c files
#   DRIVERS  driver folders under drivers/, e.g. GPIO_driver
# and then includes this file.

ROOT := $(abspath $(dir $(lastword $(MAKEFILE_LIST)))/..)

CC  := arm-none-eabi-gcc
CPU := -mcpu=cortex-m4 -mthumb -mfloat-abi=soft

DRIVER_SRC := $(foreach d,$(DRIVERS),$(wildcard $(ROOT)/drivers/$(d)/*.c))
DRIVER_INC := $(addprefix -I$(ROOT)/drivers/,$(DRIVERS))
ALL_SRC    := $(SOURCES) $(ROOT)/generic/startup.c $(DRIVER_SRC)

CFLAGS  := $(CPU) -std=gnu11 -Wall -Wextra -O0 -g3 \
           -I$(ROOT)/generic $(DRIVER_INC) $(EXTRA_CFLAGS)
LDFLAGS := -nostartfiles -nodefaultlibs $(CPU) \
           -T $(ROOT)/generic/stm32_ls.ld -Wl,-Map=build/$(TARGET).map

OBJECTS := $(addprefix build/,$(notdir $(ALL_SRC:.c=.o)))
vpath %.c $(sort $(dir $(ALL_SRC)))

.PHONY: all clean flash load
all: build/$(TARGET).elf

build/$(TARGET).elf: $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $(OBJECTS)

build/%.o: %.c | build
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

build:
	mkdir -p build

clean:
	rm -rf build

flash: build/$(TARGET).elf
	openocd -f interface/stlink.cfg -f target/stm32f4x.cfg \
		-c "program $< verify reset exit"

load:
	-pkill -x openocd || true
	openocd -f interface/stlink.cfg -f target/stm32f4x.cfg

-include $(OBJECTS:.o=.d)
