CC   = arm-none-eabi-gcc
ARC  = cortex-m4

CFLAGS  = -mcpu=$(ARC) -mthumb -mfloat-abi=soft -std=gnu11 -Wall -O0 -I.
LDFLAGS = -nostartfiles -nodefaultlibs -mcpu=$(ARC) -mthumb -mfloat-abi=soft -T stm32_ls.ld -Wl,-Map=blink.map
DBFLAGS = -g3

TARGET  = blink.elf
SOURCES = main.c startup.c stm32f401xx_GPIO_driver.c
OBJECTS = $(SOURCES:.c=.o)

$(TARGET): $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $(OBJECTS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.s
	$(CC) $(CFLAGS) -c $< -o $@

build: $(TARGET)
	rm -f $(OBJECTS) *.map

build_debug:
	$(MAKE) CFLAGS="$(CFLAGS) $(DBFLAGS)" \
	        LDFLAGS="$(LDFLAGS) $(DBFLAGS)" \
		$(TARGET)

clean:
	rm -f $(TARGET) $(OBJECTS) *.map

load:
	-pkill -x openocd || true && openocd -f interface/stlink.cfg -f target/stm32f4x.cfg

flash:
	openocd -f interface/stlink.cfg -f target/stm32f4x.cfg \
		-c "program $(TARGET) verify reset exit"
