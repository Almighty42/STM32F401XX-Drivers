#include "stm32f401xx.h"
#include "stm32f401xx_RCC_driver.h"
#include "stm32f401xx_RCC_regs.h"
#include "unity.h"
#include <string.h>

RCC_TypeDef fake_rcc;
FLASH_TypeDef fake_flash;
PWR_TypeDef fake_pwr;

/* ---- Fake hardware: ready flags follow enable bits, SWS follows SW ---- */

static struct
{
	bool hse_dead;     /* HSE never becomes ready */
	bool pll_dead;     /* PLL never locks */
	bool switch_stuck; /* SWS never follows SW */
	bool lsi_dead;     /* LSI never becomes ready */
	bool lse_dead;     /* LSE crystal does not start */
} hw;

void fake_hw_step(void)
{
	uint32_t cr =
	    fake_rcc.CR & ~(RCC_CR_HSIRDY | RCC_CR_HSERDY | RCC_CR_PLLRDY);

	if ((cr & RCC_CR_HSION) != 0U) {
		cr |= RCC_CR_HSIRDY;
	}
	if ((cr & RCC_CR_HSEON) != 0U && !hw.hse_dead) {
		cr |= RCC_CR_HSERDY;
	}
	if ((cr & RCC_CR_PLLON) != 0U && !hw.pll_dead) {
		cr |= RCC_CR_PLLRDY;
	}
	fake_rcc.CR = cr;

	fake_rcc.CSR = (fake_rcc.CSR & ~RCC_CSR_LSIRDY) |
	               (((fake_rcc.CSR & RCC_CSR_LSION) != 0U && !hw.lsi_dead)
	                    ? RCC_CSR_LSIRDY
	                    : 0U);
	fake_rcc.BDCR =
	    (fake_rcc.BDCR & ~RCC_BDCR_LSERDY) |
	    (((fake_rcc.BDCR & RCC_BDCR_LSEON) != 0U && !hw.lse_dead)
	         ? RCC_BDCR_LSERDY
	         : 0U);

	if (hw.switch_stuck) {
		return;
	}
	const uint32_t sw = FIELD_GET(RCC_CFGR_SW_MASK, fake_rcc.CFGR);
	const uint32_t ready = (sw == 0U)   ? (cr & RCC_CR_HSIRDY)
	                       : (sw == 1U) ? (cr & RCC_CR_HSERDY)
	                       : (sw == 2U) ? (cr & RCC_CR_PLLRDY)
	                                    : 0U;
	if (ready != 0U) {
		reg_set_field(&fake_rcc.CFGR, RCC_CFGR_SWS_MASK, sw);
	}
}

/* ---- Helpers --------------------------------------------------------- */

#define ASSERT_FIELD(expected, mask, reg)                                      \
	TEST_ASSERT_EQUAL_UINT32((expected), FIELD_GET((mask), (reg)))

static RCC_clock_config_t cfg_hsi(void)
{
	RCC_clock_config_t c = {.sysclk = RCC_SYSCLK_HSI,
	                        .ahb_div = RCC_AHB_DIV_1,
	                        .apb1_div = RCC_APB_DIV_1,
	                        .apb2_div = RCC_APB_DIV_1};
	return c;
}

static RCC_clock_config_t cfg_hse(uint32_t hz, bool bypass)
{
	RCC_clock_config_t c = cfg_hsi();
	c.sysclk = RCC_SYSCLK_HSE;
	c.hse_hz = hz;
	c.hse_bypass = bypass;
	return c;
}

static RCC_clock_config_t cfg_pll_hsi_84(void)
{
	RCC_clock_config_t c = cfg_hsi();
	c.sysclk = RCC_SYSCLK_PLL;
	c.pll = (RCC_pll_config_t){
	    .src = RCC_PLL_SRC_HSI, .m = 8, .n = 168, .p = 4, .q = 7};
	c.apb1_div = RCC_APB_DIV_2;
	return c;
}

static RCC_clock_config_t cfg_pll_hse_84(void)
{
	RCC_clock_config_t c = cfg_pll_hsi_84();
	c.pll = (RCC_pll_config_t){
	    .src = RCC_PLL_SRC_HSE, .m = 8, .n = 336, .p = 4, .q = 7};
	c.hse_hz = 8000000U;
	c.hse_bypass = true;
	return c;
}

