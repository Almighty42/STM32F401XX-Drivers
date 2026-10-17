#ifndef STM32F401XX_RCC_DRIVER_H
#define STM32F401XX_RCC_DRIVER_H

#include "stm32f401xx.h"
#include <stdbool.h>
#include <stdint.h>

/**
 * @file stm32f401xx_RCC_driver.h
 * @brief Clock tree driver for the STM32F401xD/xE: oscillators, PLL and
 *        bus prescalers.
 *
 * Assumes a 2.7 V to 3.6 V supply (flash wait states are chosen for it).
 * Peripheral clock gating stays in each peripheral driver.
 */

/**
 * @defgroup rcc_driver RCC clock driver
 * @brief System clock selection, PLL set-up and clock frequency queries.
 * @{
 */

/** @name Limits (see RM0368)
 *  @{ */
#define RCC_HSI_HZ         16000000U /**< Internal oscillator.     */
#define RCC_SYSCLK_MAX_HZ  84000000U /**< Maximum SYSCLK / HCLK.   */
#define RCC_APB1_MAX_HZ    42000000U /**< Maximum APB1 clock.      */
#define RCC_APB2_MAX_HZ    84000000U /**< Maximum APB2 clock.      */
/** @} */

/** System clock source. */
typedef enum {
    RCC_SYSCLK_HSI, /**< Internal 16 MHz oscillator. */
    RCC_SYSCLK_HSE, /**< External oscillator or bypass clock. */
    RCC_SYSCLK_PLL  /**< Main PLL output. */
} RCC_sysclk_src_t;

/** PLL input clock. */
typedef enum {
    RCC_PLL_SRC_HSI, /**< HSI. */
    RCC_PLL_SRC_HSE  /**< HSE. */
} RCC_pll_src_t;

/** AHB prescaler. Values equal the HPRE register encoding. */
typedef enum {
    RCC_AHB_DIV_1   = 0x0U, /**< HCLK = SYSCLK.       */
    RCC_AHB_DIV_2   = 0x8U, /**< HCLK = SYSCLK / 2.   */
    RCC_AHB_DIV_4   = 0x9U, /**< HCLK = SYSCLK / 4.   */
    RCC_AHB_DIV_8   = 0xAU, /**< HCLK = SYSCLK / 8.   */
    RCC_AHB_DIV_16  = 0xBU, /**< HCLK = SYSCLK / 16.  */
    RCC_AHB_DIV_64  = 0xCU, /**< HCLK = SYSCLK / 64.  */
    RCC_AHB_DIV_128 = 0xDU, /**< HCLK = SYSCLK / 128. */
    RCC_AHB_DIV_256 = 0xEU, /**< HCLK = SYSCLK / 256. */
    RCC_AHB_DIV_512 = 0xFU  /**< HCLK = SYSCLK / 512. */
} RCC_ahb_div_t;

/** APB prescaler. Values equal the PPRE1/PPRE2 register encoding. */
typedef enum {
    RCC_APB_DIV_1  = 0x0U, /**< PCLK = HCLK.      */
    RCC_APB_DIV_2  = 0x4U, /**< PCLK = HCLK / 2.  */
    RCC_APB_DIV_4  = 0x5U, /**< PCLK = HCLK / 4.  */
    RCC_APB_DIV_8  = 0x6U, /**< PCLK = HCLK / 8.  */
    RCC_APB_DIV_16 = 0x7U  /**< PCLK = HCLK / 16. */
} RCC_apb_div_t;

/** MCO1 output source. Values equal the MCO1 register encoding. */
typedef enum {
    RCC_MCO1_HSI = 0U, /**< HSI.        */
    RCC_MCO1_LSE = 1U, /**< LSE.        */
    RCC_MCO1_HSE = 2U, /**< HSE.        */
    RCC_MCO1_PLL = 3U  /**< PLL output. */
} RCC_mco1_src_t;

/** MCO1 output divider. Values equal the MCO1PRE register encoding. */
typedef enum {
    RCC_MCO_DIV_1 = 0U, /**< No division. */
    RCC_MCO_DIV_2 = 4U, /**< Divide by 2. */
    RCC_MCO_DIV_3 = 5U, /**< Divide by 3. */
    RCC_MCO_DIV_4 = 6U, /**< Divide by 4. */
    RCC_MCO_DIV_5 = 7U  /**< Divide by 5. */
} RCC_mco_div_t;

