/**
 * @file stm32f401xx_GPIO_regs.h
 * @brief Private register-field masks for the GPIO driver (RM0368).
 *
 * n is a pin number 0..15, line is an EXTI line 0..15.
 */

#ifndef STM32F401XX_GPIO_REGS_H
#define STM32F401XX_GPIO_REGS_H

#include "reg_utils.h"

#define GPIO_PINS_PER_PORT  16U

/* 2 bits per pin */
#define GPIO_MODER_MASK(n)    GENMASK(2U * (n) + 1U, 2U * (n))
#define GPIO_OSPEEDR_MASK(n)  GENMASK(2U * (n) + 1U, 2U * (n))
#define GPIO_PUPDR_MASK(n)    GENMASK(2U * (n) + 1U, 2U * (n))

/* 1 bit per pin */
#define GPIO_OTYPER_MASK(n)   BIT(n)

/* 4 bits per pin, 8 pins per register (AFRL: pins 0-7, AFRH: pins 8-15) */
#define GPIO_AFR_MASK(n)      GENMASK(4U * ((n) % 8U) + 3U, 4U * ((n) % 8U))

/* LCKR: bits 15:0 are the per-pin locks, bit 16 is the lock key */
#define GPIO_LCKR_LCK_MASK    GENMASK(15, 0)

/* BSRR: BSy (set) in bits 15:0, BRy (reset) in bits 31:16 */
#define GPIO_BSRR_BS_MASK     GENMASK(15, 0)
#define GPIO_BSRR_BR_MASK     GENMASK(31, 16)
#define GPIO_BSRR_SET(pins)   FIELD_PREP(GPIO_BSRR_BS_MASK, (pins))
#define GPIO_BSRR_RESET(pins) FIELD_PREP(GPIO_BSRR_BR_MASK, (pins))

/* SYSCFG EXTICR: four 4-bit port codes per register */
#define EXTICR_MASK(line)     GENMASK(4U * ((line) % 4U) + 3U, 4U * ((line) % 4U))

#define GPIO_LCKR_LCKK_MASK   BIT(16)     /* lock key, bit 16 of LCKR */

/* Compile-time checks of the layout */
_Static_assert(GPIO_MODER_MASK(14)  == 0x30000000U, "MODER pin 14");
_Static_assert(GPIO_OTYPER_MASK(5)  == 0x00000020U, "OTYPER pin 5");
_Static_assert(GPIO_AFR_MASK(10)    == 0x00000F00U, "AFRH pin 10");
_Static_assert(EXTICR_MASK(13)      == 0x000000F0U, "EXTICR line 13");
_Static_assert(GPIO_BSRR_SET(0x20U)   == 0x00000020U, "BSRR set");
_Static_assert(GPIO_BSRR_RESET(0x20U) == 0x00200000U, "BSRR reset");
_Static_assert(GPIO_LCKR_LCKK_MASK == 0x00010000U, "LCKR lock key");

#endif /* STM32F401XX_GPIO_REGS_H */
