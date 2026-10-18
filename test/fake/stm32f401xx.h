#ifndef FAKE_STM32F401XX_H
#define FAKE_STM32F401XX_H

/* Host-test replacement for the device header: the real register structs and
 * types come from the generic header, but every peripheral pointer is
 * redirected to a plain variable that each test defines. */

#include "../../generic/stm32f401xx.h"

/* ---- Fake peripherals (defined in the test files) -------------------- */

extern GPIO_TypeDef  fake_gpio[6]; /* A, B, C, D, E, H */
extern RCC_TypeDef   fake_rcc;
extern FLASH_TypeDef fake_flash;
extern EXTI_Type     fake_exti;
extern SYSCFG_Type   fake_syscfg;
extern SYSTICK_TypeDef fake_systick;

/* ---- Redirect the real addresses to the fakes ------------------------ */

#undef GPIOA
#undef GPIOB
#undef GPIOC
#undef GPIOD
#undef GPIOE
#undef GPIOH
#undef RCC
#undef FLASH
#undef EXTI
#undef SYSCFG
#undef SYSTICK

#define GPIOA   (&fake_gpio[0])
#define GPIOB   (&fake_gpio[1])
#define GPIOC   (&fake_gpio[2])
#define GPIOD   (&fake_gpio[3])
#define GPIOE   (&fake_gpio[4])
#define GPIOH   (&fake_gpio[5])
#define RCC     (&fake_rcc)
#define FLASH   (&fake_flash)
#define EXTI    (&fake_exti)
#define SYSCFG  (&fake_syscfg)
#define SYSTICK (&fake_systick)

#endif
