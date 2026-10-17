/**
 * @file stm32f401xx_RCC_driver.c
 * @brief Implementation of the RCC clock driver. The API is documented in
 * the header.
 */

#include "stm32f401xx_RCC_driver.h"
#include "reg_utils.h"
#include <stddef.h>

#define RCC_WAIT_LOOPS 100000U
#define RCC_FLASH_WS_STEP_HZ 30000000U
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

static RCC_status_t rcc_pll_output(const RCC_pll_config_t* pll, uint32_t src_hz,
                                   uint32_t* out_hz)
{
	if (pll->m < 2U || pll->m > 63U || pll->n < 50U || pll->n > 432U ||
	    pll->q < 2U || pll->q > 15U) {
		return RCC_ERR_INVALID_PLL;
	}
	if (pll->p != 2U && pll->p != 4U && pll->p != 6U && pll->p != 8U) {
		return RCC_ERR_INVALID_PLL;
	}
	/* VCO input = src / m must be 1 to 2 MHz (compared without dividing) */
	if (src_hz < 1000000U * pll->m || src_hz > 2000000U * pll->m) {
		return RCC_ERR_INVALID_PLL;
	}

	const uint32_t vco_out = (uint32_t)((uint64_t)src_hz * pll->n / pll->m);
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

// static bool rcc_flash_set_latency(uint32_t ws)
// {
// 	reg_set_field(&FLASH->ACR, FLASH_ACR_LATENCY_MASK, ws);
// 	return reg_poll(&FLASH->ACR, FLASH_ACR_LATENCY_MASK,
// 	                FIELD_PREP(FLASH_ACR_LATENCY_MASK, ws), RCC_WAIT_LOOPS);
// }

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

	/* 1. Worst-case wait states first, then enable prefetch and caches. */
	if (!rcc_flash_set_latency(rcc_flash_latency(RCC_SYSCLK_MAX_HZ))) {
		return RCC_ERR_TIMEOUT;
	}

	return RCC_OK;
}

uint32_t RCC_get_sysclk_hz(void)
{
	return 0U;
}

uint32_t RCC_get_hclk_hz(void)
{
	return 0U;
}

uint32_t RCC_get_pclk1_hz(void)
{
	return 0U;
}

uint32_t RCC_get_pclk2_hz(void)
{
	return 0U;
}

RCC_status_t RCC_mco1_config(RCC_mco1_src_t src, RCC_mco_div_t div)
{
	(void)src;
	(void)div;
	return RCC_OK;
}
