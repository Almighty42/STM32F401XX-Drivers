#ifndef STM32F401XX_SYSTICK_DRIVER_H
#define STM32F401XX_SYSTICK_DRIVER_H

#include <stdint.h>

/**
 * @file stm32f401xx_SYSTICK_driver.h
 * @brief Cortex-M4 SysTick driver: a periodic interrupt at a chosen rate.
 *
 * The timer runs from the processor clock (HCLK). The driver does not read
 * the clock tree: the caller passes the frequency. It does not set the
 * exception priority either (SCB SHPR3).
 */

/**
 * @defgroup systick_driver SysTick driver
 * @brief Periodic tick generation with an optional per-tick callback.
 * @{
 */

/** Largest reload value the 24-bit counter accepts. */
#define SYSTICK_RELOAD_MAX 0x00FFFFFFU

/** Result of a SysTick driver call. 0 means success; all errors are non-zero. */
typedef enum {
    SYSTICK_OK               = 0, /**< Success. */
    SYSTICK_ERR_INVALID_RATE = 1, /**< core_hz or tick_hz is zero, or tick_hz above core_hz. */
    SYSTICK_ERR_RELOAD_RANGE = 2  /**< Needed reload value does not fit in 24 bits. */
} SYSTICK_status_t;

/**
 * Per-tick callback, called from interrupt context.
 * @param[in] ctx  Pointer given at registration; may be NULL.
 */
typedef void (*SYSTICK_callback_t)(void *ctx);

/**
 * @brief Start the tick at @p tick_hz.
 *
 * Reload is @c core_hz / tick_hz - 1. Enables the counter and its
 * interrupt, clocked from HCLK.
 *
 * @param[in] core_hz  HCLK in Hz, for example from ::RCC_get_hclk_hz.
 * @param[in] tick_hz  Wanted tick rate in Hz, for example 1000.
 * @retval SYSTICK_OK               Success.
 * @retval SYSTICK_ERR_INVALID_RATE A rate is zero, or tick_hz exceeds core_hz.
 * @retval SYSTICK_ERR_RELOAD_RANGE The reload value does not fit in 24 bits.
 * @note A failed call leaves SysTick unchanged.
 * @warning Call again after every clock change.
 */
SYSTICK_status_t SYSTICK_init(uint32_t core_hz, uint32_t tick_hz);

/** @brief Stop the counter and its interrupt. The callback stays registered. */
void SYSTICK_deinit(void);

/**
 * @brief Register the per-tick callback, or remove it.
 * @param[in] cb   Callback, or NULL to remove.
 * @param[in] ctx  Passed to @p cb.
 */
void SYSTICK_set_callback(SYSTICK_callback_t cb, void *ctx);

/**
 * @brief Handle one tick. Call from the SysTick exception handler.
 *
 * Example:
 * @code
 * void SysTick_Handler(void) { SYSTICK_irq_handler(); }
 * @endcode
 */
void SYSTICK_irq_handler(void);

/** @} */ /* end of systick_driver */

#endif
