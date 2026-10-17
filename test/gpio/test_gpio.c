#include "stm32f401xx_GPIO_driver.h"
#include "unity.h"
#include <string.h>

GPIO_TypeDef fake_gpio[6];
RCC_TypeDef fake_rcc;
EXTI_Type fake_exti;
SYSCFG_Type fake_syscfg;

static int g_calls;
static uint16_t g_pin;
static void* g_ctx;

static void cb(uint16_t pin, void* ctx)
{
	g_calls++;
	g_pin = pin;
	g_ctx = ctx;
}

void setUp(void)
{
	memset(fake_gpio, 0, sizeof fake_gpio);
	memset(&fake_rcc, 0, sizeof fake_rcc);
	memset(&fake_exti, 0, sizeof fake_exti);
	memset(&fake_syscfg, 0, sizeof fake_syscfg);
	g_calls = 0;
	g_pin = 0;
	g_ctx = NULL;
	for (uint32_t n = 0; n < 16; n++) {
		GPIO_exti_set_callback((uint16_t)(1U << n), NULL, NULL);
	}
}
void tearDown(void)
{
}

/* ---- Data access: the tests check the word written to BSRR ---- */

static void test_set_reset(void)
{
	GPIO_set(GPIOA, GPIO_PIN_5);
	TEST_ASSERT_EQUAL_HEX32(0x00000020U, fake_gpio[0].BSRR);
	GPIO_reset(GPIOA, GPIO_PIN_5);
	TEST_ASSERT_EQUAL_HEX32(0x00200000U, fake_gpio[0].BSRR);
}

static void test_toggle(void)
{
	fake_gpio[0].ODR = 0x0020U; /* pin 5 high, pin 6 low */
	GPIO_toggle(GPIOA, GPIO_PIN_5 | GPIO_PIN_6);
	TEST_ASSERT_EQUAL_HEX32(0x00200040U, fake_gpio[0].BSRR);
}

static void test_write_pins_ignores_bits_outside_mask(void)
{
	GPIO_write_pins(GPIOA, 0x000FU, 0xFF05U);
	TEST_ASSERT_EQUAL_HEX32(0x000A0005U, fake_gpio[0].BSRR);
}

static void test_read_pins(void)
{
	fake_gpio[0].IDR = 0x2020U;
	fake_gpio[0].ODR = 0x0021U;
	TEST_ASSERT_EQUAL_HEX16(0x2000U, GPIO_read_pins(GPIOA, GPIO_PIN_13));
	TEST_ASSERT_EQUAL_HEX16(
	    0x0001U, GPIO_read_output_pins(GPIOA, GPIO_PIN_0 | GPIO_PIN_1));
}

/* ---- GPIO_init ---- */

static void test_init_output_pin(void)
{
	GPIO_config_t cfg = {.pins = GPIO_PIN_5,
	                     .mode = GPIO_MODE_OUTPUT,
	                     .speed = GPIO_SPEED_FAST};
	TEST_ASSERT_EQUAL(GPIO_OK, GPIO_init(GPIOA, &cfg));
	TEST_ASSERT_EQUAL_HEX32(0x00000400U, fake_gpio[0].MODER);
	TEST_ASSERT_EQUAL_HEX32(0x00000800U, fake_gpio[0].OSPEEDR);
	TEST_ASSERT_BIT_HIGH(0, fake_rcc.AHB1ENR);
}

static void test_init_leaves_neighbours_untouched(void)
{
	fake_gpio[0].MODER = 0xFFFFFFFFU;
	GPIO_config_t cfg = {.pins = GPIO_PIN_5, .mode = GPIO_MODE_OUTPUT};
	TEST_ASSERT_EQUAL(GPIO_OK, GPIO_init(GPIOA, &cfg));
	TEST_ASSERT_EQUAL_HEX32(0xFFFFF7FFU, fake_gpio[0].MODER);
}

