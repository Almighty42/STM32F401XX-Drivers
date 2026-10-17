#include "stm32f401xx_GPIO_driver.h"

#define BLINK_DELAY_LOOPS 400000U

static void fail(void)
{
	for (;;) {
	}
}

static void delay(volatile uint32_t loops)
{
	while (loops-- > 0U) {
	}
}

int main(void)
{
	const GPIO_config_t led = {.pins = GPIO_PIN_5,
	                           .mode = GPIO_MODE_OUTPUT,
	                           .speed = GPIO_SPEED_FAST};

	if (GPIO_init(GPIOA, &led) != GPIO_OK) {
		fail();
	}

	for (;;) {
		GPIO_toggle(GPIOA, GPIO_PIN_5);
		delay(BLINK_DELAY_LOOPS);
	}
}
