#include "stm32f401xx.h"
#include "stm32f401xx_GPIO_driver.h"
#include "stm32f401xx_RCC_driver.h"
#include "stm32f401xx_SYSTICK_driver.h"
#include "stm32f401xx_SYSTIME_driver.h"

/* Steady LED = something failed. (Before the LED is configured this does
 * nothing, which is harmless.) */
static void fail(void)
{
	GPIO_set(GPIOA, GPIO_PIN_5);
	for (;;) {
	}
}

void SysTick_Handler(void)
{
	SYSTICK_irq_handler();
}

int main(void)
{
	const GPIO_config_t led = {.pins = GPIO_PIN_5,
	                           .mode = GPIO_MODE_OUTPUT,
	                           .speed = GPIO_SPEED_FAST};

	const RCC_clock_config_t clk = {
	    .sysclk = RCC_SYSCLK_PLL,
	    .pll = {.src = RCC_PLL_SRC_HSI, .m = 8, .n = 168, .p = 4, .q = 7},
	    .ahb_div = RCC_AHB_DIV_1,
	    .apb1_div = RCC_APB_DIV_2,
	    .apb2_div = RCC_APB_DIV_1};

	if (GPIO_init(GPIOA, &led) != GPIO_OK) {
		fail();
	}

	if (RCC_clock_init(&clk) != RCC_OK) {
		fail();
	}

	/* The clock changed, so the tick must be set up after it. */
	if (SYSTIME_init(RCC_get_hclk_hz()) != SYSTIME_OK) {
		fail(); /* without this check, the delay below would hang silently */
	}

	for (;;) {
		GPIO_toggle(GPIOA, GPIO_PIN_5);
		/* delay(500) waits 501 ticks: the full period is 1.002 s. */
		SYSTIME_delay_ms(500U);
	}
}
