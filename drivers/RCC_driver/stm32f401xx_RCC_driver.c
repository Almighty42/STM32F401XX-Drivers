/**
 * @file stm32f401xx_RCC_driver.c
 * @brief Implementation of the RCC clock driver. The API is documented in
 * the header.
 */

#include "stm32f401xx_RCC_driver.h"
#include "reg_utils.h"
#include "stm32f401xx.h"
#include "stm32f401xx_RCC_regs.h"
#include <stddef.h>
#include <stdint.h>

#define RCC_WAIT_LOOPS 100000U
#define RCC_FLASH_WS_STEP_HZ 30000000U

#define RCC_PLLM_MIN 2U
#define RCC_PLLM_MAX 63U
#define RCC_PLLN_MIN 50U
#define RCC_PLLN_MAX 432U
#define RCC_PLLQ_MIN 2U
#define RCC_PLLQ_MAX 15U
#define RCC_VCO_IN_MIN_HZ 1000000U
#define RCC_VCO_IN_MAX_HZ 2000000U
#define RCC_VCO_MIN_HZ 192000000U
#define RCC_VCO_MAX_HZ 432000000U
#define RCC_PLL_OUT_MIN_HZ 24000000U
#define RCC_PLL48_MAX_HZ 75000000U

#define RCC_SW_HSI 0U
#define RCC_SW_HSE 1U
#define RCC_SW_PLL 2U

typedef struct
{
	uint32_t sysclk;
	uint32_t hclk;
	uint32_t pclk1;
	uint32_t pclk2;
} rcc_freqs_t;

static uint32_t rcc_hse_hz; /* the value given to RCC_clock_init */

static uint32_t rcc_ahb_div_value(RCC_ahb_div_t d)
{
	switch (d) {
		case RCC_AHB_DIV_1:
			return 1U;
		case RCC_AHB_DIV_2:
			return 2U;
		case RCC_AHB_DIV_4:
			return 4U;
		case RCC_AHB_DIV_8:
			return 8U;
		case RCC_AHB_DIV_16:
			return 16U;
		case RCC_AHB_DIV_64:
			return 64U;
		case RCC_AHB_DIV_128:
			return 128U;
		case RCC_AHB_DIV_256:
			return 256U;
		case RCC_AHB_DIV_512:
			return 512U;
		default:
			return 0U;
	}
}

static uint32_t rcc_apb_div_value(RCC_apb_div_t d)
{
	switch (d) {
		case RCC_APB_DIV_1:
			return 1U;
		case RCC_APB_DIV_2:
			return 2U;
		case RCC_APB_DIV_4:
			return 4U;
		case RCC_APB_DIV_8:
			return 8U;
		case RCC_APB_DIV_16:
			return 16U;
		default:
			return 0U;
	}
}

/** VCO frequency: source / m * n, computed in 64 bits to avoid overflow. */
static uint32_t rcc_pll_vco_hz(uint32_t src_hz, uint32_t m, uint32_t n)
{
	return (uint32_t)((uint64_t)src_hz * n / m);
}

static RCC_status_t rcc_pll_output(const RCC_pll_config_t* pll, uint32_t src_hz,
                                   uint32_t* out_hz)
{
	if (pll->m < RCC_PLLM_MIN || pll->m > RCC_PLLM_MAX ||
	    pll->n < RCC_PLLN_MIN || pll->n > RCC_PLLN_MAX ||
	    pll->q < RCC_PLLQ_MIN || pll->q > RCC_PLLQ_MAX) {
		return RCC_ERR_INVALID_PLL;
	}
	if (pll->p != 2U && pll->p != 4U && pll->p != 6U && pll->p != 8U) {
		return RCC_ERR_INVALID_PLL;
	}

	if (src_hz < RCC_VCO_IN_MIN_HZ * pll->m ||
	    src_hz > RCC_VCO_IN_MAX_HZ * pll->m) {
		return RCC_ERR_INVALID_PLL;
	}

	const uint32_t vco_out = rcc_pll_vco_hz(src_hz, pll->m, pll->n);
	if (vco_out < RCC_VCO_MIN_HZ || vco_out > RCC_VCO_MAX_HZ) {
		return RCC_ERR_INVALID_PLL;
	}
	if (vco_out / pll->q > RCC_PLL48_MAX_HZ) {
		return RCC_ERR_INVALID_PLL;
	}

	*out_hz = vco_out / pll->p;
	if (*out_hz < RCC_PLL_OUT_MIN_HZ) {
		return RCC_ERR_INVALID_PLL;
	}
	return RCC_OK;
}

