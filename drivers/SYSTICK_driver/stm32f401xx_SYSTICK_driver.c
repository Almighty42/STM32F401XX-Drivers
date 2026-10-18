/**
 * @file stm32f401xx_SYSTICK_driver.c
 * @brief Implementation of the SysTick driver. The API is documented in
 * the header.
 */

#include "stm32f401xx_SYSTICK_driver.h"
#include "reg_utils.h"
#include "stm32f401xx.h"
#include "stm32f401xx_SYSTICK_regs.h"
#include <stddef.h>

/* A reload of 0 is not useful (the counter would stop), so at least 2
 * counts per tick are required. */
#define SYSTICK_MIN_TICKS 2U

_Static_assert(SYSTICK_RELOAD_MAX == SYSTICK_LOAD_MASK,
               "public limit must match the LOAD register width");

/* ---- Private data ---------------------------------------------------- */

typedef struct
{
	volatile SYSTICK_callback_t cb;
	void* volatile ctx;
} systick_slot_t;

static systick_slot_t systick_slot;

/* ---- Private: pure calculation (no register access) ------------------ */

/** Reload value for @p tick_hz at @p core_hz, or the reason it is invalid. */
static SYSTICK_status_t systick_calc_reload(uint32_t core_hz, uint32_t tick_hz,
                                            uint32_t* reload)
{
	if (core_hz == 0U || tick_hz == 0U) {
		return SYSTICK_ERR_INVALID_RATE;
	}

	/* Integer division: the real rate is core_hz / ticks, exact only when
	 * tick_hz divides core_hz. */
	const uint32_t ticks = core_hz / tick_hz;
	if (ticks < SYSTICK_MIN_TICKS) {
		return SYSTICK_ERR_INVALID_RATE;
	}
	if (ticks - 1U > SYSTICK_RELOAD_MAX) {
		return SYSTICK_ERR_RELOAD_RANGE;
	}

	*reload = ticks - 1U;
	return SYSTICK_OK;
}

SYSTICK_status_t SYSTICK_init(uint32_t core_hz, uint32_t tick_hz)
{
	uint32_t reload = 0U;
	const SYSTICK_status_t status =
	    systick_calc_reload(core_hz, tick_hz, &reload);

	if (status != SYSTICK_OK) {
		return status;
	}

	reg_modify(&SYSTICK->CTRL, SYSTICK_CTRL_RUN, 0U); /* stop first */
	reg_set_field(&SYSTICK->LOAD, SYSTICK_LOAD_MASK, reload);
	SYSTICK->VAL = 0U; /* any write clears the counter and COUNTFLAG */
	reg_modify(&SYSTICK->CTRL, SYSTICK_CTRL_RUN, SYSTICK_CTRL_RUN);
	return SYSTICK_OK;
}

void SYSTICK_deinit(void)
{
	reg_modify(&SYSTICK->CTRL, SYSTICK_CTRL_RUN, 0U);
}

void SYSTICK_set_callback(SYSTICK_callback_t cb, void* ctx)
{
	systick_slot.cb = NULL;
	systick_slot.ctx = ctx;
	systick_slot.cb = cb;
}

void SYSTICK_irq_handler(void)
{
	const SYSTICK_callback_t cb = systick_slot.cb;
	void* const ctx = systick_slot.ctx;
	if (cb != NULL) {
		cb(ctx);
	}
}
