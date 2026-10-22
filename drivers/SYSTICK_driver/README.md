# SysTick driver

Periodic tick driver for the Cortex-M4 SysTick timer on the STM32F401xD/xE,
tested on the NUCLEO-F401RE.

- `SYSTICK_init` starts a tick at a chosen rate from the core clock (HCLK).
- The caller passes the clock frequency, so the driver does not depend on the
  RCC driver. Use `RCC_get_hclk_hz()` to get it.
- The rate is checked and the reload value is calculated before any register is
  touched, so a call that fails validation leaves SysTick unchanged.
- An optional per-tick callback with a context pointer, called from the
  SysTick exception.
- `SYSTICK_deinit` stops the counter and its interrupt.
- Every call that can fail returns a `SYSTICK_status_t`.

## Driver usage example

A 1 kHz tick on the 84 MHz clock from the RCC example, toggling a counter in
the callback.

```c
static volatile uint32_t ticks;

static void on_tick(void *ctx)
{
    (void)ctx;
    ticks++;
}

SYSTICK_set_callback(on_tick, NULL);

if (SYSTICK_init(RCC_get_hclk_hz(), 1000U) != SYSTICK_OK) {
    /* SysTick is unchanged */
}

void SysTick_Handler(void) { SYSTICK_irq_handler(); }
```

Register the callback before `SYSTICK_init`, so the first tick is not lost.
Call `SYSTICK_init` again after every clock change.

## API

| Function | Purpose |
|---|---|
| `SYSTICK_init(core_hz, tick_hz)` | Start the tick at `tick_hz`. |
| `SYSTICK_deinit()` | Stop the counter and its interrupt. |
| `SYSTICK_set_callback(cb, ctx)` | Register the per-tick callback, or remove it with NULL. |
| `SYSTICK_irq_handler()` | Handle one tick. Call from `SysTick_Handler`. |

### Status codes

`SYSTICK_OK` is 0. Every error is non-zero.

| Code | Meaning |
|---|---|
| `SYSTICK_ERR_INVALID_RATE` | `core_hz` or `tick_hz` is zero, or `tick_hz` is above `core_hz / 2`. |
| `SYSTICK_ERR_RELOAD_RANGE` | The needed reload value does not fit in 24 bits. |

## Limits

The reload value is `core_hz / tick_hz - 1`, written to a 24-bit register.

| Parameter | Range |
|---|---|
| Counts per tick | 2 to 16777216 |
| Reload value | 1 to `SYSTICK_RELOAD_MAX` (0x00FFFFFF) |
| Fastest tick | `core_hz / 2` |
| Slowest tick at 84 MHz | about 6 Hz |
| Slowest tick at 16 MHz | 1 Hz |

Typical values:

| Core clock | Tick rate | Reload value | Exact |
|---|---|---|---|
| 16 MHz (reset clock) | 1 kHz | 15999 | yes |
| 84 MHz | 1 kHz | 83999 | yes |
| 84 MHz | 6 Hz | 13999999 | yes |

The division is an integer division. The real rate is
`core_hz / (core_hz / tick_hz)`, which equals `tick_hz` only when `tick_hz`
divides `core_hz`.

## Examples

Each example is a complete program for the NUCLEO-F401RE. Build and flash
commands are in `mk/example.mk` and each example's Makefile.

| Example | Demonstrates | Check with |
|---|---|---|
| [`systick/tick_blink`](../../examples/systick/tick_blink) | SysTick alone on the 16 MHz reset clock, callback toggles
