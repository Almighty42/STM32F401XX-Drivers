#ifndef STM32F401XX_GPIO_DRIVER_H
#define STM32F401XX_GPIO_DRIVER_H

#include <stdbool.h>
#include <stdint.h>
#include "stm32f401xx.h"

/**
 * @file stm32f401xx_GPIO_driver.h
 * @brief GPIO driver for the STM32F401xD/xE.
 *
 * Pins are always passed as bit masks (::GPIO_PIN_0 ... ::GPIO_PIN_15),
 * never as pin indices. The driver does not touch the NVIC: enable and
 * prioritise the EXTI IRQ yourself (see ::GPIO_exti_irq_handler).
 */

/**
 * @defgroup gpio_driver GPIO driver
 * @brief Pin configuration, data access and EXTI for GPIO ports A-E and H.
 * @{
 */

/** @name Pin masks
 *  Bit n selects pin n. Combine with `|`. Never pass a pin index.
 *  @{ */
#define GPIO_PIN_0    ((uint16_t)(1U << 0))   /**< Pin 0 mask.  */
#define GPIO_PIN_1    ((uint16_t)(1U << 1))   /**< Pin 1 mask.  */
#define GPIO_PIN_2    ((uint16_t)(1U << 2))   /**< Pin 2 mask.  */
#define GPIO_PIN_3    ((uint16_t)(1U << 3))   /**< Pin 3 mask.  */
#define GPIO_PIN_4    ((uint16_t)(1U << 4))   /**< Pin 4 mask.  */
#define GPIO_PIN_5    ((uint16_t)(1U << 5))   /**< Pin 5 mask.  */
#define GPIO_PIN_6    ((uint16_t)(1U << 6))   /**< Pin 6 mask.  */
#define GPIO_PIN_7    ((uint16_t)(1U << 7))   /**< Pin 7 mask.  */
#define GPIO_PIN_8    ((uint16_t)(1U << 8))   /**< Pin 8 mask.  */
#define GPIO_PIN_9    ((uint16_t)(1U << 9))   /**< Pin 9 mask.  */
#define GPIO_PIN_10   ((uint16_t)(1U << 10))  /**< Pin 10 mask. */
#define GPIO_PIN_11   ((uint16_t)(1U << 11))  /**< Pin 11 mask. */
#define GPIO_PIN_12   ((uint16_t)(1U << 12))  /**< Pin 12 mask. */
#define GPIO_PIN_13   ((uint16_t)(1U << 13))  /**< Pin 13 mask. */
#define GPIO_PIN_14   ((uint16_t)(1U << 14))  /**< Pin 14 mask. */
#define GPIO_PIN_15   ((uint16_t)(1U << 15))  /**< Pin 15 mask. */
#define GPIO_PIN_ALL  ((uint16_t)0xFFFFU)     /**< All 16 pins of a port. */
/** @} */

/** Pin function. Values equal the MODER register encoding. */
typedef enum {
    GPIO_MODE_INPUT  = 0U, /**< Digital input. Also used for EXTI pins. */
    GPIO_MODE_OUTPUT = 1U, /**< General-purpose output. */
    GPIO_MODE_AF     = 2U, /**< Alternate function, selected by ::GPIO_af_t. */
    GPIO_MODE_ANALOG = 3U  /**< Analog mode, digital input buffer disabled. */
} GPIO_mode_t;

/** Output driver type. Values equal the OTYPER bit. Ignored for inputs. */
typedef enum {
    GPIO_OTYPE_PUSH_PULL  = 0U, /**< Push-pull output stage. */
    GPIO_OTYPE_OPEN_DRAIN = 1U  /**< Open-drain output stage. */
} GPIO_otype_t;


/** Output slew rate. Values equal the OSPEEDR encoding. Higher is faster
 *  and noisier. Actual rise time depends on VDD and load, see datasheet. */
