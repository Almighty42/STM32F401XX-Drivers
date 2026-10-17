/**
 * @file stm32f401xx_SYSTIME_driver.c
 * @brief Implementation of the systime service. The API is documented in
 * the header.
 */

#include "stm32f401xx_SYSTIME_driver.h"
#include <stddef.h>

SYSTIME_status_t SYSTIME_init(uint32_t core_hz)
{
	(void)core_hz;
	return SYSTIME_OK;
}

uint32_t SYSTIME_ms(void)
{
	return 0U;
}

uint32_t SYSTIME_since(uint32_t start_ms)
{
	(void)start_ms;
	return 0U;
}

bool SYSTIME_elapsed(uint32_t start_ms, uint32_t timeout_ms)
{
	(void)start_ms;
	(void)timeout_ms;
	return false;
}

void SYSTIME_delay_ms(uint32_t ms)
{
	(void)ms;
}

void SYSTIME_timer_start(SYSTIME_timer_t* t, uint32_t period_ms)
{
	(void)t;
	(void)period_ms;
}

bool SYSTIME_timer_expired(SYSTIME_timer_t* t)
{
	(void)t;
	return false;
}