static RCC_status_t rcc_compute(const RCC_clock_config_t* cfg, rcc_freqs_t* f)
{
	const bool pll_used = (cfg->sysclk == RCC_SYSCLK_PLL);

	if (pll_used && cfg->pll.src != RCC_PLL_SRC_HSI &&
	    cfg->pll.src != RCC_PLL_SRC_HSE) {
		return RCC_ERR_INVALID_SOURCE;
	}
	const bool uses_hse = (cfg->sysclk == RCC_SYSCLK_HSE) ||
	                      (pll_used && cfg->pll.src == RCC_PLL_SRC_HSE);
	if (uses_hse && cfg->hse_hz == 0U) {
		return RCC_ERR_INVALID_SOURCE;
	}

	switch (cfg->sysclk) {
		case RCC_SYSCLK_HSI:
			f->sysclk = RCC_HSI_HZ;
			break;
		case RCC_SYSCLK_HSE:
			f->sysclk = cfg->hse_hz;
			break;
		case RCC_SYSCLK_PLL: {
			const uint32_t src_hz =
			    (cfg->pll.src == RCC_PLL_SRC_HSE) ? cfg->hse_hz
			                                      : RCC_HSI_HZ;
			const RCC_status_t st =
			    rcc_pll_output(&cfg->pll, src_hz, &f->sysclk);
			if (st != RCC_OK) {
				return st;
			}
			break;
		}
		default:
			return RCC_ERR_INVALID_SOURCE;
	}

	const uint32_t ahb = rcc_ahb_div_value(cfg->ahb_div);
	const uint32_t apb1 = rcc_apb_div_value(cfg->apb1_div);
	const uint32_t apb2 = rcc_apb_div_value(cfg->apb2_div);
	if (ahb == 0U || apb1 == 0U || apb2 == 0U) {
		return RCC_ERR_INVALID_PRESCALER;
	}

	f->hclk = f->sysclk / ahb;
	f->pclk1 = f->hclk / apb1;
	f->pclk2 = f->hclk / apb2;

	if (f->sysclk > RCC_SYSCLK_MAX_HZ || f->pclk1 > RCC_APB1_MAX_HZ ||
	    f->pclk2 > RCC_APB2_MAX_HZ) {
		return RCC_ERR_FREQ_LIMIT;
	}
	return RCC_OK;
}

static uint32_t rcc_flash_latency(uint32_t hclk_hz)
{
	return (hclk_hz == 0U) ? 0U : (hclk_hz - 1U) / RCC_FLASH_WS_STEP_HZ;
}

static uint32_t rcc_sw_encoding(RCC_sysclk_src_t src)
{
	return (src == RCC_SYSCLK_HSI)   ? RCC_SW_HSI
	       : (src == RCC_SYSCLK_HSE) ? RCC_SW_HSE
	                                 : RCC_SW_PLL;
}

/** SYSCLK from the PLL, computed from what is in RCC_PLLCFGR right now. */
static uint32_t rcc_pll_hz_from_regs(void)
{
	const uint32_t pllcfgr = RCC->PLLCFGR;
	const uint32_t m = FIELD_GET(RCC_PLLCFGR_PLLM_MASK, pllcfgr);
	const uint32_t n = FIELD_GET(RCC_PLLCFGR_PLLN_MASK, pllcfgr);
	const uint32_t p =
	    (FIELD_GET(RCC_PLLCFGR_PLLP_MASK, pllcfgr) + 1U) * 2U;
	const uint32_t src_hz = (FIELD_GET(RCC_PLLCFGR_PLLSRC, pllcfgr) != 0U)
	                            ? rcc_hse_hz
	                            : RCC_HSI_HZ;

	if (m < RCC_PLLM_MIN) {
		return 0U; /* not a valid PLL setting: report "unknown" */
	}
	return rcc_pll_vco_hz(src_hz, m, n) / p;
}

/* Register encoding to divider. Every value is covered, including the
 * "0xxx = not divided" ones, so any register content gives a divider >= 1. */
static uint32_t rcc_ahb_div_decode(uint32_t hpre)
{
	static const uint16_t div[16] = {1, 1, 1, 1,  1,  1,   1,   1,
	                                 2, 4, 8, 16, 64, 128, 256, 512};
	return div[hpre & 0xFU];
}

static uint32_t rcc_apb_div_decode(uint32_t ppre)
{
	static const uint8_t div[8] = {1, 1, 1, 1, 2, 4, 8, 16};
	return div[ppre & 0x7U];
}

/** Bus clock for the APB bus whose divider field is @p ppre_mask. */
static uint32_t rcc_pclk_hz(uint32_t ppre_mask)
{
	return RCC_get_hclk_hz() /
	       rcc_apb_div_decode(FIELD_GET(ppre_mask, RCC->CFGR));
}

/* ---- Private: hardware steps ----------------------------------------- */