typedef enum {
    GPIO_SPEED_LOW    = 0U, /**< Low speed.       */
    GPIO_SPEED_MEDIUM = 1U, /**< Medium speed.    */
    GPIO_SPEED_FAST   = 2U, /**< Fast speed.      */
    GPIO_SPEED_HIGH   = 3U  /**< Very high speed. */
} GPIO_speed_t;

/** Internal pull resistor. Values equal the PUPDR encoding (3 is reserved). */
typedef enum {
    GPIO_PULL_NONE = 0U, /**< Floating. */
    GPIO_PULL_UP   = 1U, /**< Pull-up.  */
    GPIO_PULL_DOWN = 2U  /**< Pull-down. */
} GPIO_pull_t;

/** Edge that raises an EXTI interrupt on an input pin. These are software
 *  values; the driver maps them to the EXTI RTSR and FTSR bits. */
typedef enum {
    GPIO_TRIG_NONE = 0U, /**< No interrupt; EXTI line masked. */
    GPIO_TRIG_RISING,    /**< Rising edge only.   */
    GPIO_TRIG_FALLING,   /**< Falling edge only.  */
    GPIO_TRIG_BOTH       /**< Both edges.         */
} GPIO_trigger_t;

/**
 * Alternate function number (AFRL/AFRH 4-bit field).
 * Names follow the column groups of Table 9 in the STM32F401xD/xE datasheet.
 * The AF number for a given signal depends on the pin: always check Table 9
 * for the exact pin. Several names may share one value on purpose.
 */
typedef enum {
    GPIO_AF0_SYS       = 0U,   /**< SWD/JTAG, MCO, RTC, TRACE. */
    GPIO_AF1_TIM1      = 1U,   /**< TIM1.  */
    GPIO_AF1_TIM2      = 1U,   /**< TIM2.  */
    GPIO_AF2_TIM3      = 2U,   /**< TIM3.  */
    GPIO_AF2_TIM4      = 2U,   /**< TIM4.  */
    GPIO_AF2_TIM5      = 2U,   /**< TIM5.  */
    GPIO_AF3_TIM9      = 3U,   /**< TIM9.  */
    GPIO_AF3_TIM10     = 3U,   /**< TIM10. */
    GPIO_AF3_TIM11     = 3U,   /**< TIM11. */
    GPIO_AF4_I2C1      = 4U,   /**< I2C1.  */
    GPIO_AF4_I2C2      = 4U,   /**< I2C2.  */
    GPIO_AF4_I2C3      = 4U,   /**< I2C3 (some pins use AF9, see below). */
    GPIO_AF5_SPI1      = 5U,   /**< SPI1.  */
    GPIO_AF5_SPI2      = 5U,   /**< SPI2.  */
    GPIO_AF5_SPI3      = 5U,   /**< SPI3 (some pins use AF6/AF7). */
    GPIO_AF5_SPI4      = 5U,   /**< SPI4.  */
    GPIO_AF5_I2S2      = 5U,   /**< I2S2.  */
    GPIO_AF5_I2S3      = 5U,   /**< I2S3.  */
    GPIO_AF6_SPI2      = 6U,   /**< SPI2 on pins that use AF6. */
    GPIO_AF6_SPI3      = 6U,   /**< SPI3 on pins that use AF6. */
    GPIO_AF6_I2S2      = 6U,   /**< I2S2 on pins that use AF6. */
    GPIO_AF6_I2S3      = 6U,   /**< I2S3 on pins that use AF6. */
    GPIO_AF7_SPI3      = 7U,   /**< SPI3 on pins that use AF7. */
    GPIO_AF7_I2S3      = 7U,   /**< I2S3 on pins that use AF7. */
    GPIO_AF7_USART1    = 7U,   /**< USART1. */
    GPIO_AF7_USART2    = 7U,   /**< USART2. */
    GPIO_AF8_USART6    = 8U,   /**< USART6. */
    GPIO_AF9_I2C2      = 9U,   /**< I2C2 on pins that use AF9. */
    GPIO_AF9_I2C3      = 9U,   /**< I2C3 on pins that use AF9. */
    GPIO_AF10_OTG1_FS  = 10U,  /**< USB OTG FS. */
    GPIO_AF12_SDIO     = 12U,  /**< SDIO.  */
    GPIO_AF15_EVENTOUT = 15U,  /**< EVENTOUT. */
    GPIO_AF_NONE       = 0xFFU /**< Not an AF. Rejected when mode is ::GPIO_MODE_AF. */
} GPIO_af_t;

