#include "stm32f401xx.h"
#include "stm32f401xx_SYSTICK_driver.h"
#include "stm32f401xx_SYSTIME_driver.h"
#include "unity.h"
#include <string.h>

SYSTICK_TypeDef fake_systick;

void systime_test_set_ms(uint32_t ms); /* test seam, see the driver */

static unsigned g_idle_calls;

/* SYSTIME_delay_ms calls this while it waits: one fake tick passes. */
void fake_idle(void)
{
	g_idle_calls++;
	SYSTICK_irq_handler();
}

static void tick(unsigned n)
{
	while (n-- > 0U) {
		SYSTICK_irq_handler();
	}
}

void setUp(void)
{
	memset(&fake_systick, 0, sizeof fake_systick);
	g_idle_calls = 0U;
	SYSTICK_set_callback(NULL, NULL);
	(void)SYSTIME_init(84000000U);
	systime_test_set_ms(0U);
}

void tearDown(void)
{
}

/* ---- Init and the tick counter ---------------------------------------- */

static void test_init_programs_a_1khz_tick(void)
{
	TEST_ASSERT_EQUAL(SYSTIME_OK, SYSTIME_init(84000000U));
	TEST_ASSERT_EQUAL_HEX32(83999U, fake_systick.LOAD);
	TEST_ASSERT_EQUAL_HEX32(0x7U, fake_systick.CTRL);
}

static void test_init_fails_when_systick_refuses_the_rate(void)
{
	TEST_ASSERT_EQUAL(SYSTIME_ERR_TICK_RATE, SYSTIME_init(1000U));
	TEST_ASSERT_EQUAL(SYSTIME_ERR_TICK_RATE, SYSTIME_init(0U));
}

static void test_init_again_keeps_the_counter(void)
{
	tick(5);
	TEST_ASSERT_EQUAL(SYSTIME_OK, SYSTIME_init(16000000U));
	TEST_ASSERT_EQUAL_UINT32(5U, SYSTIME_ms());
}

static void test_ms_starts_at_zero(void)
{
	TEST_ASSERT_EQUAL_UINT32(0U, SYSTIME_ms());
}

static void test_each_tick_adds_one_millisecond(void)
{
	tick(1000);
	TEST_ASSERT_EQUAL_UINT32(1000U, SYSTIME_ms());
}

static void test_ms_wraps_at_2_pow_32(void)
{
	systime_test_set_ms(0xFFFFFFFFU);
	tick(1);
	TEST_ASSERT_EQUAL_UINT32(0U, SYSTIME_ms());
}

/* ---- since / elapsed -------------------------------------------------- */

static void test_since_counts_the_ticks_after_start(void)
{
	const uint32_t start = SYSTIME_ms();

	tick(5);
	TEST_ASSERT_EQUAL_UINT32(5U, SYSTIME_since(start));
}

static void test_since_is_correct_across_the_wrap(void)
{
	systime_test_set_ms(0xFFFFFFFDU);
	const uint32_t start = SYSTIME_ms();

	tick(5); /* now 2 */
	TEST_ASSERT_EQUAL_UINT32(2U, SYSTIME_ms());
	TEST_ASSERT_EQUAL_UINT32(5U, SYSTIME_since(start));
}

static void test_elapsed_flips_exactly_at_the_timeout(void)
{
	const uint32_t start = SYSTIME_ms();

	tick(9);
	TEST_ASSERT_FALSE(SYSTIME_elapsed(start, 10U));
	tick(1);
	TEST_ASSERT_TRUE(SYSTIME_elapsed(start, 10U));
}

static void test_elapsed_with_zero_timeout_is_immediately_true(void)
{
	TEST_ASSERT_TRUE(SYSTIME_elapsed(SYSTIME_ms(), 0U));
}

static void test_elapsed_works_across_the_wrap(void)
{
	systime_test_set_ms(0xFFFFFFF8U);
	const uint32_t start = SYSTIME_ms();

	tick(9);
	TEST_ASSERT_FALSE(SYSTIME_elapsed(start, 10U));
	tick(1);
	TEST_ASSERT_TRUE(SYSTIME_elapsed(start, 10U));
}

/* ---- delay ------------------------------------------------------------ */

static void test_delay_waits_ms_plus_one_ticks(void)
{
	SYSTIME_delay_ms(10U);
	TEST_ASSERT_EQUAL_UINT(11U, g_idle_calls);
	TEST_ASSERT_EQUAL_UINT32(11U, SYSTIME_ms());
}

static void test_delay_zero_waits_one_tick(void)
{
	SYSTIME_delay_ms(0U);
	TEST_ASSERT_EQUAL_UINT(1U, g_idle_calls);
}

static void test_delay_across_the_wrap(void)
{
	systime_test_set_ms(0xFFFFFFF0U);
	SYSTIME_delay_ms(32U);
	TEST_ASSERT_EQUAL_UINT(33U, g_idle_calls);
	TEST_ASSERT_EQUAL_UINT32(0x11U, SYSTIME_ms());
}