static bool rcc_flash_set_latency(uint32_t ws)
{
	reg_set_field(&FLASH->ACR, FLASH_ACR_LATENCY_MASK, ws);
	return reg_poll(&FLASH->ACR, FLASH_ACR_LATENCY_MASK,
	                FIELD_PREP(FLASH_ACR_LATENCY_MASK, ws), RCC_WAIT_LOOPS);
}

static void rcc_set_apb_dividers(RCC_apb_div_t apb1, RCC_apb_div_t apb2)
{
	reg_set_field(&RCC->CFGR, RCC_CFGR_PPRE1_MASK, (uint32_t)apb1);
	reg_set_field(&RCC->CFGR, RCC_CFGR_PPRE2_MASK, (uint32_t)apb2);
}

static bool rcc_hsi_start(void)
{
	reg_set_field(&RCC->CR, RCC_CR_HSION, 1U);
	return reg_poll(&RCC->CR, RCC_CR_HSIRDY, RCC_CR_HSIRDY, RCC_WAIT_LOOPS);
}

static bool rcc_hse_start(bool bypass)
{
	/* HSEBYP is writable only while HSE is off */
	if (FIELD_GET(RCC_CR_HSEON, RCC->CR) == 0U) {
		reg_set_field(&RCC->CR, RCC_CR_HSEBYP, bypass ? 1U : 0U);
		reg_set_field(&RCC->CR, RCC_CR_HSEON, 1U);
	}
	return reg_poll(&RCC->CR, RCC_CR_HSERDY, RCC_CR_HSERDY, RCC_WAIT_LOOPS);
}

static bool rcc_switch_sysclk(uint32_t sw)
{
	reg_set_field(&RCC->CFGR, RCC_CFGR_SW_MASK, sw);
	return reg_poll(&RCC->CFGR, RCC_CFGR_SWS_MASK,
	                FIELD_PREP(RCC_CFGR_SWS_MASK, sw), RCC_WAIT_LOOPS);
}

static bool rcc_pll_start(const RCC_pll_config_t* pll)
{
	const uint32_t mask = RCC_PLLCFGR_PLLM_MASK | RCC_PLLCFGR_PLLN_MASK |
	                      RCC_PLLCFGR_PLLP_MASK | RCC_PLLCFGR_PLLQ_MASK |
	                      RCC_PLLCFGR_PLLSRC;
	const uint32_t value =
	    FIELD_PREP(RCC_PLLCFGR_PLLM_MASK, pll->m) |
	    FIELD_PREP(RCC_PLLCFGR_PLLN_MASK, pll->n) |
	    FIELD_PREP(RCC_PLLCFGR_PLLP_MASK, (pll->p / 2U) - 1U) |
	    FIELD_PREP(RCC_PLLCFGR_PLLQ_MASK, pll->q) |
	    FIELD_PREP(RCC_PLLCFGR_PLLSRC,
	               (pll->src == RCC_PLL_SRC_HSE) ? 1U : 0U);

	reg_set_field(&RCC->CR, RCC_CR_PLLON,
	              0U); /* PLLCFGR needs the PLL off */
	if (!reg_poll(&RCC->CR, RCC_CR_PLLRDY, 0U, RCC_WAIT_LOOPS)) {
		return false;
	}
	reg_modify(&RCC->PLLCFGR, mask, value);
	reg_set_field(&RCC->CR, RCC_CR_PLLON, 1U);
	return reg_poll(&RCC->CR, RCC_CR_PLLRDY, RCC_CR_PLLRDY, RCC_WAIT_LOOPS);
}

/** Start whatever the configuration needs, and wait until it is ready. */
static bool rcc_start_clock(const RCC_clock_config_t* cfg)
{
	switch (cfg->sysclk) {
		case RCC_SYSCLK_HSI:
			return rcc_hsi_start();
		case RCC_SYSCLK_HSE:
			return rcc_hse_start(cfg->hse_bypass);
		case RCC_SYSCLK_PLL:
			if (cfg->pll.src == RCC_PLL_SRC_HSE) {
				if (!rcc_hse_start(cfg->hse_bypass)) {
					return false;
				}
			}
			else if (!rcc_hsi_start()) {
				return false;
			}
			/* If the PLL is running the core, step off it before reconfiguring. */
			if (FIELD_GET(RCC_CFGR_SWS_MASK, RCC->CFGR) ==
			        RCC_SW_PLL &&
			    !(rcc_hsi_start() &&
			      rcc_switch_sysclk(RCC_SW_HSI))) {
				return false;
			}
			return rcc_pll_start(&cfg->pll);
		default:
			return false;
	}
}