/** Result of a GPIO driver call. 0 means success; all errors are non-zero. */
typedef enum {
    GPIO_OK                  = 0,  /**< Success. */
    GPIO_ERR_NULL_PTR        = 1,  /**< A required pointer argument was NULL. */
    GPIO_ERR_INVALID_PORT    = 2,  /**< Port is not GPIOA..GPIOE or GPIOH. */
    GPIO_ERR_INVALID_PINS    = 3,  /**< Pin mask is zero. */
    GPIO_ERR_INVALID_MODE    = 4,  /**< Mode is not a ::GPIO_mode_t value. */
    GPIO_ERR_INVALID_OTYPE   = 5,  /**< Output type out of range. */
    GPIO_ERR_INVALID_SPEED   = 6,  /**< Speed out of range. */
    GPIO_ERR_INVALID_PULL    = 7,  /**< Pull setting out of range. */
    GPIO_ERR_INVALID_AF      = 8,  /**< AF above 15, or AF_NONE in AF mode. */
    GPIO_ERR_INVALID_TRIGGER = 9,  /**< Trigger out of range, or used on a non-input pin. */
    GPIO_ERR_EXTI_LINE_BUSY  = 10, /**< EXTI line already routed to another port. */
    GPIO_ERR_EXTI_NOT_SINGLE = 11, /**< EXTI needs exactly one pin in the mask. */
    GPIO_ERR_PIN_LOCKED      = 12, /**< A selected pin is locked until reset. */
    GPIO_ERR_LOCK_FAILED     = 13  /**< Lock sequence failed or cannot be applied. */
} GPIO_status_t;

/** Configuration applied to every pin selected in @c pins. */
typedef struct {
    uint16_t       pins;    /**< Mask of GPIO_PIN_x. Must be non-zero. */
    GPIO_mode_t    mode;    /**< Pin function. */
    GPIO_otype_t   otype;   /**< Output type. Read only in output and AF modes. */
    GPIO_speed_t   speed;   /**< Output speed. Read only in output and AF modes. */
    GPIO_pull_t    pull;    /**< Pull resistor. */
    GPIO_af_t      af;      /**< Read only when @c mode is ::GPIO_MODE_AF. */
    GPIO_trigger_t trigger; /**< Read only when @c mode is ::GPIO_MODE_INPUT. */
} GPIO_config_t;

/**
 * EXTI callback, called from interrupt context.
 * @param[in] pin  Mask of the single pin that triggered (one GPIO_PIN_x).
 * @param[in] ctx  Pointer given at registration; may be NULL.
 */
typedef void (*GPIO_exti_callback_t)(uint16_t pin, void *ctx);

/** @name Clock and initialisation
 *  @{ */

/**
 * @brief Enable or disable the AHB1 clock of a GPIO port.
 * @param[in] port    GPIOA..GPIOE or GPIOH.
 * @param[in] enable  true to enable, false to disable.
 * @retval GPIO_OK                 Done.
 * @retval GPIO_ERR_INVALID_PORT   @p port is not a valid port.
 * @note Disabling the clock of a port stops every pin on that port from
 *       working in this driver, including other drivers' pins.
 */
GPIO_status_t GPIO_peri_clk_control(GPIO_TypeDef *port, bool enable);

