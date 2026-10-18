# test/Makefile: host unit tests (Unity + fake registers)
#
# To add a test: copy one block under "Test registry", change the names.

ROOT  := ..
UNITY := $(ROOT)/third_party/Unity/src
BUILD := build
CC    ?= gcc

CFLAGS := -std=gnu11 -Wall -Wextra -g -O0 -fsanitize=address,undefined \
          -Ifake -I$(UNITY) -I$(ROOT)/generic

# Any header change rebuilds every test (small repo: simple and safe).
HEADERS := $(wildcard fake/*.h $(ROOT)/generic/*.h $(ROOT)/drivers/*/*.h)

.DEFAULT_GOAL := test
.DELETE_ON_ERROR:

# ---- Test registry ---------------------------------------------------------
# Per test, three things: its name in TESTS, the test file plus the driver
# source under test, and the include path (and extra defines) for that driver.

TESTS :=

TESTS += gpio
$(BUILD)/test_gpio: gpio/test_gpio.c \
                    $(ROOT)/drivers/GPIO_driver/stm32f401xx_GPIO_driver.c
$(BUILD)/test_gpio: CFLAGS += -I$(ROOT)/drivers/GPIO_driver

TESTS += rcc
$(BUILD)/test_rcc: rcc/test_rcc.c \
                   $(ROOT)/drivers/RCC_driver/stm32f401xx_RCC_driver.c
$(BUILD)/test_rcc: CFLAGS += -I$(ROOT)/drivers/RCC_driver \
                             -DREG_POLL_HOOK=fake_hw_step

# ---- Rules (no need to edit below when adding a test) -----------------------

TEST_BINS = $(addprefix $(BUILD)/test_,$(TESTS))

.PHONY: test clean
test: $(TEST_BINS)
	@for t in $(TEST_BINS); do echo "== $$t"; ./$$t || exit 1; done

# make run-rcc: build and run one test only
.PHONY: run-%
run-%: $(BUILD)/test_%
	@./$<

# Shared part of every test build: Unity and the headers. The test file and
# driver sources come from the registry above; $(filter %.c,$^) picks them up.
$(BUILD)/test_%: $(UNITY)/unity.c $(HEADERS) | $(BUILD)
	$(CC) $(CFLAGS) $(filter %.c,$^) -o $@

$(BUILD):
	mkdir -p $(BUILD)

clean:
	rm -rf $(BUILD)