static void power_on_reset(void)
{
	memset(&hw, 0, sizeof hw);
	memset(&fake_flash, 0, sizeof fake_flash);
	memset(&fake_pwr, 0, sizeof fake_pwr);
	memset(&fake_rcc, 0, sizeof fake_rcc);
	memset(&fake_flash, 0, sizeof fake_flash);
	fake_rcc.CR = RCC_CR_HSION | RCC_CR_HSIRDY;
}

void setUp(void)
{
	power_on_reset();
	/* The driver remembers the HSE frequency in a static: an HSI init clears it. */
	const RCC_clock_config_t hsi = cfg_hsi();
	(void)RCC_clock_init(&hsi);
	power_on_reset();
}

void tearDown(void)
{
}

/* Initialise with an invalid config and prove no register changed. */
static void expect_rejected(const RCC_clock_config_t* c, RCC_status_t err)
{
	RCC_TypeDef rcc_before;
	FLASH_TypeDef flash_before;

	memcpy(&rcc_before, &fake_rcc, sizeof rcc_before);
	memcpy(&flash_before, &fake_flash, sizeof flash_before);
	TEST_ASSERT_EQUAL(err, RCC_clock_init(c));
	TEST_ASSERT_EQUAL_MEMORY(&rcc_before, &fake_rcc, sizeof rcc_before);
	TEST_ASSERT_EQUAL_MEMORY(&flash_before, &fake_flash,
	                         sizeof flash_before);
}

/* ---- reg_poll -------------------------------------------------------- */

static void test_reg_poll_succeeds_when_bits_match(void)
{
	volatile uint32_t reg = 0x30U;
	TEST_ASSERT_TRUE(reg_poll(&reg, 0x10U, 0x10U, 5U));
	TEST_ASSERT_TRUE(
	    reg_poll(&reg, 0x03U, 0x00U, 5U)); /* waiting for clear */
}

static void test_reg_poll_fails_after_the_given_loops(void)
{
	volatile uint32_t reg = 0U;
	TEST_ASSERT_FALSE(reg_poll(&reg, 0x1U, 0x1U, 3U));
}

static void test_reg_poll_zero_loops_never_succeeds(void)
{
	volatile uint32_t reg = 0x1U;
	TEST_ASSERT_FALSE(reg_poll(&reg, 0x1U, 0x1U, 0U));
}

/* ---- Validation: errors leave every register untouched ---------------- */

static void test_init_null_pointer(void)
{
	TEST_ASSERT_EQUAL(RCC_ERR_NULL_PTR, RCC_clock_init(NULL));
}

static void test_pll_m_out_of_range(void)
{
	RCC_clock_config_t c = cfg_pll_hsi_84();
	c.pll.m = 1;
	expect_rejected(&c, RCC_ERR_INVALID_PLL);
}

static void test_pll_n_out_of_range(void)
{
	RCC_clock_config_t c = cfg_pll_hsi_84();
	c.pll.n = 49;
	expect_rejected(&c, RCC_ERR_INVALID_PLL);
}

static void test_pll_q_out_of_range(void)
{
	RCC_clock_config_t c = cfg_pll_hsi_84();
	c.pll.q = 1;
	expect_rejected(&c, RCC_ERR_INVALID_PLL);
}

static void test_pll_p_must_be_2_4_6_or_8(void)
{
	RCC_clock_config_t c = cfg_pll_hsi_84();
	c.pll.p = 3;
	expect_rejected(&c, RCC_ERR_INVALID_PLL);
}

static void test_pll_vco_input_too_high(void)
{
	RCC_clock_config_t c = cfg_pll_hsi_84();
	c.pll.m = 7; /* 16 MHz / 7 = 2.29 MHz */
	expect_rejected(&c, RCC_ERR_INVALID_PLL);
}

static void test_pll_vco_output_too_low(void)
{
	RCC_clock_config_t c = cfg_pll_hsi_84();
	c.pll.n = 50; /* 2 MHz * 50 = 100 MHz */
	expect_rejected(&c, RCC_ERR_INVALID_PLL);
}

static void test_pll_vco_output_too_high(void)
{
	RCC_clock_config_t c = cfg_pll_hsi_84();
	c.pll.n = 432; /* 2 MHz * 432 = 864 MHz */
	expect_rejected(&c, RCC_ERR_INVALID_PLL);
}

static void test_pll_48mhz_domain_too_fast(void)
{
	RCC_clock_config_t c = cfg_pll_hsi_84();
	c.pll.q = 4; /* 336 MHz / 4 = 84 MHz > 75 MHz */
	expect_rejected(&c, RCC_ERR_INVALID_PLL);
}

