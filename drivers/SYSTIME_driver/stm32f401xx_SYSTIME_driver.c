/**
 * @file stm32f401xx_SYSTIME_driver.c
 * @brief Implementation of the systime service. The API is documented in
 * the header.
 */

#include "stm32f401xx_SYSTIME_driver.h"
#include "stm32f401xx_SYSTICK_driver.h"
#include <stddef.h>

/* What to do while waiting for the next tick. On the target: sleep until an
 * interrupt. In a host test, define SYSTIME_IDLE_HOOK as the name of a function
 * that advances the fake time, or SYSTIME_delay_ms would wait forever. */
#ifdef SYSTIME_IDLE_HOOK
void SYSTIME_IDLE_HOOK(void);
#define SYSTIME_IDLE() SYSTIME_IDLE_HOOK()
#elif defined(__arm__)
#define SYSTIME_IDLE() __asm volatile("wfi")
#else
#define SYSTIME_IDLE() ((void)0)
#endif

/* ---- Private data ---------------------------------------------------- */

static volatile uint32_t systime_ticks; /* written only by the tick callback */

/* ---- Private helpers ------------------------------------------------- */

/** Tick callback: one millisecond has passed. Runs in interrupt context. */
static void systime_on_tick(void* ctx)
{
	(void)ctx;
	systime_ticks = systime_ticks + 1U; /* wraps after 2^32 ms, by design */
}

/** True once the absolute time @p deadline has been reached. Valid while the
 * deadline is within 2^31 ms of now, in the past or the future. */
static bool systime_reached(uint32_t deadline)
{
	return (int32_t)(SYSTIME_ms() - deadline) >= 0;
}

/* ---- Public API ------------------------------------------------------ */

SYSTIME_status_t SYSTIME_init(uint32_t core_hz)
{
	SYSTICK_set_callback(systime_on_tick, NULL); /* before the first tick */
	if (SYSTICK_init(core_hz, SYSTIME_TICK_HZ) != SYSTICK_OK) {
		return SYSTIME_ERR_TICK_RATE;
	}

	return SYSTIME_OK;
}

uint32_t SYSTIME_ms(void)
{
	return systime_ticks;
}

uint32_t SYSTIME_since(uint32_t start_ms)
{
	return (uint32_t)(SYSTIME_ms() - start_ms);
}

bool SYSTIME_elapsed(uint32_t start_ms, uint32_t timeout_ms)
{
	return SYSTIME_since(start_ms) >= timeout_ms;
}

void SYSTIME_delay_ms(uint32_t ms)
{
	const uint32_t start = SYSTIME_ms();
	/* One extra tick: starting mid-tick would otherwise cut the delay short. */
	const uint32_t wait = (ms == UINT32_MAX) ? ms : ms + 1U;

	while (!SYSTIME_elapsed(start, wait)) {
		SYSTIME_IDLE();
	}
}

void SYSTIME_timer_start(SYSTIME_timer_t* t, uint32_t period_ms)
{
	if (t == NULL) {

		return;
	}
	t->period = period_ms;
	t->next = SYSTIME_ms() + period_ms;
}

bool SYSTIME_timer_expired(SYSTIME_timer_t* t)
{
	if (t == NULL || !systime_reached(t->next)) {
		return false;
	}
	t->next += t->period;
	return true;
}

#ifdef SYSTIME_TESTING
/** Test seam: preset the millisecond counter (wrap-around tests). */
void systime_test_set_ms(uint32_t ms)
{
	systime_ticks = ms;
}
#endif
