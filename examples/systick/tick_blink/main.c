#include "stm32f401xx.h"
#include "stm32f401xx_GPIO_driver.h"
#include "stm32f401xx_SYSTICK_driver.h"

#define CORE_HZ 16000000U /* HSI: the clock after reset */
#define TICK_HZ 1000U
#define TOGGLE_EVERY_TICKS 500U

static void fail()
{
	for (;;) {
	}
}

/* Runs in interrupt context once per tick. */
static void on_tick(void* ctx)
{
	uint32_t* count = (uint32_t*)ctx;
	if (++*count >= TOGGLE_EVERY_TICKS) {
		*count = 0U;
		GPIO_toggle(GPIOA, GPIO_PIN_5);
	}
}

void SysTick_Handler(void)
{
	SYSTICK_irq_handler();
}

int main(void)
{
	static uint32_t count;
	const GPIO_config_t led = {.pins = GPIO_PIN_5,
	                           .mode = GPIO_MODE_OUTPUT,
	                           .speed = GPIO_SPEED_FAST};

	if (GPIO_init(GPIOA, &led) != GPIO_OK) {
		fail();
	}

	SYSTICK_set_callback(on_tick, &count);
	if (SYSTICK_init(CORE_HZ, TICK_HZ) != SYSTICK_OK) {
		fail();
	}

	for (;;) {
		__asm volatile("wfi");
	}
}
