#ifndef STM32F401XX_SYSTIME_DRIVER_H
#define STM32F401XX_SYSTIME_DRIVER_H

#include <stdbool.h>
#include <stdint.h>

/**
 * @file stm32f401xx_SYSTIME_driver.h
 * @brief Millisecond time base, delays and timeouts built on SysTick.
 *
 * All times are 32-bit milliseconds since ::SYSTIME_init and wrap after
 * about 49.7 days. Compare times only through the functions here; never
 * compare two absolute values directly.
 */

/**
 * @defgroup systime_driver Systime service
 * @brief Time since start, blocking delay and non-blocking timeouts.
 * @{
 */

/** Tick rate of the time base in Hz (one tick is one millisecond). */
#define SYSTIME_TICK_HZ 1000U

/** Result of a systime call. 0 means success. */
typedef enum {
    SYSTIME_OK            = 0, /**< Success. */
    SYSTIME_ERR_TICK_RATE = 1  /**< SysTick could not provide ::SYSTIME_TICK_HZ at this core clock. */
} SYSTIME_status_t;

/** Periodic software timer. Fill with ::SYSTIME_timer_start. */
typedef struct {
    uint32_t next;   /**< Time of the next expiry, in ms. */
    uint32_t period; /**< Period in ms. */
} SYSTIME_timer_t;

/**
 * @brief Start the time base and register it with SysTick.
 * @param[in] core_hz  HCLK in Hz.
 * @retval SYSTIME_OK            Success.
 * @retval SYSTIME_ERR_TICK_RATE SysTick refused the rate.
 * @warning Call again after every clock change. The counter keeps its value.
 */
SYSTIME_status_t SYSTIME_init(uint32_t core_hz);

/**
 * @brief Milliseconds since ::SYSTIME_init, modulo 2^32.
 * @note Atomic read; safe from ISRs.
 */
uint32_t SYSTIME_ms(void);

/**
 * @brief Milliseconds elapsed since @p start_ms, wrap-safe.
 * @param[in] start_ms  A value earlier returned by ::SYSTIME_ms.
 */
uint32_t SYSTIME_since(uint32_t start_ms);

/**
 * @brief Test whether @p timeout_ms has passed since @p start_ms.
 * @param[in] start_ms    A value earlier returned by ::SYSTIME_ms.
 * @param[in] timeout_ms  Timeout in ms.
 * @return true once at least @p timeout_ms have elapsed.
 */
bool SYSTIME_elapsed(uint32_t start_ms, uint32_t timeout_ms);

/**
 * @brief Block for at least @p ms milliseconds.
 *
 * Waits one extra tick so the delay is never shorter than requested.
 *
 * @param[in] ms  Delay in ms.
 * @warning Never call from the SysTick interrupt or from an interrupt of
 *          equal or higher priority: the tick cannot advance there.
 */
void SYSTIME_delay_ms(uint32_t ms);

/**
 * @brief Start a periodic timer.
 * @param[out] t          Timer to initialise.
 * @param[in]  period_ms  Period in ms.
 */
void SYSTIME_timer_start(SYSTIME_timer_t *t, uint32_t period_ms);

/**
 * @brief Test and advance a periodic timer.
 *
 * Moves the next expiry forward by exactly one period when it fires, so the
 * average rate does not drift.
 *
 * @param[in,out] t  Timer started with ::SYSTIME_timer_start.
 * @return true once per period.
 */
bool SYSTIME_timer_expired(SYSTIME_timer_t *t);

/** @} */ /* end of systime_driver */

#endif