static void test_init_alternate_function_pin_low_and_high(void)
{
	GPIO_config_t lo = {
	    .pins = GPIO_PIN_2, .mode = GPIO_MODE_AF, .af = GPIO_AF7_USART2};
	GPIO_config_t hi = {
	    .pins = GPIO_PIN_10, .mode = GPIO_MODE_AF, .af = GPIO_AF7_USART1};
	TEST_ASSERT_EQUAL(GPIO_OK, GPIO_init(GPIOA, &lo));
	TEST_ASSERT_EQUAL_HEX32(0x00000700U,
	                        fake_gpio[0].AFRL); /* pin 2: bits 11:8 */
	TEST_ASSERT_EQUAL(GPIO_OK, GPIO_init(GPIOA, &hi));
	TEST_ASSERT_EQUAL_HEX32(0x00000700U,
	                        fake_gpio[0].AFRH); /* pin 10: bits 11:8 */
}

static void test_init_exti_falling_edge_on_pc13(void)
{
	GPIO_config_t btn = {.pins = GPIO_PIN_13,
	                     .mode = GPIO_MODE_INPUT,
	                     .pull = GPIO_PULL_UP,
	                     .trigger = GPIO_TRIG_FALLING};
	TEST_ASSERT_EQUAL(GPIO_OK, GPIO_init(GPIOC, &btn));
	TEST_ASSERT_EQUAL_HEX32(0x04000000U,
	                        fake_gpio[2].PUPDR); /* bits 27:26 = 01 */
	TEST_ASSERT_EQUAL_HEX32(0x00000020U,
	                        fake_syscfg.EXTICR[3]); /* line 13, port C */
	TEST_ASSERT_EQUAL_HEX32(0x00002000U, fake_exti.FTSR);
	TEST_ASSERT_EQUAL_HEX32(0x00000000U, fake_exti.RTSR);
	TEST_ASSERT_EQUAL_HEX32(0x00002000U, fake_exti.IMR);
	TEST_ASSERT_BIT_HIGH(14, fake_rcc.APB2ENR); /* SYSCFG clock */
}

static void test_init_exti_line_busy(void)
{
	GPIO_config_t pc13 = {.pins = GPIO_PIN_13,
	                      .mode = GPIO_MODE_INPUT,
	                      .trigger = GPIO_TRIG_FALLING};
	GPIO_config_t pa13 = pc13;
	TEST_ASSERT_EQUAL(GPIO_OK, GPIO_init(GPIOC, &pc13));
	TEST_ASSERT_EQUAL(GPIO_ERR_EXTI_LINE_BUSY, GPIO_init(GPIOA, &pa13));
	TEST_ASSERT_EQUAL_HEX32(0x00000020U,
	                        fake_syscfg.EXTICR[3]); /* still port C */
}

static void test_init_validation_errors_change_nothing(void)
{
	GPIO_config_t cfg = {.pins = GPIO_PIN_5, .mode = GPIO_MODE_OUTPUT};

	TEST_ASSERT_EQUAL(GPIO_ERR_NULL_PTR, GPIO_init(GPIOA, NULL));
	TEST_ASSERT_EQUAL(GPIO_ERR_INVALID_PORT,
	                  GPIO_init((GPIO_TypeDef*)0x1000, &cfg));

	cfg.pins = 0;
	TEST_ASSERT_EQUAL(GPIO_ERR_INVALID_PINS, GPIO_init(GPIOA, &cfg));

	cfg.pins = GPIO_PIN_5;
	cfg.mode = GPIO_MODE_AF;
	cfg.af = GPIO_AF_NONE;
	TEST_ASSERT_EQUAL(GPIO_ERR_INVALID_AF, GPIO_init(GPIOA, &cfg));

	cfg.mode = GPIO_MODE_OUTPUT;
	cfg.trigger = GPIO_TRIG_RISING;
	TEST_ASSERT_EQUAL(GPIO_ERR_INVALID_TRIGGER, GPIO_init(GPIOA, &cfg));

	cfg.mode = GPIO_MODE_INPUT;
	cfg.pins = GPIO_PIN_1 | GPIO_PIN_2;
	TEST_ASSERT_EQUAL(GPIO_ERR_EXTI_NOT_SINGLE, GPIO_init(GPIOA, &cfg));

	TEST_ASSERT_EQUAL_HEX32(0U, fake_gpio[0].MODER);
	TEST_ASSERT_EQUAL_HEX32(0U, fake_rcc.AHB1ENR); /* clock not touched */
}

static void test_init_refuses_locked_pin(void)
{
	fake_gpio[0].LCKR = (1U << 16) | GPIO_PIN_5;
	GPIO_config_t cfg = {.pins = GPIO_PIN_5, .mode = GPIO_MODE_OUTPUT};
	TEST_ASSERT_EQUAL(GPIO_ERR_PIN_LOCKED, GPIO_init(GPIOA, &cfg));
	TEST_ASSERT_EQUAL_HEX32(0U, fake_gpio[0].MODER);
}

