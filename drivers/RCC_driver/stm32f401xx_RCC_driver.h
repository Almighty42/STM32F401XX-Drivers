#ifndef STM32F401RE_RCC_DRIVER_H
#define STM32F401RE_RCC_DRIVER_H

#include "stm32f401xx.h"

 // Get PCLK value
uint32_t RCC_get_pclk_1_val(void);
uint32_t RCC_get_pclk_2_val(void);
uint32_t RCC_get_pll_output_clk(void);

#endif