static void test_sysclk_above_84mhz_is_a_frequency_limit(void)
{
	RCC_clock_config_t c = cfg_pll_hsi_84();
	c.pll.p = 2; /* 336 MHz / 2 = 168 MHz */
	expect_rejected(&c, RCC_ERR_FREQ_LIMIT);
}

static void test_apb1_above_42mhz_is_a_frequency_limit(void)
{
	RCC_clock_config_t c = cfg_pll_hsi_84();
	c.apb1_div = RCC_APB_DIV_1; /* 84 MHz on APB1 */
	expect_rejected(&c, RCC_ERR_FREQ_LIMIT);
}

static void test_invalid_prescalers(void)
{
	RCC_clock_config_t c = cfg_hsi();
	c.ahb_div = (RCC_ahb_div_t)3;
	expect_rejected(&c, RCC_ERR_INVALID_PRESCALER);

	c = cfg_hsi();
	c.apb2_div = (RCC_apb_div_t)1;
	expect_rejected(&c, RCC_ERR_INVALID_PRESCALER);
}

static void test_invalid_sources(void)
{
	RCC_clock_config_t c = cfg_hse(0U, false); /* HSE without a frequency */
	expect_rejected(&c, RCC_ERR_INVALID_SOURCE);

	c = cfg_pll_hse_84();
	c.hse_hz = 0U;
	expect_rejected(&c, RCC_ERR_INVALID_SOURCE);

	c = cfg_hsi();
	c.sysclk = (RCC_sysclk_src_t)3;
	expect_rejected(&c, RCC_ERR_INVALID_SOURCE);

	c = cfg_pll_hsi_84();
	c.pll.src = (RCC_pll_src_t)2;
	expect_rejected(&c, RCC_ERR_INVALID_SOURCE);
}

/* ---- Successful sequences --------------------------------------------- */

static void test_init_hsi_to_hsi(void)
{
	const RCC_clock_config_t c = cfg_hsi();
	TEST_ASSERT_EQUAL(RCC_OK, RCC_clock_init(&c));
	ASSERT_FIELD(0U, RCC_CFGR_SWS_MASK, fake_rcc.CFGR);
	ASSERT_FIELD(0U, FLASH_ACR_LATENCY_MASK, fake_flash.ACR);
	TEST_ASSERT_EQUAL_UINT32(RCC_HSI_HZ, RCC_get_sysclk_hz());
}

static void test_init_pll_from_hsi_84mhz(void)
{
	const uint32_t accel =
	    FLASH_ACR_PRFTEN | FLASH_ACR_ICEN | FLASH_ACR_DCEN;
	const RCC_clock_config_t c = cfg_pll_hsi_84();

	TEST_ASSERT_EQUAL(RCC_OK, RCC_clock_init(&c));

	ASSERT_FIELD(8U, RCC_PLLCFGR_PLLM_MASK, fake_rcc.PLLCFGR);
	ASSERT_FIELD(168U, RCC_PLLCFGR_PLLN_MASK, fake_rcc.PLLCFGR);
	ASSERT_FIELD(1U, RCC_PLLCFGR_PLLP_MASK, fake_rcc.PLLCFGR); /* p = 4 */
	ASSERT_FIELD(7U, RCC_PLLCFGR_PLLQ_MASK, fake_rcc.PLLCFGR);
	ASSERT_FIELD(0U, RCC_PLLCFGR_PLLSRC, fake_rcc.PLLCFGR);
	TEST_ASSERT_BITS_HIGH(RCC_CR_PLLON, fake_rcc.CR);
	ASSERT_FIELD(2U, RCC_CFGR_SWS_MASK, fake_rcc.CFGR);
	ASSERT_FIELD(0U, RCC_CFGR_HPRE_MASK, fake_rcc.CFGR);
	ASSERT_FIELD(4U, RCC_CFGR_PPRE1_MASK, fake_rcc.CFGR); /* /2 */
	ASSERT_FIELD(0U, RCC_CFGR_PPRE2_MASK, fake_rcc.CFGR); /* /1 */
	ASSERT_FIELD(2U, FLASH_ACR_LATENCY_MASK, fake_flash.ACR);
	TEST_ASSERT_BITS_HIGH(accel, fake_flash.ACR);

	TEST_ASSERT_EQUAL_UINT32(84000000U, RCC_get_sysclk_hz());
	TEST_ASSERT_EQUAL_UINT32(84000000U, RCC_get_hclk_hz());
	TEST_ASSERT_EQUAL_UINT32(42000000U, RCC_get_pclk1_hz());
	TEST_ASSERT_EQUAL_UINT32(84000000U, RCC_get_pclk2_hz());
}