/** Cancel any pending switch, restore the saved dividers, report @p status. */
static RCC_status_t rcc_abort(uint32_t saved_cfgr, RCC_status_t status)
{
	const uint32_t mask =
	    RCC_CFGR_HPRE_MASK | RCC_CFGR_PPRE1_MASK | RCC_CFGR_PPRE2_MASK;

	/* Request the source that is running now (SWS), not the new one. */
	reg_set_field(&RCC->CFGR, RCC_CFGR_SW_MASK,
	              FIELD_GET(RCC_CFGR_SWS_MASK, RCC->CFGR));
	reg_modify(&RCC->CFGR, mask, saved_cfgr);
	return status;
}

/* ---- Clock initialisation -------------------------------------------- */

RCC_status_t RCC_clock_init(const RCC_clock_config_t* cfg)
{
	if (cfg == NULL) {
		return RCC_ERR_NULL_PTR;
	}

	rcc_freqs_t target;
	const RCC_status_t status = rcc_compute(cfg, &target);
	if (status != RCC_OK) {
		return status;
	}

	const uint32_t saved_cfgr = RCC->CFGR;
	const uint32_t sw = rcc_sw_encoding(cfg->sysclk);
	const uint32_t accel =
	    FLASH_ACR_PRFTEN | FLASH_ACR_ICEN | FLASH_ACR_DCEN;

	/* 1. Worst-case wait states first, then enable prefetch and caches. */
	if (!rcc_flash_set_latency(rcc_flash_latency(RCC_SYSCLK_MAX_HZ))) {
		return RCC_ERR_TIMEOUT;
	}
	reg_modify(&FLASH->ACR, accel, accel);

	/* 2. Slow both APB buses to the maximum divider during the transition. */
	rcc_set_apb_dividers(RCC_APB_DIV_16, RCC_APB_DIV_16);

	/* 3. Start the oscillator and the PLL, and wait until they are ready. */
	if (!rcc_start_clock(cfg)) {
		return rcc_abort(saved_cfgr, RCC_ERR_TIMEOUT);
	}

	/* 4. Final AHB divider (the buses are still on /16). */
	reg_set_field(&RCC->CFGR, RCC_CFGR_HPRE_MASK, (uint32_t)cfg->ahb_div);

	/* 5. Switch SYSCLK and wait until the hardware reports the new source. */
	if (!rcc_switch_sysclk(sw)) {
		return rcc_abort(saved_cfgr, RCC_ERR_TIMEOUT);
	}

	/* 6. Final APB dividers. */
	rcc_set_apb_dividers(cfg->apb1_div, cfg->apb2_div);

	/* 7. Settle on the lowest wait states for the real HCLK. */
	(void)rcc_flash_set_latency(rcc_flash_latency(target.hclk));

	rcc_hse_hz = cfg->hse_hz;
	return RCC_OK;
}

uint32_t RCC_get_sysclk_hz(void)
{
	switch (FIELD_GET(RCC_CFGR_SWS_MASK, RCC->CFGR)) {
		case RCC_SW_HSI: {
			return RCC_HSI_HZ;
		}
		case RCC_SW_HSE: {
			return rcc_hse_hz;
		}
		case RCC_SW_PLL: {
			return rcc_pll_hz_from_regs();
		}
		default: {
			return 0U;
		}
	}
}

uint32_t RCC_get_hclk_hz(void)
{
	return RCC_get_sysclk_hz() /
	       rcc_ahb_div_decode(FIELD_GET(RCC_CFGR_HPRE_MASK, RCC->CFGR));
}

uint32_t RCC_get_pclk1_hz(void)
{
	return rcc_pclk_hz(RCC_CFGR_PPRE1_MASK);
}

uint32_t RCC_get_pclk2_hz(void)
{
	return rcc_pclk_hz(RCC_CFGR_PPRE2_MASK);
}

RCC_status_t RCC_mco1_config(RCC_mco1_src_t src, RCC_mco_div_t div)
{
	const uint32_t s = (uint32_t)src;
	const uint32_t d = (uint32_t)div;

	if (s > (uint32_t)RCC_MCO1_PLL) {
		return RCC_ERR_INVALID_SOURCE;
	}
	if (d != (uint32_t)RCC_MCO_DIV_1 &&
	    (d < (uint32_t)RCC_MCO_DIV_2 || d > (uint32_t)RCC_MCO_DIV_5)) {
		return RCC_ERR_INVALID_PRESCALER;
	}

	/* One read-modify-write: source and divider change together. */
	reg_modify(&RCC->CFGR, RCC_CFGR_MCO1_MASK | RCC_CFGR_MCO1PRE_MASK,
	           FIELD_PREP(RCC_CFGR_MCO1_MASK, s) |
	               FIELD_PREP(RCC_CFGR_MCO1PRE_MASK, d));
	return RCC_OK;
}
