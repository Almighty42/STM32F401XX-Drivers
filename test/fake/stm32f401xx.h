#ifndef FAKE_STM32F401XX_H
#define FAKE_STM32F401XX_H

#include "../../generic/stm32f401xx.h"

extern GPIO_TypeDef fake_gpio[6];           /* A, B, C, D, E, H */
extern RCC_TypeDef  fake_rcc;
extern EXTI_Type    fake_exti;
extern SYSCFG_Type  fake_syscfg;

#undef GPIOA
#undef GPIOB
#undef GPIOC
#undef GPIOD
#undef GPIOE
#undef GPIOH
#undef RCC
#undef EXTI
#undef SYSCFG
#define GPIOA   (&fake_gpio[0])
#define GPIOB   (&fake_gpio[1])
#define GPIOC   (&fake_gpio[2])
#define GPIOD   (&fake_gpio[3])
#define GPIOE   (&fake_gpio[4])
#define GPIOH   (&fake_gpio[5])
#define RCC     (&fake_rcc)
#define EXTI    (&fake_exti)
#define SYSCFG  (&fake_syscfg)

#endif