static void test_init_pll_from_hse_bypass_84mhz(void)
{
	const RCC_clock_config_t c = cfg_pll_hse_84();

	TEST_ASSERT_EQUAL(RCC_OK, RCC_clock_init(&c));

	TEST_ASSERT_BITS_HIGH(RCC_CR_HSEON | RCC_CR_HSEBYP, fake_rcc.CR);
	ASSERT_FIELD(1U, RCC_PLLCFGR_PLLSRC, fake_rcc.PLLCFGR);
	ASSERT_FIELD(336U, RCC_PLLCFGR_PLLN_MASK, fake_rcc.PLLCFGR);
	TEST_ASSERT_EQUAL_UINT32(84000000U, RCC_get_sysclk_hz());
}

/* Regression: HSEON was tested through FIELD_PREP, which read HSION. */
static void test_init_hse_starts_hse_while_hsi_is_on(void)
{
	const RCC_clock_config_t c = cfg_hse(8000000U, false);

	TEST_ASSERT_BITS_HIGH(RCC_CR_HSION, fake_rcc.CR);
	TEST_ASSERT_EQUAL(RCC_OK, RCC_clock_init(&c));
	TEST_ASSERT_BITS_HIGH(RCC_CR_HSEON, fake_rcc.CR);
	TEST_ASSERT_BITS_LOW(RCC_CR_HSEBYP,
	                     fake_rcc.CR); /* crystal, not bypass */
	ASSERT_FIELD(1U, RCC_CFGR_SWS_MASK, fake_rcc.CFGR);
	TEST_ASSERT_EQUAL_UINT32(8000000U, RCC_get_sysclk_hz());
}

static void test_flash_latency_boundaries(void)
{
	static const struct
	{
		uint32_t hz;
		uint32_t ws;
	} cases[] = {{16000000U, 0U}, {30000000U, 0U}, {30000001U, 1U},
	             {60000000U, 1U}, {60000001U, 2U}, {84000000U, 2U}};

	for (size_t i = 0; i < sizeof cases / sizeof cases[0]; i++) {
		power_on_reset();
		RCC_clock_config_t c = cfg_hse(cases[i].hz, true);
		c.apb1_div = RCC_APB_DIV_2;
		TEST_ASSERT_EQUAL(RCC_OK, RCC_clock_init(&c));
		TEST_ASSERT_EQUAL_UINT32_MESSAGE(
		    cases[i].ws,
		    FIELD_GET(FLASH_ACR_LATENCY_MASK, fake_flash.ACR),
		    "flash wait states at a boundary");
	}
}

static void test_reinit_from_pll_back_to_hsi(void)
{
	const RCC_clock_config_t pll = cfg_pll_hsi_84();
	const RCC_clock_config_t hsi = cfg_hsi();

	TEST_ASSERT_EQUAL(RCC_OK, RCC_clock_init(&pll));
	TEST_ASSERT_EQUAL(RCC_OK, RCC_clock_init(&hsi));
	ASSERT_FIELD(0U, RCC_CFGR_SWS_MASK, fake_rcc.CFGR);
	ASSERT_FIELD(0U, FLASH_ACR_LATENCY_MASK, fake_flash.ACR);
	TEST_ASSERT_EQUAL_UINT32(RCC_HSI_HZ, RCC_get_sysclk_hz());
}

/* The PLL runs the core and is reconfigured: the driver steps off it first. */
static void test_reinit_pll_to_another_pll(void)
{
	RCC_clock_config_t second = cfg_pll_hsi_84();
	const RCC_clock_config_t first = cfg_pll_hsi_84();

	second.pll.n = 144; /* 288 MHz VCO, 72 MHz SYSCLK, 48 MHz on Q */
	second.pll.q = 6;

	TEST_ASSERT_EQUAL(RCC_OK, RCC_clock_init(&first));
	TEST_ASSERT_EQUAL(RCC_OK, RCC_clock_init(&second));
	ASSERT_FIELD(144U, RCC_PLLCFGR_PLLN_MASK, fake_rcc.PLLCFGR);
	ASSERT_FIELD(2U, RCC_CFGR_SWS_MASK, fake_rcc.CFGR);
	TEST_ASSERT_EQUAL_UINT32(72000000U, RCC_get_sysclk_hz());
}

/* ---- Failure paths ----------------------------------------------------- */

