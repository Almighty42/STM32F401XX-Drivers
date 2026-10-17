# basic_blink

Blinks the on-board LED (LD2, PA5) forever. The smallest possible use of the GPIO
driver: `GPIO_init` to make PA5 a push-pull output, then `GPIO_toggle` in a loop
with a busy-wait delay.

```sh
make          # build -> build/basic_blink.elf
make flash    # program through ST-LINK and OpenOCD
```

The delay is a plain software loop, so the blink rate is approximate. Change
`BLINK_DELAY_LOOPS` in `main.c` to adjust it.