/* ---- De-init ---- */

static void test_pin_deinit_restores_swd_pin_reset_state(void)
{
	fake_gpio[0].MODER = 0x55555555U;
	TEST_ASSERT_EQUAL(GPIO_OK, GPIO_pin_deinit(GPIOA, GPIO_PIN_14));
	TEST_ASSERT_EQUAL_HEX32(0x65555555U,
	                        fake_gpio[0].MODER); /* pin 14 -> AF (10) */
	TEST_ASSERT_EQUAL_HEX32(0x20000000U,
	                        fake_gpio[0].PUPDR); /* pin 14 -> pull-down */
	TEST_ASSERT_EQUAL_HEX32(0x40000000U,
	                        fake_gpio[0].BSRR); /* ODR bit cleared */
}

static void test_pin_deinit_plain_port_returns_to_input(void)
{
	fake_gpio[2].MODER = 0xFFFFFFFFU;
	TEST_ASSERT_EQUAL(GPIO_OK, GPIO_pin_deinit(GPIOC, GPIO_PIN_5));
	TEST_ASSERT_EQUAL_HEX32(0xFFFFF3FFU, fake_gpio[2].MODER);
}

static void test_pin_deinit_releases_exti_line(void)
{
	GPIO_config_t btn = {.pins = GPIO_PIN_13,
	                     .mode = GPIO_MODE_INPUT,
	                     .trigger = GPIO_TRIG_FALLING};
	GPIO_init(GPIOC, &btn);
	TEST_ASSERT_EQUAL(GPIO_OK, GPIO_pin_deinit(GPIOC, GPIO_PIN_13));
	TEST_ASSERT_EQUAL_HEX32(0U, fake_exti.IMR);
	TEST_ASSERT_EQUAL_HEX32(0U, fake_exti.FTSR);
}

static void test_pin_deinit_refuses_locked_pin(void)
{
	fake_gpio[0].MODER = 0xFFFFFFFFU;
	fake_gpio[0].LCKR = (1U << 16) | GPIO_PIN_5;
	TEST_ASSERT_EQUAL(GPIO_ERR_PIN_LOCKED,
	                  GPIO_pin_deinit(GPIOA, GPIO_PIN_5));
	TEST_ASSERT_EQUAL_HEX32(0xFFFFFFFFU, fake_gpio[0].MODER);
}

static void test_port_deinit_gates_clock(void)
{
	GPIO_peri_clk_control(GPIOA, true);
	TEST_ASSERT_BIT_HIGH(0, fake_rcc.AHB1ENR);
	TEST_ASSERT_EQUAL(GPIO_OK, GPIO_port_deinit(GPIOA));
	TEST_ASSERT_BIT_LOW(0, fake_rcc.AHB1ENR);
	TEST_ASSERT_EQUAL_HEX32(0U, fake_rcc.AHB1RSTR); /* reset released */
}

/* ---- EXTI ---- */

static void test_callback_rejects_bad_masks(void)
{
	TEST_ASSERT_EQUAL(GPIO_ERR_EXTI_NOT_SINGLE,
	                  GPIO_exti_set_callback(0, cb, NULL));
	TEST_ASSERT_EQUAL(
	    GPIO_ERR_EXTI_NOT_SINGLE,
	    GPIO_exti_set_callback(GPIO_PIN_0 | GPIO_PIN_1, cb, NULL));
}

static void test_handler_calls_callback_and_clears_only_its_line(void)
{
	int token = 0;
	GPIO_exti_set_callback(GPIO_PIN_13, cb, &token);
	fake_exti.IMR = BIT(13);
	fake_exti.PR = BIT(13) | BIT(3); /* line 3 is outside 10..15 */

	GPIO_exti_irq_handler(10, 15);

	TEST_ASSERT_EQUAL(1, g_calls);
	TEST_ASSERT_EQUAL_HEX16(GPIO_PIN_13, g_pin);
	TEST_ASSERT_EQUAL_PTR(&token, g_ctx);
	TEST_ASSERT_EQUAL_HEX32(BIT(13),
	                        fake_exti.PR); /* wrote exactly line 13 */
}