static void test_hse_never_ready_times_out_and_restores_dividers(void)
{
	RCC_clock_config_t c = cfg_pll_hse_84();

	fake_rcc.CFGR = FIELD_PREP(RCC_CFGR_PPRE1_MASK, 5U) |
	                FIELD_PREP(RCC_CFGR_PPRE2_MASK, 4U);
	const uint32_t before = fake_rcc.CFGR;
	hw.hse_dead = true;

	TEST_ASSERT_EQUAL(RCC_ERR_TIMEOUT, RCC_clock_init(&c));
	TEST_ASSERT_EQUAL_HEX32(before, fake_rcc.CFGR);
	TEST_ASSERT_EQUAL_UINT32(RCC_HSI_HZ, RCC_get_sysclk_hz());
}

static void test_pll_never_locks_times_out(void)
{
	RCC_clock_config_t c = cfg_pll_hsi_84();

	hw.pll_dead = true;
	TEST_ASSERT_EQUAL(RCC_ERR_TIMEOUT, RCC_clock_init(&c));
	ASSERT_FIELD(0U, RCC_CFGR_SWS_MASK, fake_rcc.CFGR);
	ASSERT_FIELD(0U, RCC_CFGR_SW_MASK, fake_rcc.CFGR);
}

/* The core was running from the PLL: after a failed reconfigure it is on HSI. */
static void test_failed_pll_reconfigure_leaves_core_on_hsi(void)
{
	RCC_clock_config_t second = cfg_pll_hsi_84();
	const RCC_clock_config_t first = cfg_pll_hsi_84();

	second.pll.n = 144;
	second.pll.q = 6;

	TEST_ASSERT_EQUAL(RCC_OK, RCC_clock_init(&first));
	hw.pll_dead = true;
	TEST_ASSERT_EQUAL(RCC_ERR_TIMEOUT, RCC_clock_init(&second));
	TEST_ASSERT_EQUAL_UINT32(RCC_HSI_HZ, RCC_get_sysclk_hz());
}

/* A switch that never completes must not leave the new source requested. */
static void test_failed_switch_cancels_the_request(void)
{
	RCC_clock_config_t c = cfg_pll_hsi_84();

	hw.switch_stuck = true;
	TEST_ASSERT_EQUAL(RCC_ERR_TIMEOUT, RCC_clock_init(&c));
	ASSERT_FIELD(0U, RCC_CFGR_SWS_MASK, fake_rcc.CFGR);
	ASSERT_FIELD(0U, RCC_CFGR_SW_MASK, fake_rcc.CFGR);
}

/* ---- Getters ----------------------------------------------------------- */

static void test_getters_after_reset_report_hsi(void)
{
	TEST_ASSERT_EQUAL_UINT32(RCC_HSI_HZ, RCC_get_sysclk_hz());
	TEST_ASSERT_EQUAL_UINT32(RCC_HSI_HZ, RCC_get_hclk_hz());
	TEST_ASSERT_EQUAL_UINT32(RCC_HSI_HZ, RCC_get_pclk1_hz());
	TEST_ASSERT_EQUAL_UINT32(RCC_HSI_HZ, RCC_get_pclk2_hz());
}

static void test_sysclk_from_pll_registers(void)
{
	fake_rcc.PLLCFGR = 0x07012A08U; /* HSI, M=8, N=168, P=4, Q=7 */
	fake_rcc.CFGR = FIELD_PREP(RCC_CFGR_SWS_MASK, 2U);
	TEST_ASSERT_EQUAL_UINT32(84000000U, RCC_get_sysclk_hz());
}

static void test_sysclk_unknown_cases_report_zero(void)
{
	fake_rcc.CFGR =
	    FIELD_PREP(RCC_CFGR_SWS_MASK, 1U); /* HSE, never inited */
	TEST_ASSERT_EQUAL_UINT32(0U, RCC_get_sysclk_hz());

	fake_rcc.CFGR = FIELD_PREP(RCC_CFGR_SWS_MASK, 3U); /* not allowed */
	TEST_ASSERT_EQUAL_UINT32(0U, RCC_get_sysclk_hz());

	fake_rcc.PLLCFGR = 0U; /* M = 0 */
	fake_rcc.CFGR = FIELD_PREP(RCC_CFGR_SWS_MASK, 2U);
	TEST_ASSERT_EQUAL_UINT32(0U, RCC_get_sysclk_hz());
}

