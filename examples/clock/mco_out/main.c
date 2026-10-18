#include "stm32f401xx.h"
#include "stm32f401xx_GPIO_driver.h"
#include "stm32f401xx_RCC_driver.h"

static void fail(void)
{
	for (;;) {
	}
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
	const GPIO_config_t mco = {.pins = GPIO_PIN_8,
	                           .mode = GPIO_MODE_AF,
	                           .af = GPIO_AF0_SYS,
	                           .speed = GPIO_SPEED_HIGH};

	if (RCC_clock_init(&clk) != RCC_OK) {
		fail(); /* no signal on PA8 means the PLL setup failed */
	}
	if (GPIO_init(GPIOA, &mco) != GPIO_OK) {
		fail();
	}
	/* HSI / 5 = 3.2 MHz on PA8. */
	if (RCC_mco1_config(RCC_MCO1_HSI, RCC_MCO_DIV_5) != RCC_OK) {
		fail();
	}

	for (;;) {
	}
}
