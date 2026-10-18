#include "stm32f401xx.h"
#include "stm32f401xx_SYSTICK_driver.h"
#include "stm32f401xx_SYSTICK_regs.h"
#include "unity.h"
#include <string.h>

SYSTICK_TypeDef fake_systick;

static int g_calls;
static int g_other_calls;
static void* g_ctx;

static void cb(void* ctx)
{
	g_calls++;
	g_ctx = ctx;
}

static void other_cb(void* ctx)
{
	(void)ctx;
	g_other_calls++;
}

void setUp(void)
{
	memset(&fake_systick, 0, sizeof fake_systick);
	g_calls = 0;
	g_other_calls = 0;
	g_ctx = NULL;
	SYSTICK_set_callback(
	    NULL, NULL); /* the driver keeps the callback in a static */
}

void tearDown(void)
{
}

/* Init with an invalid rate and prove no register changed. */
static void expect_rejected(uint32_t core_hz, uint32_t tick_hz,
                            SYSTICK_status_t err)
{
	SYSTICK_TypeDef before;

	fake_systick.CTRL = SYSTICK_CTRL_ENABLE;
	fake_systick.LOAD = 0x1234U;
	fake_systick.VAL = 0x55U;
	memcpy(&before, &fake_systick, sizeof before);

	TEST_ASSERT_EQUAL(err, SYSTICK_init(core_hz, tick_hz));
	TEST_ASSERT_EQUAL_MEMORY(&before, &fake_systick, sizeof before);
}

/* ---- SYSTICK_init: the reload value ----------------------------------- */

static void test_init_84mhz_at_1khz(void)
{
	TEST_ASSERT_EQUAL(SYSTICK_OK, SYSTICK_init(84000000U, 1000U));
	TEST_ASSERT_EQUAL_HEX32(83999U, fake_systick.LOAD);
	TEST_ASSERT_EQUAL_HEX32(0U, fake_systick.VAL);
	TEST_ASSERT_EQUAL_HEX32(
	    0x7U, fake_systick.CTRL); /* ENABLE|TICKINT|CLKSOURCE */
}

static void test_init_16mhz_at_1khz(void)
{
	TEST_ASSERT_EQUAL(SYSTICK_OK, SYSTICK_init(16000000U, 1000U));
	TEST_ASSERT_EQUAL_HEX32(15999U, fake_systick.LOAD);
}

static void test_init_truncates_when_rate_does_not_divide(void)
{
	/* 16 MHz / 3 kHz = 5333.33: the driver uses 5333 counts. */
	TEST_ASSERT_EQUAL(SYSTICK_OK, SYSTICK_init(16000000U, 3000U));
	TEST_ASSERT_EQUAL_HEX32(5332U, fake_systick.LOAD);
}

static void test_init_two_counts_per_tick_is_the_fastest(void)
{
	TEST_ASSERT_EQUAL(SYSTICK_OK, SYSTICK_init(84000000U, 42000000U));
	TEST_ASSERT_EQUAL_HEX32(1U, fake_systick.LOAD);
}

static void test_init_24bit_limit_is_exact(void)
{
	/* 2^24 counts per tick is the largest period: reload 0xFFFFFF. */
	TEST_ASSERT_EQUAL(SYSTICK_OK, SYSTICK_init(16777216U, 1U));
	TEST_ASSERT_EQUAL_HEX32(0x00FFFFFFU, fake_systick.LOAD);
}

static void test_init_slowest_tick_at_84mhz_is_6hz(void)
{
	TEST_ASSERT_EQUAL(SYSTICK_OK, SYSTICK_init(84000000U, 6U));
	TEST_ASSERT_EQUAL_HEX32(13999999U, fake_systick.LOAD);
}

/* ---- SYSTICK_init: rejected rates leave the registers alone ------------ */

static void test_init_rejects_zero_rates(void)
{
	expect_rejected(0U, 1000U, SYSTICK_ERR_INVALID_RATE);
	expect_rejected(84000000U, 0U, SYSTICK_ERR_INVALID_RATE);
	expect_rejected(0U, 0U, SYSTICK_ERR_INVALID_RATE);
}

static void test_init_rejects_one_count_per_tick(void)
{
	expect_rejected(84000000U, 84000000U, SYSTICK_ERR_INVALID_RATE);
}

static void test_init_rejects_tick_faster_than_core(void)
{
	expect_rejected(84000000U, 100000000U, SYSTICK_ERR_INVALID_RATE);
}

static void test_init_rejects_reload_wider_than_24_bits(void)
{
	expect_rejected(16777217U, 1U, SYSTICK_ERR_RELOAD_RANGE); /* 2^24 + 1 */
	expect_rejected(84000000U, 5U, SYSTICK_ERR_RELOAD_RANGE);
}

/* ---- SYSTICK_init: register effects ------------------------------------ */

static void test_init_clears_the_counter(void)
{
	fake_systick.VAL = 0x1234U;
	TEST_ASSERT_EQUAL(SYSTICK_OK, SYSTICK_init(84000000U, 1000U));
	TEST_ASSERT_EQUAL_HEX32(0U, fake_systick.VAL);
}

static void test_init_again_replaces_the_previous_rate(void)
{
	TEST_ASSERT_EQUAL(SYSTICK_OK, SYSTICK_init(84000000U, 1000U));
	TEST_ASSERT_EQUAL(SYSTICK_OK, SYSTICK_init(16000000U, 1000U));
	TEST_ASSERT_EQUAL_HEX32(15999U, fake_systick.LOAD);
	TEST_ASSERT_BITS_HIGH(SYSTICK_CTRL_RUN, fake_systick.CTRL);
}