static void test_hclk_pclk_with_dividers(void)
{
	fake_rcc.PLLCFGR = 0x07012A08U;
	fake_rcc.CFGR = FIELD_PREP(RCC_CFGR_SWS_MASK, 2U) |
	                FIELD_PREP(RCC_CFGR_HPRE_MASK, 8U) |  /* AHB /2 */
	                FIELD_PREP(RCC_CFGR_PPRE1_MASK, 4U) | /* APB1 /2 */
	                FIELD_PREP(RCC_CFGR_PPRE2_MASK, 0U);  /* APB2 /1 */
	TEST_ASSERT_EQUAL_UINT32(42000000U, RCC_get_hclk_hz());
	TEST_ASSERT_EQUAL_UINT32(21000000U, RCC_get_pclk1_hz());
	TEST_ASSERT_EQUAL_UINT32(42000000U, RCC_get_pclk2_hz());
}

static void test_every_hpre_encoding_decodes(void)
{
	static const uint32_t div[16] = {1, 1, 1, 1,  1,  1,   1,   1,
	                                 2, 4, 8, 16, 64, 128, 256, 512};
	for (uint32_t i = 0U; i < 16U; i++) {
		fake_rcc.CFGR = FIELD_PREP(RCC_CFGR_HPRE_MASK, i);
		TEST_ASSERT_EQUAL_UINT32_MESSAGE(
		    RCC_HSI_HZ / div[i], RCC_get_hclk_hz(), "HPRE encoding");
	}
}

static void test_every_ppre_encoding_decodes(void)
{
	static const uint32_t div[8] = {1, 1, 1, 1, 2, 4, 8, 16};
	for (uint32_t i = 0U; i < 8U; i++) {
		fake_rcc.CFGR = FIELD_PREP(RCC_CFGR_PPRE1_MASK, i) |
		                FIELD_PREP(RCC_CFGR_PPRE2_MASK, i);
		TEST_ASSERT_EQUAL_UINT32_MESSAGE(
		    RCC_HSI_HZ / div[i], RCC_get_pclk1_hz(), "PPRE1 encoding");
		TEST_ASSERT_EQUAL_UINT32_MESSAGE(
		    RCC_HSI_HZ / div[i], RCC_get_pclk2_hz(), "PPRE2 encoding");
	}
}

/* ---- MCO1 -------------------------------------------------------------- */

static void test_mco1_pll_div5_writes_one_word(void)
{
	fake_rcc.CFGR = 0U;
	TEST_ASSERT_EQUAL(RCC_OK, RCC_mco1_config(RCC_MCO1_PLL, RCC_MCO_DIV_5));
	TEST_ASSERT_EQUAL_HEX32(0x07600000U, fake_rcc.CFGR);
}

static void test_mco1_all_valid_combinations(void)
{
	static const uint32_t pre[] = {0U, 4U, 5U, 6U, 7U};

	for (uint32_t s = 0U; s <= 3U; s++) {
		for (size_t j = 0; j < sizeof pre / sizeof pre[0]; j++) {
			fake_rcc.CFGR = 0U;
			TEST_ASSERT_EQUAL(
			    RCC_OK, RCC_mco1_config((RCC_mco1_src_t)s,
			                            (RCC_mco_div_t)pre[j]));
			TEST_ASSERT_EQUAL_HEX32(
			    FIELD_PREP(RCC_CFGR_MCO1_MASK, s) |
			        FIELD_PREP(RCC_CFGR_MCO1PRE_MASK, pre[j]),
			    fake_rcc.CFGR);
		}
	}
}

static void test_mco1_keeps_other_cfgr_bits(void)
{
	fake_rcc.CFGR = 0x0000A0F3U;
	RCC_mco1_config(RCC_MCO1_HSE, RCC_MCO_DIV_1);
	TEST_ASSERT_EQUAL_HEX32(0x0040A0F3U, fake_rcc.CFGR);
}

static void test_mco1_rejects_bad_values_and_changes_nothing(void)
{
	fake_rcc.CFGR = 0x1234U;
	TEST_ASSERT_EQUAL(RCC_ERR_INVALID_SOURCE,
	                  RCC_mco1_config((RCC_mco1_src_t)4, RCC_MCO_DIV_1));
	TEST_ASSERT_EQUAL(RCC_ERR_INVALID_PRESCALER,
	                  RCC_mco1_config(RCC_MCO1_HSI, (RCC_mco_div_t)1));
	TEST_ASSERT_EQUAL(RCC_ERR_INVALID_PRESCALER,
	                  RCC_mco1_config(RCC_MCO1_HSI, (RCC_mco_div_t)3));
	TEST_ASSERT_EQUAL(RCC_ERR_INVALID_PRESCALER,
	                  RCC_mco1_config(RCC_MCO1_HSI, (RCC_mco_div_t)8));
	TEST_ASSERT_EQUAL_HEX32(0x1234U, fake_rcc.CFGR);
}

