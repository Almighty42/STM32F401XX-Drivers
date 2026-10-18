/* drivers/RCC_driver/stm32f401xx_RCC_regs.h */
#ifndef STM32F401XX_RCC_REGS_H
#define STM32F401XX_RCC_REGS_H

#include "reg_utils.h"

/* RCC_CR */
#define RCC_CR_HSION      BIT(0)
#define RCC_CR_HSIRDY     BIT(1)
#define RCC_CR_HSEON      BIT(16)
#define RCC_CR_HSERDY     BIT(17)
#define RCC_CR_HSEBYP     BIT(18)
#define RCC_CR_PLLON      BIT(24)
#define RCC_CR_PLLRDY     BIT(25)

/* RCC_PLLCFGR */
#define RCC_PLLCFGR_PLLM_MASK   GENMASK(5, 0)
#define RCC_PLLCFGR_PLLN_MASK   GENMASK(14, 6)
#define RCC_PLLCFGR_PLLP_MASK   GENMASK(17, 16)
#define RCC_PLLCFGR_PLLSRC      BIT(22)
#define RCC_PLLCFGR_PLLQ_MASK   GENMASK(27, 24)

/* RCC_CFGR */
#define RCC_CFGR_SW_MASK        GENMASK(1, 0)
#define RCC_CFGR_SWS_MASK       GENMASK(3, 2)
#define RCC_CFGR_HPRE_MASK      GENMASK(7, 4)
#define RCC_CFGR_PPRE1_MASK     GENMASK(12, 10)
#define RCC_CFGR_PPRE2_MASK     GENMASK(15, 13)
#define RCC_CFGR_MCO1_MASK      GENMASK(22, 21)
#define RCC_CFGR_MCO1PRE_MASK   GENMASK(26, 24)

/* FLASH_ACR */
#define FLASH_ACR_LATENCY_MASK  GENMASK(2, 0)
#define FLASH_ACR_PRFTEN        BIT(8)
#define FLASH_ACR_ICEN          BIT(9)
#define FLASH_ACR_DCEN          BIT(10)

/* RCC_APB1ENR */
#define RCC_APB1ENR_PWREN BIT(28)

/* RCC_BDCR (backup domain, write-protected until PWR_CR.DBP is set) */
#define RCC_BDCR_LSEON BIT(0)
#define RCC_BDCR_LSERDY BIT(1)
#define RCC_BDCR_LSEBYP BIT(2)

/* RCC_CSR */
#define RCC_CSR_LSION BIT(0)
#define RCC_CSR_LSIRDY BIT(1)

/* PWR_CR */
#define PWR_CR_DBP BIT(8)

#endif
