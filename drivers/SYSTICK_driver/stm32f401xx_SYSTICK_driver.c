/**
 * @file stm32f401xx_SYSTICK_driver.c
 * @brief Implementation of the SysTick driver. The API is documented in
 * the header.
 */

#include "stm32f401xx_SYSTICK_driver.h"
#include <stddef.h>

SYSTICK_status_t SYSTICK_init(uint32_t core_hz, uint32_t tick_hz)
{
	(void)core_hz;
	(void)tick_hz;
	return SYSTICK_OK;
}

void SYSTICK_deinit(void)
{
}

void SYSTICK_set_callback(SYSTICK_callback_t cb, void* ctx)
{
	(void)cb;
	(void)ctx;
}

void SYSTICK_irq_handler(void)
{
}