/* ---- periodic timers -------------------------------------------------- */

static void test_timer_start_schedules_the_first_expiry(void)
{
	SYSTIME_timer_t t;

	tick(7);
	SYSTIME_timer_start(&t, 10U);
	TEST_ASSERT_EQUAL_UINT32(10U, t.period);
	TEST_ASSERT_EQUAL_UINT32(17U, t.next);
}

static void test_timer_expires_once_per_period(void)
{
	SYSTIME_timer_t t;

	SYSTIME_timer_start(&t, 10U);
	tick(9);
	TEST_ASSERT_FALSE(SYSTIME_timer_expired(&t));
	tick(1);
	TEST_ASSERT_TRUE(SYSTIME_timer_expired(&t));
	TEST_ASSERT_FALSE(SYSTIME_timer_expired(&t)); /* same instant */
	tick(10);
	TEST_ASSERT_TRUE(SYSTIME_timer_expired(&t));
}

static void test_timer_does_not_drift_when_polled_late(void)
{
	SYSTIME_timer_t t;

	SYSTIME_timer_start(&t, 10U);
	tick(13); /* polled 3 ms late */
	TEST_ASSERT_TRUE(SYSTIME_timer_expired(&t));
	TEST_ASSERT_EQUAL_UINT32(20U,
	                         t.next); /* still on the original schedule */
	TEST_ASSERT_FALSE(SYSTIME_timer_expired(&t));
	tick(7);
	TEST_ASSERT_TRUE(SYSTIME_timer_expired(&t)); /* exactly at 20 */
}

static void test_timer_catches_up_after_a_long_gap(void)
{
	SYSTIME_timer_t t;

	SYSTIME_timer_start(&t, 10U);
	tick(35); /* three periods missed: 10, 20, 30 */
	TEST_ASSERT_TRUE(SYSTIME_timer_expired(&t));
	TEST_ASSERT_TRUE(SYSTIME_timer_expired(&t));
	TEST_ASSERT_TRUE(SYSTIME_timer_expired(&t));
	TEST_ASSERT_FALSE(SYSTIME_timer_expired(&t));
}

static void test_timer_with_period_zero_fires_on_every_call(void)
{
	SYSTIME_timer_t t;

	SYSTIME_timer_start(&t, 0U);
	TEST_ASSERT_TRUE(SYSTIME_timer_expired(&t));
	TEST_ASSERT_TRUE(SYSTIME_timer_expired(&t));
}

static void test_timer_works_across_the_wrap(void)
{
	SYSTIME_timer_t t;

	systime_test_set_ms(0xFFFFFFF8U);
	SYSTIME_timer_start(&t, 10U); /* next = 2 after the wrap */
	TEST_ASSERT_EQUAL_UINT32(2U, t.next);
	tick(9);
	TEST_ASSERT_FALSE(SYSTIME_timer_expired(&t));
	tick(1);
	TEST_ASSERT_TRUE(SYSTIME_timer_expired(&t));
}

static void test_timer_null_pointers_are_ignored(void)
{
	SYSTIME_timer_start(NULL, 5U);
	TEST_ASSERT_FALSE(SYSTIME_timer_expired(NULL));
}

int main(void)
{
	UNITY_BEGIN();
	RUN_TEST(test_init_programs_a_1khz_tick);
	RUN_TEST(test_init_fails_when_systick_refuses_the_rate);
	RUN_TEST(test_init_again_keeps_the_counter);
	RUN_TEST(test_ms_starts_at_zero);
	RUN_TEST(test_each_tick_adds_one_millisecond);
	RUN_TEST(test_ms_wraps_at_2_pow_32);
	RUN_TEST(test_since_counts_the_ticks_after_start);
	RUN_TEST(test_since_is_correct_across_the_wrap);
	RUN_TEST(test_elapsed_flips_exactly_at_the_timeout);
	RUN_TEST(test_elapsed_with_zero_timeout_is_immediately_true);
	RUN_TEST(test_elapsed_works_across_the_wrap);
	RUN_TEST(test_delay_waits_ms_plus_one_ticks);
	RUN_TEST(test_delay_zero_waits_one_tick);
	RUN_TEST(test_delay_across_the_wrap);
	RUN_TEST(test_timer_start_schedules_the_first_expiry);
	RUN_TEST(test_timer_expires_once_per_period);
	RUN_TEST(test_timer_does_not_drift_when_polled_late);
	RUN_TEST(test_timer_catches_up_after_a_long_gap);
	RUN_TEST(test_timer_with_period_zero_fires_on_every_call);
	RUN_TEST(test_timer_works_across_the_wrap);
	RUN_TEST(test_timer_null_pointers_are_ignored);
	return UNITY_END();
}
