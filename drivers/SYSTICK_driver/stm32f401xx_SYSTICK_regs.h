#ifndef STM32F401XX_SYSTICK_REGS_H
#define STM32F401XX_SYSTICK_REGS_H

#include "reg_utils.h"

#define SYSTICK_CTRL_ENABLE    BIT(0)  /* counter enabled */
#define SYSTICK_CTRL_TICKINT   BIT(1)  /* exception request at zero */
#define SYSTICK_CTRL_CLKSOURCE BIT(2)  /* 1 = processor clock, 0 = HCLK / 8 */
#define SYSTICK_CTRL_COUNTFLAG BIT(16) /* read only: counted to zero */

#define SYSTICK_CTRL_RUN \
	(SYSTICK_CTRL_ENABLE | SYSTICK_CTRL_TICKINT | SYSTICK_CTRL_CLKSOURCE)

/* SysTick_LOAD: 24-bit reload value */
#define SYSTICK_LOAD_MASK GENMASK(23, 0)

#endif