/** Result of an RCC driver call. 0 means success; all errors are non-zero. */
typedef enum {
    RCC_OK                    = 0, /**< Success. */
    RCC_ERR_NULL_PTR          = 1, /**< A required pointer argument was NULL. */
    RCC_ERR_INVALID_PLL       = 2, /**< M, N, P or Q out of range. */
    RCC_ERR_INVALID_PRESCALER = 3, /**< Prescaler is not an enum value. */
    RCC_ERR_FREQ_LIMIT        = 4, /**< SYSCLK, APB1 or APB2 above its limit. */
    RCC_ERR_INVALID_SOURCE    = 5, /**< Source not usable, e.g. HSE with hse_hz 0. */
    RCC_ERR_TIMEOUT           = 6  /**< Oscillator, PLL or clock switch did not become ready. */
} RCC_status_t;

/** PLL settings. VCO = source / m * n; SYSCLK = VCO / p. */
typedef struct {
    RCC_pll_src_t src; /**< PLL input clock. */
    uint8_t       m;   /**< Input divider, 2..63. Source / m must be 1 to 2 MHz. */
    uint16_t      n;   /**< VCO multiplier, 50..432. VCO must be 192 to 432 MHz. */
    uint8_t       p;   /**< SYSCLK divider: 2, 4, 6 or 8 (the value, not the encoding). */
    uint8_t       q;   /**< 48 MHz domain divider, 2..15. VCO / q must not exceed 75 MHz. */
} RCC_pll_config_t;

/** Complete clock configuration. */
typedef struct {
    RCC_sysclk_src_t sysclk;      /**< Source of SYSCLK. */
    RCC_pll_config_t pll;         /**< Read only when sysclk is ::RCC_SYSCLK_PLL or the PLL is fed. */
    uint32_t         hse_hz;      /**< HSE frequency in Hz; 0 when HSE is unused. */
    bool             hse_bypass;  /**< true if HSE is an external clock, not a crystal. */
    RCC_ahb_div_t    ahb_div;     /**< AHB prescaler. */
    RCC_apb_div_t    apb1_div;    /**< APB1 prescaler. */
    RCC_apb_div_t    apb2_div;    /**< APB2 prescaler. */
} RCC_clock_config_t;

/**
 * @brief Apply a complete clock configuration.
 *
 * Validates every limit first, then runs a fixed sequence: flash wait
 * states and caches, bus dividers to maximum, oscillator, PLL, switch,
 * final dividers, final wait states. Waits are bounded by a loop count,
 * not by time.
 * @note SYSCLK is either unchanged or HSI on failure (HSI only when the PLL
 *       was in use). Dividers are restored; flash wait states may stay high.
 * @pre  Not called from an ISR.
 *
 * @param[in] cfg  Configuration. Not modified.
 * @retval RCC_OK                    Success; the new clock is running.
 * @retval RCC_ERR_NULL_PTR          @p cfg is NULL.
 * @retval RCC_ERR_INVALID_PLL       PLL parameters out of range.
 * @retval RCC_ERR_INVALID_PRESCALER A prescaler is not an enum value.
 * @retval RCC_ERR_FREQ_LIMIT        A resulting frequency exceeds its limit.
 * @retval RCC_ERR_INVALID_SOURCE    The chosen source cannot be used.
 * @retval RCC_ERR_TIMEOUT           Something did not become ready in time.
 * @note A failed call leaves the previous clock running.
 * @warning Peripherals that derive timing from the clock (SysTick, UART
 *          baud rate, timers) must be reconfigured afterwards.
 */
RCC_status_t RCC_clock_init(const RCC_clock_config_t *cfg);

/** @brief Current SYSCLK in Hz, computed from the RCC registers. */
uint32_t RCC_get_sysclk_hz(void);

/** @brief Current HCLK (AHB, core, SysTick) in Hz. */
uint32_t RCC_get_hclk_hz(void);

/** @brief Current APB1 peripheral clock in Hz. */
uint32_t RCC_get_pclk1_hz(void);

/** @brief Current APB2 peripheral clock in Hz. */
uint32_t RCC_get_pclk2_hz(void);

/**
 * @brief Route a clock to the MCO1 pin (PA8).
 * @note The selected source must already be running. 
 * Divide the output (e.g. ::RCC_MCO_DIV_5) at high frequencies: the pin cannot output 84 MHz cleanly.
 * @param[in] src  Clock to output.
 * @param[in] div  Output divider.
 * @retval RCC_OK                    Success.
 * @retval RCC_ERR_INVALID_SOURCE    @p src is not an enum value.
 * @retval RCC_ERR_INVALID_PRESCALER @p div is not an enum value.
 * @note The caller configures PA8 as alternate function 0 with the GPIO driver.
 */
RCC_status_t RCC_mco1_config(RCC_mco1_src_t src, RCC_mco_div_t div);

/** @} */ /* end of rcc_driver */

#endif