static void test_init_writes_only_the_24bit_load_field(void)
{
	fake_systick.LOAD = 0xFF000000U; /* reserved bits set */
	TEST_ASSERT_EQUAL(SYSTICK_OK, SYSTICK_init(84000000U, 1000U));
	TEST_ASSERT_EQUAL_UINT32(
	    83999U, FIELD_GET(SYSTICK_LOAD_MASK, fake_systick.LOAD));
}

static void test_init_does_not_touch_the_callback(void)
{
	SYSTICK_set_callback(cb, NULL);
	SYSTICK_init(84000000U, 1000U);
	SYSTICK_irq_handler();
	TEST_ASSERT_EQUAL(1, g_calls);
}

/* ---- SYSTICK_deinit ----------------------------------------------------- */

static void test_deinit_stops_counter_and_interrupt(void)
{
	SYSTICK_init(84000000U, 1000U);
	SYSTICK_deinit();
	TEST_ASSERT_BITS_LOW(SYSTICK_CTRL_RUN, fake_systick.CTRL);
}

static void test_deinit_leaves_other_ctrl_bits_alone(void)
{
	fake_systick.CTRL = SYSTICK_CTRL_RUN | SYSTICK_CTRL_COUNTFLAG;
	SYSTICK_deinit();
	TEST_ASSERT_EQUAL_HEX32(SYSTICK_CTRL_COUNTFLAG, fake_systick.CTRL);
}

static void test_deinit_without_init_is_safe(void)
{
	SYSTICK_deinit();
	TEST_ASSERT_BITS_LOW(SYSTICK_CTRL_RUN, fake_systick.CTRL);
}

static void test_deinit_keeps_the_callback_registered(void)
{
	SYSTICK_set_callback(cb, NULL);
	SYSTICK_init(84000000U, 1000U);
	SYSTICK_deinit();
	SYSTICK_irq_handler(); /* one late interrupt is possible */
	TEST_ASSERT_EQUAL(1, g_calls);
}

/* ---- Callback and handler ---------------------------------------------- */

static void test_handler_calls_callback_with_its_context(void)
{
	int token = 0;

	SYSTICK_set_callback(cb, &token);
	SYSTICK_irq_handler();
	TEST_ASSERT_EQUAL(1, g_calls);
	TEST_ASSERT_EQUAL_PTR(&token, g_ctx);
}

static void test_handler_passes_a_null_context(void)
{
	g_ctx = (void*)&g_calls; /* anything but NULL */
	SYSTICK_set_callback(cb, NULL);
	SYSTICK_irq_handler();
	TEST_ASSERT_NULL(g_ctx);
}

static void test_handler_calls_once_per_tick(void)
{
	SYSTICK_set_callback(cb, NULL);
	for (int i = 0; i < 5; i++) {
		SYSTICK_irq_handler();
	}
	TEST_ASSERT_EQUAL(5, g_calls);
}

static void test_handler_without_callback_does_nothing(void)
{
	SYSTICK_irq_handler(); /* must not crash */
	TEST_ASSERT_EQUAL(0, g_calls);
}

static void test_set_callback_null_removes_it(void)
{
	SYSTICK_set_callback(cb, NULL);
	SYSTICK_set_callback(NULL, NULL);
	SYSTICK_irq_handler();
	TEST_ASSERT_EQUAL(0, g_calls);
}

static void test_set_callback_replaces_the_previous_one(void)
{
	SYSTICK_set_callback(cb, NULL);
	SYSTICK_set_callback(other_cb, NULL);
	SYSTICK_irq_handler();
	TEST_ASSERT_EQUAL(0, g_calls);
	TEST_ASSERT_EQUAL(1, g_other_calls);
}

int main(void)
{
	UNITY_BEGIN();
	RUN_TEST(test_init_84mhz_at_1khz);
	RUN_TEST(test_init_16mhz_at_1khz);
	RUN_TEST(test_init_truncates_when_rate_does_not_divide);
	RUN_TEST(test_init_two_counts_per_tick_is_the_fastest);
	RUN_TEST(test_init_24bit_limit_is_exact);
	RUN_TEST(test_init_slowest_tick_at_84mhz_is_6hz);
	RUN_TEST(test_init_rejects_zero_rates);
	RUN_TEST(test_init_rejects_one_count_per_tick);
	RUN_TEST(test_init_rejects_tick_faster_than_core);
	RUN_TEST(test_init_rejects_reload_wider_than_24_bits);
	RUN_TEST(test_init_clears_the_counter);
	RUN_TEST(test_init_again_replaces_the_previous_rate);
	RUN_TEST(test_init_writes_only_the_24bit_load_field);
	RUN_TEST(test_init_does_not_touch_the_callback);
	RUN_TEST(test_deinit_stops_counter_and_interrupt);
	RUN_TEST(test_deinit_leaves_other_ctrl_bits_alone);
	RUN_TEST(test_deinit_without_init_is_safe);
	RUN_TEST(test_deinit_keeps_the_callback_registered);
	RUN_TEST(test_handler_calls_callback_with_its_context);
	RUN_TEST(test_handler_passes_a_null_context);
	RUN_TEST(test_handler_calls_once_per_tick);
	RUN_TEST(test_handler_without_callback_does_nothing);
	RUN_TEST(test_set_callback_null_removes_it);
	RUN_TEST(test_set_callback_replaces_the_previous_one);
	return UNITY_END();
}
