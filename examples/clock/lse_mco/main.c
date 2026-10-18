#include "stm32f401xx.h"
#include "stm32f401xx_GPIO_driver.h"
#include "stm32f401xx_RCC_driver.h"
#include "stm32f401xx_SYSTICK_driver.h"
#include "stm32f401xx_SYSTIME_driver.h"

#define LSE_TIMEOUT_MS 3000U

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
	const GPIO_config_t mco = {.pins = GPIO_PIN_8,
	                           .mode = GPIO_MODE_AF,
	                           .af = GPIO_AF0_SYS,
	                           .speed = GPIO_SPEED_HIGH};

	if (GPIO_init(GPIOA, &led) != GPIO_OK) {
		fail();
	}
	if (RCC_clock_init(&clk) != RCC_OK) {
		fail();
	}
	if (SYSTIME_init(RCC_get_hclk_hz()) != SYSTIME_OK) {
		fail();
	}
	if (GPIO_init(GPIOA, &mco) != GPIO_OK) {
		fail();
	}

	/* The crystal needs time: poll with our own timeout. */
	SYSTIME_timer_t timeout;
	SYSTIME_timer_start(&timeout, LSE_TIMEOUT_MS);
	RCC_lse_start(false);
	while (!RCC_lse_ready()) {
		if (SYSTIME_timer_expired(&timeout)) {
			fail(); /* no crystal, or the solder bridges are open */
		}
	}

	if (RCC_mco1_config(RCC_MCO1_LSE, RCC_MCO_DIV_1) != RCC_OK) {
		fail();
	}

	for (;;) {
		GPIO_toggle(GPIOA, GPIO_PIN_5);
		SYSTIME_delay_ms(500U);
	}
}