/* ---- LSI / LSE --------------------------------------------------------- */

static void test_lsi_enable_sets_lsion_and_reports_ready(void)
{
	TEST_ASSERT_EQUAL(RCC_OK, RCC_lsi_enable());
	TEST_ASSERT_BITS_HIGH(RCC_CSR_LSION, fake_rcc.CSR);
	TEST_ASSERT_BITS_HIGH(RCC_CSR_LSIRDY, fake_rcc.CSR);
}

static void test_lsi_never_ready_times_out_and_switches_off(void)
{
	hw.lsi_dead = true;
	TEST_ASSERT_EQUAL(RCC_ERR_TIMEOUT, RCC_lsi_enable());
	TEST_ASSERT_BITS_LOW(RCC_CSR_LSION, fake_rcc.CSR);
}

static void test_lsi_disable_clears_lsion(void)
{
	TEST_ASSERT_EQUAL(RCC_OK, RCC_lsi_enable());
	RCC_lsi_disable();
	TEST_ASSERT_BITS_LOW(RCC_CSR_LSION, fake_rcc.CSR);
	fake_hw_step();
	TEST_ASSERT_BITS_LOW(RCC_CSR_LSIRDY, fake_rcc.CSR);
}

static void test_lse_start_unlocks_backup_domain_and_starts(void)
{
	RCC_lse_start(false);
	TEST_ASSERT_BITS_HIGH(RCC_APB1ENR_PWREN, fake_rcc.APB1ENR);
	TEST_ASSERT_BITS_HIGH(PWR_CR_DBP, fake_pwr.CR);
	TEST_ASSERT_BITS_HIGH(RCC_BDCR_LSEON, fake_rcc.BDCR);
	TEST_ASSERT_BITS_LOW(RCC_BDCR_LSEBYP, fake_rcc.BDCR); /* crystal */
}

static void test_lse_start_bypass_sets_lsebyp(void)
{
	RCC_lse_start(true);
	TEST_ASSERT_BITS_HIGH(RCC_BDCR_LSEON | RCC_BDCR_LSEBYP, fake_rcc.BDCR);
}

static void test_lse_bypass_is_not_changed_while_running(void)
{
	RCC_lse_start(false);
	RCC_lse_start(true); /* LSEBYP is writable only while LSE is off */
	TEST_ASSERT_BITS_LOW(RCC_BDCR_LSEBYP, fake_rcc.BDCR);

	RCC_lse_stop();
	RCC_lse_start(true);
	RCC_lse_start(false);
	TEST_ASSERT_BITS_HIGH(RCC_BDCR_LSEBYP, fake_rcc.BDCR);
}

static void test_lse_ready_only_after_the_hardware_reports_it(void)
{
	TEST_ASSERT_FALSE(RCC_lse_ready());
	RCC_lse_start(false);
	TEST_ASSERT_FALSE(RCC_lse_ready()); /* start does not wait */
	fake_hw_step();
	TEST_ASSERT_TRUE(RCC_lse_ready());
}

static void test_lse_dead_crystal_never_becomes_ready(void)
{
	hw.lse_dead = true;
	RCC_lse_start(false);
	fake_hw_step();
	fake_hw_step();
	TEST_ASSERT_BITS_HIGH(RCC_BDCR_LSEON, fake_rcc.BDCR);
	TEST_ASSERT_FALSE(RCC_lse_ready());
}

static void test_lse_stop_clears_lseon_and_ready_follows(void)
{
	RCC_lse_start(false);
	fake_hw_step();
	RCC_lse_stop();
	TEST_ASSERT_BITS_LOW(RCC_BDCR_LSEON, fake_rcc.BDCR);
	fake_hw_step();
	TEST_ASSERT_FALSE(RCC_lse_ready());
}

static void test_lse_stop_unlocks_backup_domain_too(void)
{
	RCC_lse_stop(); /* DBP is still 0 after reset */
	TEST_ASSERT_BITS_HIGH(PWR_CR_DBP, fake_pwr.CR);
}

