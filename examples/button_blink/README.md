# button_blink

Toggles the on-board LED (LD2, PA5) each time the user button (B1, PC13) is pressed.
Uses the GPIO driver's EXTI support: the button is a falling-edge interrupt, and the
LED toggles from a callback registered with `GPIO_exti_set_callback`.


```sh
make          # build -> build/button_blink.elf
make flash    # program through ST-LINK and OpenOCD
```
