ROOT  := ..
UNITY := $(ROOT)/third_party/Unity/src
CC    ?= gcc

CFLAGS := -std=gnu11 -Wall -Wextra -g -O0 -fsanitize=address,undefined \
          -Ifake -I$(UNITY) -I$(ROOT)/generic -I$(ROOT)/drivers/GPIO_driver

GPIO_SRC := $(ROOT)/drivers/GPIO_driver/stm32f401xx_GPIO_driver.c
TESTS    := build/test_gpio

.PHONY: test clean
test: $(TESTS)
	@for t in $(TESTS); do echo "== $$t"; ./$$t || exit 1; done

build/test_gpio: gpio/test_gpio.c $(GPIO_SRC) $(UNITY)/unity.c | build
	$(CC) $(CFLAGS) $^ -o $@

build:
	mkdir -p build

clean:
	rm -rf build