static void test_low_speed_oscillators_do_not_touch_the_system_clock(void)
{
	const RCC_clock_config_t pll = cfg_pll_hsi_84();
	RCC_TypeDef before;
	FLASH_TypeDef flash_before;

	TEST_ASSERT_EQUAL(RCC_OK, RCC_clock_init(&pll));
	memcpy(&before, &fake_rcc, sizeof before);
	memcpy(&flash_before, &fake_flash, sizeof flash_before);

	TEST_ASSERT_EQUAL(RCC_OK, RCC_lsi_enable());
	RCC_lse_start(false);
	fake_hw_step();
	RCC_lse_stop();
	RCC_lsi_disable();

	TEST_ASSERT_EQUAL_HEX32(before.CR, fake_rcc.CR);
	TEST_ASSERT_EQUAL_HEX32(before.CFGR, fake_rcc.CFGR);
	TEST_ASSERT_EQUAL_HEX32(before.PLLCFGR, fake_rcc.PLLCFGR);
	TEST_ASSERT_EQUAL_MEMORY(&flash_before, &fake_flash,
	                         sizeof flash_before);
	TEST_ASSERT_EQUAL_UINT32(84000000U, RCC_get_sysclk_hz());
}

int main(void)
{
	UNITY_BEGIN();
	RUN_TEST(test_reg_poll_succeeds_when_bits_match);
	RUN_TEST(test_reg_poll_fails_after_the_given_loops);
	RUN_TEST(test_reg_poll_zero_loops_never_succeeds);
	RUN_TEST(test_init_null_pointer);
	RUN_TEST(test_pll_m_out_of_range);
	RUN_TEST(test_pll_n_out_of_range);
	RUN_TEST(test_pll_q_out_of_range);
	RUN_TEST(test_pll_p_must_be_2_4_6_or_8);
	RUN_TEST(test_pll_vco_input_too_high);
	RUN_TEST(test_pll_vco_output_too_low);
	RUN_TEST(test_pll_vco_output_too_high);
	RUN_TEST(test_pll_48mhz_domain_too_fast);
	RUN_TEST(test_sysclk_above_84mhz_is_a_frequency_limit);
	RUN_TEST(test_apb1_above_42mhz_is_a_frequency_limit);
	RUN_TEST(test_invalid_prescalers);
	RUN_TEST(test_invalid_sources);
	RUN_TEST(test_init_hsi_to_hsi);
	RUN_TEST(test_init_pll_from_hsi_84mhz);
	RUN_TEST(test_init_pll_from_hse_bypass_84mhz);
	RUN_TEST(test_init_hse_starts_hse_while_hsi_is_on);
	RUN_TEST(test_flash_latency_boundaries);
	RUN_TEST(test_reinit_from_pll_back_to_hsi);
	RUN_TEST(test_reinit_pll_to_another_pll);
	RUN_TEST(test_hse_never_ready_times_out_and_restores_dividers);
	RUN_TEST(test_pll_never_locks_times_out);
	RUN_TEST(test_failed_pll_reconfigure_leaves_core_on_hsi);
	RUN_TEST(test_failed_switch_cancels_the_request);
	RUN_TEST(test_getters_after_reset_report_hsi);
	RUN_TEST(test_sysclk_from_pll_registers);
	RUN_TEST(test_sysclk_unknown_cases_report_zero);
	RUN_TEST(test_hclk_pclk_with_dividers);
	RUN_TEST(test_every_hpre_encoding_decodes);
	RUN_TEST(test_every_ppre_encoding_decodes);
	RUN_TEST(test_mco1_pll_div5_writes_one_word);
	RUN_TEST(test_mco1_all_valid_combinations);
	RUN_TEST(test_mco1_keeps_other_cfgr_bits);
	RUN_TEST(test_mco1_rejects_bad_values_and_changes_nothing);
	RUN_TEST(test_lsi_enable_sets_lsion_and_reports_ready);
	RUN_TEST(test_lsi_never_ready_times_out_and_switches_off);
	RUN_TEST(test_lsi_disable_clears_lsion);
	RUN_TEST(test_lse_start_unlocks_backup_domain_and_starts);
	RUN_TEST(test_lse_start_bypass_sets_lsebyp);
	RUN_TEST(test_lse_bypass_is_not_changed_while_running);
	RUN_TEST(test_lse_ready_only_after_the_hardware_reports_it);
	RUN_TEST(test_lse_dead_crystal_never_becomes_ready);
	RUN_TEST(test_lse_stop_clears_lseon_and_ready_follows);
	RUN_TEST(test_lse_stop_unlocks_backup_domain_too);
	RUN_TEST(test_low_speed_oscillators_do_not_touch_the_system_clock);
	return UNITY_END();
}
