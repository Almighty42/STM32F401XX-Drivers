/**
 * @file stm32f401xx_RCC_driver.c
 * @brief Implementation of the RCC clock driver. The API is documented in
 * the header.
 */

#include "stm32f401xx_RCC_driver.h"
#include <stddef.h>

RCC_status_t RCC_clock_init(const RCC_clock_config_t* cfg)
{
	(void)cfg;
	return RCC_OK;
}

uint32_t RCC_get_sysclk_hz(void)
{
	return 0U;
}

uint32_t RCC_get_hclk_hz(void)
{
	return 0U;
}

uint32_t RCC_get_pclk1_hz(void)
{
	return 0U;
}

uint32_t RCC_get_pclk2_hz(void)
{
	return 0U;
}

RCC_status_t RCC_mco1_config(RCC_mco1_src_t src, RCC_mco_div_t div)
{
	(void)src;
	(void)div;
	return RCC_OK;
}
