# GPIO Driver

A simple GPIO driver for the STM32F401xx series of microcontrollers, tested on the
STM32F401RE. This driver acts as a simple HAL-style API over the GPIO pins, allowing easier configuration and control.

## Driver APIs
- GPIO peripheral initialize / de-initialize functions
- Simple data read / write / toggle
- Lock GPIO configuration pin ( preventing it from being changed )
- IRQ configuration / ISR handling

## Also included:
- Configuration structures
- Handle structures
- Application states / events
- Return status codes

### Usage example

```c

GPIO_peri_clk_control(GPIOA, ENABLE);

// GPIO handle initialization ( LED )
GPIO_Handle_t gpio_led = {
 .p_GPIOx = GPIOA,
 .GPIO_Pin_Config.GPIO_Pin_Number = GPIO_PIN_NO_5,
 .GPIO_Pin_Config.GPIO_Pin_Mode = GPIO_MODE_OUT,
 .GPIO_Pin_Config.GPIO_Pin_Speed = GPIO_SPEED_FAST,
 .GPIO_Pin_Config.GPIO_Pin_OTP_Type = GPIO_OP_TYPE_PP,
 .GPIO_Pin_Config.GPIO_Pin_PuPd_Control = GPIO_NO_PUPD
};

GPIO_init(&gpio_led);

GPIO_peri_clk_control(GPIOC, ENABLE);

// GPIO handle initialization ( User Button )
GPIO_Handle_t gpio_btn = {
 .p_GPIOx = GPIOC,
 .GPIO_Pin_Config.GPIO_Pin_Number = GPIO_PIN_NO_13,
 .GPIO_Pin_Config.GPIO_Pin_Mode = GPIO_MODE_IT_FT,
 .GPIO_Pin_Config.GPIO_Pin_Speed = GPIO_SPEED_FAST,
 .GPIO_Pin_Config.GPIO_Pin_PuPd_Control = GPIO_PIN_PU};

GPIO_init(&gpio_btn);

GPIO_irq_priority_config(IRQ_NO_EXTI15_10, NVIC_IRQ_PRIO_15);
GPIO_irq_interrupt_config(IRQ_NO_EXTI15_10, ENABLE);

...

// ISR example
// Toggle LED state when EXTI15_10 is triggered by the user button (PC13)
void EXTI15_10_IRQHandler(void)
{
	GPIO_irq_handling(GPIO_PIN_NO_13);

	GPIO_toggle_output_pin(GPIOA, GPIO_PIN_NO_5);
}


```