/**
 * @brief Configure the pins selected in @p cfg->pins.
 *
 * Enables the port clock, then applies output type, speed, pull and
 * alternate function, and writes the mode last so the pin does not glitch.
 * If @p cfg->trigger is not ::GPIO_TRIG_NONE, routes the pin to its EXTI
 * line and unmasks it. The NVIC is not touched.
 *
 * @param[in] port  GPIOA..GPIOE or GPIOH.
 * @param[in] cfg   Configuration. Not modified.
 * @retval GPIO_OK  Success.
 * @retval GPIO_ERR_NULL_PTR        @p port or @p cfg is NULL.
 * @retval GPIO_ERR_INVALID_PORT    Port is not valid.
 * @retval GPIO_ERR_INVALID_PINS    Pin mask is zero.
 * @retval GPIO_ERR_INVALID_MODE    Mode out of range.
 * @retval GPIO_ERR_INVALID_OTYPE   Output type out of range.
 * @retval GPIO_ERR_INVALID_SPEED   Speed out of range.
 * @retval GPIO_ERR_INVALID_PULL    Pull out of range.
 * @retval GPIO_ERR_INVALID_AF      AF invalid for AF mode.
 * @retval GPIO_ERR_INVALID_TRIGGER Trigger invalid, or set on a non-input pin.
 * @retval GPIO_ERR_EXTI_NOT_SINGLE Trigger set with more than one pin.
 * @retval GPIO_ERR_EXTI_LINE_BUSY  EXTI line already routed to another port.
 * @retval GPIO_ERR_PIN_LOCKED      A selected pin is locked until reset.
 * @pre  Not called from an ISR that also configures the same port.
 * @note Configuration is read-modify-write on several registers and is not
 *       safe against concurrent configuration of the same port.
 */
GPIO_status_t GPIO_init(GPIO_TypeDef *port, const GPIO_config_t *cfg);

/**
 * @brief Return the selected pins to their reset state.
 *
 * Also masks the pins' EXTI lines. Reset state is per pin: SWD/JTAG pins
 * (for example PA13, PA14, PA15, PB3, PB4) reset to alternate function.
 *
 * @param[in] port  GPIOA..GPIOE or GPIOH.
 * @param[in] pins  Mask of pins to reset. Must be non-zero.
 * @retval GPIO_OK, GPIO_ERR_NULL_PTR, GPIO_ERR_INVALID_PORT, GPIO_ERR_INVALID_PINS, GPIO_ERR_PIN_LOCKED
 */
GPIO_status_t GPIO_pin_deinit(GPIO_TypeDef *port, uint16_t pins);

/**
 * @brief Reset the whole port through RCC and gate its clock.
 * @warning Affects every pin on the port, including pins used by other
 *          drivers (for example UART TX on PA2).
 * @param[in] port  GPIOA..GPIOE or GPIOH.
 * @retval GPIO_OK, GPIO_ERR_NULL_PTR, GPIO_ERR_INVALID_PORT
 */
GPIO_status_t GPIO_port_deinit(GPIO_TypeDef *port);
/** @} */

/** @name Data access
 *  No argument validation is done here: these are hot-path functions and
 *  @p port must be a valid GPIO port with its clock enabled.
 *  @{ */

/**
 * @brief Drive the selected pins high.
 * @param[in] port  Valid GPIO port.
 * @param[in] pins  Mask of pins to set.
 * @note Atomic (single BSRR write). Safe from ISRs.
 */
void GPIO_set(GPIO_TypeDef *port, uint16_t pins);

/**
 * @brief Drive the selected pins low.
 * @param[in] port  Valid GPIO port.
 * @param[in] pins  Mask of pins to reset.
 * @note Atomic (single BSRR write). Safe from ISRs.
 */
void GPIO_reset(GPIO_TypeDef *port, uint16_t pins);

/**
 * @brief Invert the output level of the selected pins.
 * @param[in] port  Valid GPIO port.
 * @param[in] pins  Mask of pins to toggle.
 * @note Reads ODR, then writes BSRR. Atomic against other writers of
 *       different pins, but not against another writer of the same pins.
 */
void GPIO_toggle(GPIO_TypeDef *port, uint16_t pins);

/**
 * @brief Set the selected pins to the corresponding bits of @p value.
 * @param[in] port   Valid GPIO port.
 * @param[in] mask   Pins to change. Other pins are untouched.
 * @param[in] value  Bit n gives the new level of pin n (only bits in @p mask are used).
 * @note Atomic (single BSRR write). Safe from ISRs.
 */