static void test_handler_ignores_masked_line(void)
{
	GPIO_exti_set_callback(GPIO_PIN_13, cb, NULL);
	fake_exti.IMR = 0;
	fake_exti.PR = BIT(13);
	GPIO_exti_irq_handler(10, 15);
	TEST_ASSERT_EQUAL(0, g_calls);
}

static void test_handler_runs_two_lines_in_order(void)
{
	GPIO_exti_set_callback(GPIO_PIN_10, cb, NULL);
	GPIO_exti_set_callback(GPIO_PIN_12, cb, NULL);
	fake_exti.IMR = BIT(10) | BIT(12);
	fake_exti.PR = BIT(10) | BIT(12);
	GPIO_exti_irq_handler(10, 15);
	TEST_ASSERT_EQUAL(2, g_calls);
	TEST_ASSERT_EQUAL_HEX16(GPIO_PIN_12, g_pin); /* last one called */
}

static void test_handler_bad_range_does_nothing(void)
{
	fake_exti.IMR = BIT(13);
	fake_exti.PR = BIT(13);
	GPIO_exti_irq_handler(10, 16);
	TEST_ASSERT_EQUAL_HEX32(BIT(13), fake_exti.PR);
	TEST_ASSERT_EQUAL(0, g_calls);
}

static void test_exti_enable_disable(void)
{
	TEST_ASSERT_EQUAL(GPIO_OK, GPIO_exti_enable(GPIO_PIN_13));
	TEST_ASSERT_EQUAL_HEX32(BIT(13), fake_exti.IMR);
	TEST_ASSERT_EQUAL(GPIO_OK, GPIO_exti_disable(GPIO_PIN_13));
	TEST_ASSERT_EQUAL_HEX32(0U, fake_exti.IMR);
	TEST_ASSERT_EQUAL(GPIO_ERR_EXTI_NOT_SINGLE, GPIO_exti_enable(0));
}

/* ---- Lock ---- */

static void test_lock_pins_validation_and_already_locked(void)
{
	TEST_ASSERT_EQUAL(GPIO_ERR_NULL_PTR, GPIO_lock_pins(NULL, GPIO_PIN_5));
	TEST_ASSERT_EQUAL(GPIO_ERR_INVALID_PINS, GPIO_lock_pins(GPIOA, 0));

	fake_gpio[0].LCKR = (1U << 16) | 0x0060U; /* key active, pins 5 and 6 */
	TEST_ASSERT_EQUAL(GPIO_OK, GPIO_lock_pins(GPIOA, GPIO_PIN_5));
	TEST_ASSERT_EQUAL(GPIO_ERR_LOCK_FAILED,
	                  GPIO_lock_pins(GPIOA, GPIO_PIN_7));
}

int main(void)
{
	UNITY_BEGIN();
	RUN_TEST(test_set_reset);
	RUN_TEST(test_toggle);
	RUN_TEST(test_write_pins_ignores_bits_outside_mask);
	RUN_TEST(test_read_pins);
	RUN_TEST(test_init_output_pin);
	RUN_TEST(test_init_leaves_neighbours_untouched);
	RUN_TEST(test_init_alternate_function_pin_low_and_high);
	RUN_TEST(test_init_exti_falling_edge_on_pc13);
	RUN_TEST(test_init_exti_line_busy);
	RUN_TEST(test_init_validation_errors_change_nothing);
	RUN_TEST(test_init_refuses_locked_pin);
	RUN_TEST(test_pin_deinit_restores_swd_pin_reset_state);
	RUN_TEST(test_pin_deinit_plain_port_returns_to_input);
	RUN_TEST(test_pin_deinit_releases_exti_line);
	RUN_TEST(test_pin_deinit_refuses_locked_pin);
	RUN_TEST(test_port_deinit_gates_clock);
	RUN_TEST(test_callback_rejects_bad_masks);
	RUN_TEST(test_handler_calls_callback_and_clears_only_its_line);
	RUN_TEST(test_handler_ignores_masked_line);
	RUN_TEST(test_handler_runs_two_lines_in_order);
	RUN_TEST(test_handler_bad_range_does_nothing);
	RUN_TEST(test_exti_enable_disable);
	RUN_TEST(test_lock_pins_validation_and_already_locked);
	return UNITY_END();
}
