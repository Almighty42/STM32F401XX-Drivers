#include "stm32f401xx.h"
#include "stm32f401xx_GPIO_driver.h"

static void fail(void)
{
	for (;;) {
	}
}

static void on_button(uint16_t pin, void* ctx)
{
	(void)pin;
	(void)ctx;
	GPIO_toggle(GPIOA, GPIO_PIN_5);
}

void EXTI15_10_IRQHandler(void)
{
	GPIO_exti_irq_handler(10, 15);
}

int main(void)
{
	const GPIO_config_t led = {.pins = GPIO_PIN_5,
	                           .mode = GPIO_MODE_OUTPUT,
	                           .speed = GPIO_SPEED_FAST};
	const GPIO_config_t btn = {.pins = GPIO_PIN_13,
	                           .mode = GPIO_MODE_INPUT,
	                           .pull = GPIO_PULL_UP,
	                           .trigger = GPIO_TRIG_FALLING};

	if (GPIO_init(GPIOA, &led) != GPIO_OK) {
		fail();
	}
	if (GPIO_init(GPIOC, &btn) != GPIO_OK) {
		fail();
	}
	GPIO_exti_set_callback(GPIO_PIN_13, on_button, NULL);

	NVIC->ISER[IRQ_NO_EXTI15_10 / 32U] = 1U << (IRQ_NO_EXTI15_10 % 32U);

	for (;;) {
	}
}