void GPIO_write_pins(GPIO_TypeDef *port, uint16_t mask, uint16_t value);

/**
 * @brief Read the input level of the selected pins.
 * @param[in] port  Valid GPIO port.
 * @param[in] pins  Mask of pins to read.
 * @return Input data register masked with @p pins.
 */
uint16_t GPIO_read_pins(GPIO_TypeDef *port, uint16_t pins);

/**
 * @brief Read the level last written to the selected output pins.
 * @param[in] port  Valid GPIO port.
 * @param[in] pins  Mask of pins to read.
 * @return Output data register masked with @p pins.
 */
uint16_t GPIO_read_output_pins(GPIO_TypeDef *port, uint16_t pins);
/** @} */

/** @name EXTI
 *  @{ */

/**
 * @brief Register a callback for one EXTI line.
 * @param[in] pin  Exactly one GPIO_PIN_x.
 * @param[in] cb   Callback, or NULL to remove.
 * @param[in] ctx  Passed to @p cb.
 * @retval GPIO_OK
 * @retval GPIO_ERR_EXTI_NOT_SINGLE  @p pin is zero or has more than one bit set.
 */
GPIO_status_t GPIO_exti_set_callback(uint16_t pin, GPIO_exti_callback_t cb, void *ctx);

/**
 * @brief Unmask an EXTI line (enable its interrupt request).
 * @param[in] pin  Exactly one GPIO_PIN_x.
 * @retval GPIO_OK, GPIO_ERR_EXTI_NOT_SINGLE
 * @pre The line was routed with ::GPIO_init and a trigger other than ::GPIO_TRIG_NONE.
 */
GPIO_status_t GPIO_exti_enable(uint16_t pin);

/**
 * @brief Mask an EXTI line and clear its pending flag.
 * @param[in] pin  Exactly one GPIO_PIN_x.
 * @retval GPIO_OK, GPIO_ERR_EXTI_NOT_SINGLE
 */
GPIO_status_t GPIO_exti_disable(uint16_t pin);

/**
 * @brief Handle EXTI interrupts for a range of lines. Call from the ISR.
 *
 * For each pending and unmasked line in [first, last], clears the pending
 * flag and calls the registered callback. Example:
 * @code
 * void EXTI15_10_IRQHandler(void) { GPIO_exti_irq_handler(10, 15); }
 * @endcode
 *
 * @param[in] first  Lowest EXTI line, 0..15.
 * @param[in] last   Highest EXTI line, @p first..15.
 * @pre The EXTI IRQ is enabled and prioritised in the NVIC by the caller.
 */
void GPIO_exti_irq_handler(uint8_t first, uint8_t last);
/** @} */

/** @name Locking
 *  @{ */

/**
 * @brief Lock the configuration of the selected pins until the next reset.
 * @param[in] port  GPIOA..GPIOE or GPIOH.
 * @param[in] pins  Mask of pins to lock. Must be non-zero.
 * @retval GPIO_OK
 * @retval GPIO_ERR_INVALID_PORT, GPIO_ERR_INVALID_PINS, GPIO_ERR_NULL_PTR
 * @retval GPIO_ERR_LOCK_FAILED  The LCKR sequence did not complete.
 * @warning Locked pins cannot be reconfigured by software until reset.
 */
GPIO_status_t GPIO_lock_pins(GPIO_TypeDef *port, uint16_t pins);
/** @} */

_Static_assert(GPIO_MODE_INPUT  == 0U && GPIO_MODE_ANALOG == 3U, "MODER encoding");
_Static_assert(GPIO_SPEED_HIGH == 3U,   "OSPEEDR encoding");
_Static_assert(GPIO_PULL_DOWN  == 2U,   "PUPDR encoding");
_Static_assert(GPIO_OTYPE_OPEN_DRAIN == 1U, "OTYPER encoding");
_Static_assert(GPIO_AF15_EVENTOUT == 15U,  "AFR field is 4 bits");

/** @} */ /* end of gpio_driver */

#endif
