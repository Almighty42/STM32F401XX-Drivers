#include "stm32f401xx.h"
#include "stm32f401xx_GPIO_driver.h"
#include "stm32f401xx_RCC_driver.h"
#include "stm32f401xx_SYSTICK_driver.h"
#include "stm32f401xx_SYSTIME_driver.h"
#include <stdbool.h>

#define BLINK_MS 500U
#define DEBOUNCE_MS 50U

static void fail(void)
{
	for (;;) {
	}
}

void SysTick_Handler(void)
{
	SYSTICK_irq_handler();
}

int main(void)
{
	const RCC_clock_config_t clk = {
	    .sysclk = RCC_SYSCLK_PLL,
	    .pll = {.src = RCC_PLL_SRC_HSI, .m = 8, .n = 168, .p = 4, .q = 7},
	    .ahb_div = RCC_AHB_DIV_1,
	    .apb1_div = RCC_APB_DIV_2,
	    .apb2_div = RCC_APB_DIV_1,
	};
	const GPIO_config_t led = {.pins = GPIO_PIN_5,
	                           .mode = GPIO_MODE_OUTPUT,
	                           .speed = GPIO_SPEED_FAST};
	const GPIO_config_t btn = {.pins = GPIO_PIN_13,
	                           .mode = GPIO_MODE_INPUT,
	                           .pull =
	                               GPIO_PULL_UP}; /* polled, no interrupt */

	if (RCC_clock_init(&clk) != RCC_OK) {
		fail();
	}
	if (SYSTIME_init(RCC_get_hclk_hz()) != SYSTIME_OK) {
		fail();
	}
	if (GPIO_init(GPIOA, &led) != GPIO_OK ||
	    GPIO_init(GPIOC, &btn) != GPIO_OK) {
		fail();
	}

	SYSTIME_timer_t blink;
	bool blinking = true;
	bool last_pressed = false;
	uint32_t last_change = SYSTIME_ms();

	SYSTIME_timer_start(&blink, BLINK_MS);

	for (;;) {
		/* The user button is active low. */
		const bool pressed = (GPIO_read_pins(GPIOC, GPIO_PIN_13) == 0U);

		/* Accept a change only if the last one was at least 50 ms ago. */
		if (pressed != last_pressed &&
		    SYSTIME_elapsed(last_change, DEBOUNCE_MS)) {
			last_change = SYSTIME_ms();
			last_pressed = pressed;
			if (pressed) {
				blinking = !blinking;
				if (blinking) {
					/* Restart: an old timer would fire a burst to catch up. */
					SYSTIME_timer_start(&blink, BLINK_MS);
				}
				else {
					GPIO_reset(GPIOA, GPIO_PIN_5);
				}
			}
		}

		if (blinking && SYSTIME_timer_expired(&blink)) {
			GPIO_toggle(GPIOA, GPIO_PIN_5);
		}
	}
}
