# GPIO driver

GPIO and EXTI driver for the STM32F401xD/xE, tested on the NUCLEO-F401RE.

- Pins are passed as bit masks (`GPIO_PIN_5`, `GPIO_PIN_2 | GPIO_PIN_3`), never as indices.
- `GPIO_init` configures one or more pins in a single call: mode, output type, speed,
  pull, alternate function and an optional EXTI trigger.
- `BSRR`-based set, reset, toggle and masked write, plus pin and port reads.
- EXTI with per-line callbacks: `GPIO_exti_irq_handler` finds the pending lines,
  clears them and calls your callback.
- Per-pin and per-port de-init, with the real reset values of the SWD/JTAG pins.
- Configuration lock (`GPIO_lock_pins`).
- Every call that can fail returns a `GPIO_status_t`.

## Example

```c
const GPIO_config_t led = {.pins = GPIO_PIN_5, .mode = GPIO_MODE_OUTPUT,
                           .speed = GPIO_SPEED_FAST};
const GPIO_config_t btn = {.pins = GPIO_PIN_13, .mode = GPIO_MODE_INPUT,
                           .pull = GPIO_PULL_UP, .trigger = GPIO_TRIG_FALLING};

GPIO_init(GPIOA, &led);
GPIO_init(GPIOC, &btn);
GPIO_exti_set_callback(GPIO_PIN_13, on_button, NULL);
NVIC->ISER[IRQ_NO_EXTI15_10 / 32U] = 1U << (IRQ_NO_EXTI15_10 % 32U);

void EXTI15_10_IRQHandler(void) { GPIO_exti_irq_handler(10, 15); }
```

Examples of how to use the driver are in `examples/button_blink`.

## How it works

### What `GPIO_init` does

Validation touches no register, so a call that fails validation leaves the
hardware unchanged. The steps from "Write otype and speed" to the EXTI step repeat
for every pin in the mask.

```mermaid
flowchart TD
    A(["GPIO_init(port, cfg)"]) --> B{"port and cfg non-NULL, port valid?"}
    B -- no --> E1["GPIO_ERR_NULL_PTR or GPIO_ERR_INVALID_PORT"]
    B -- yes --> C{"cfg fields valid?"}
    C -- no --> E2["GPIO_ERR_INVALID_*"]
    C -- yes --> D{"EXTI rules ok?"}
    D -- no --> E3["GPIO_ERR_INVALID_TRIGGER, EXTI_NOT_SINGLE or EXTI_LINE_BUSY"]
    D -- yes --> F["Enable port clock"]
    F --> G{"Any selected pin locked?"}
    G -- yes --> E4["GPIO_ERR_PIN_LOCKED"]
    G -- no --> H["Output or AF mode: write otype and speed"]
    H --> I["Write pull, then AF if in AF mode"]
    I --> J["Write MODER"]
    J --> K{"trigger is not NONE?"}
    K -- yes --> L["Arm EXTI: mask, route, set edges, clear pending, unmask"]
    K -- no --> M["Disarm the EXTI line if this port owns it"]
    L --> N(["GPIO_OK"])
    M --> N
```

### From button press to callback

The application enables the EXTI interrupt in the NVIC and calls
`GPIO_exti_irq_handler` from the ISR. The driver clears the pending flags first,
then calls the callbacks in ascending line order, so keep callbacks short.

```mermaid
sequenceDiagram
    participant Btn as Button PC13
    participant EXTI as EXTI line 13
    participant NVIC
    participant ISR as EXTI15_10_IRQHandler
    participant Drv as GPIO_exti_irq_handler
    participant CB as on_button callback

    Note over NVIC: Enabled and prioritised by the application
    Btn->>EXTI: falling edge
    EXTI->>EXTI: set pending flag
    EXTI->>NVIC: interrupt request
    NVIC->>ISR: run handler
    ISR->>Drv: GPIO_exti_irq_handler(10, 15)
    Drv->>EXTI: read pending and unmasked lines in 10 to 15
    Drv->>EXTI: clear all of them at once
    loop each pending line, lowest first
        Drv->>CB: pin mask and ctx
    end
    CB-->>Drv: return
    Drv-->>ISR: return
```

## Notes and limits

- The driver does not modify / configure the NVIC.
- A pin switched to output starts low (there is no initial-level option yet).
- `GPIO_init` is not safe to call from an ISR on the same port as other code.
- `GPIO_lock_pins` locks a whole port once: lock every pin you need in a single
  call. Only a reset removes the lock.
- `GPIO_port_deinit` resets every pin on the port, including other drivers' pins.

## Tests

Host unit tests (Unity, fake registers): `make -C test test`.
