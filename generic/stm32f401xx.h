#ifndef STM32F401RE_HEADER
#define STM32F401RE_HEADER

#include <stdint.h>
#include <stddef.h>

// NOTE: --- Base Addresses --

#define FLASH_BASE_ADDR			0x08000000UL    // Flash
#define SRAM_BASE_ADDR			0x20000000UL    // SRAM
#define ROM_BASE_ADDR			0x1FFF0000UL    // System memory

// System Control Space ( Core peripherals )

#define SCS_BASE_ADDR			0xE000E000UL    // System Control Space start
#define SYSTICK_BASE_ADDR		(SCS_BASE_ADDR + 0x0010)    // SysTick
#define NVIC_BASE_ADDR			(SCS_BASE_ADDR + 0x0100)    // NVIC
#define SCB_BASE_ADDR			(SCS_BASE_ADDR + 0xD00)    // System Control Block

// AHBx and APBx Bus peripheral base adddresses

#define PERIPH_BASE			0x40000000UL    // Peripheral address start
#define APB1PERIPH_BASE_ADDR		PERIPH_BASE    // APB1
#define APB2PERIPH_BASE_ADDR		0x40010000UL    // APB2
#define AHB1PERIPH_BASE_ADDR		0x40020000UL    // AHB1
#define AHB2PERIPH_BASE_ADDR		0x50000000UL    // AHB2

// Base addresses of peripherals which are hanging on AHB1 bus

#define GPIOA_BASE_ADDR		(AHB1PERIPH_BASE_ADDR + 0x0000)    // GPIOA
#define GPIOB_BASE_ADDR		(AHB1PERIPH_BASE_ADDR + 0x0400)    // GPIOB
#define GPIOC_BASE_ADDR		(AHB1PERIPH_BASE_ADDR + 0x0800)    // GPIOC
#define GPIOD_BASE_ADDR		(AHB1PERIPH_BASE_ADDR + 0x0C00)    // GPIOD
#define GPIOE_BASE_ADDR		(AHB1PERIPH_BASE_ADDR + 0x1000)    // GPIOE
#define GPIOH_BASE_ADDR		(AHB1PERIPH_BASE_ADDR + 0x1C00)    // GPIOH
#define CRC_BASE_ADDR		(AHB1PERIPH_BASE_ADDR + 0x3000)    // CRC
#define RCC_BASE_ADDR		(AHB1PERIPH_BASE_ADDR + 0x3800)    // RCC
#define FLASH_INTERFACE_BASE_ADDR	(AHB1PERIPH_BASE_ADDR + 0x3C00)    // FLASH_INTERFACE
#define DMA1_BASE_ADDR		(AHB1PERIPH_BASE_ADDR + 0x6000)    // DMA1
#define DMA2_BASE_ADDR		(AHB1PERIPH_BASE_ADDR + 0x6400)    // DMA2

// Base addresses of peripherals which are hanging on AHB2 bus

#define USB_OTG_FS_BASE_ADDR		AHB1PERIPH_BASE_ADDR    // USB OTG FS

// Base addresses of peripherals which are hanging on APB1 bus

#define TIM2_BASE_ADDR		(APB1PERIPH_BASE_ADDR + 0x0000)    // TIM2
#define TIM3_BASE_ADDR		(APB1PERIPH_BASE_ADDR + 0x0400)    // TIM3
#define TIM4_BASE_ADDR		(APB1PERIPH_BASE_ADDR + 0x0800)    // TIM4
#define TIM5_BASE_ADDR		(APB1PERIPH_BASE_ADDR + 0x0C00)    // TIM5
#define RTC_BKP_BASE_ADDR	(APB1PERIPH_BASE_ADDR + 0x2800)    // RTC_BKP
#define WWDG_BASE_ADDR		(APB1PERIPH_BASE_ADDR + 0x2C00)    // WWDG
#define IWDG_BASE_ADDR		(APB1PERIPH_BASE_ADDR + 0x3000)    // IWDG
#define I2S2EXT_BASE_ADDR	(APB1PERIPH_BASE_ADDR + 0x3400)    // I2S2EXT
#define SPI2_BASE_ADDR		(APB1PERIPH_BASE_ADDR + 0x3800)    // SPI2
#define SPI3_BASE_ADDR		(APB1PERIPH_BASE_ADDR + 0x3C00)    // SPI3
#define I2S3EXT_BASE_ADDR	(APB1PERIPH_BASE_ADDR + 0x4000)    // I2S3EXT
#define USART2_BASE_ADDR	(APB1PERIPH_BASE_ADDR + 0x4400)    // USART2
#define I2C1_BASE_ADDR		(APB1PERIPH_BASE_ADDR + 0x5400)    // I2C1
#define I2C2_BASE_ADDR		(APB1PERIPH_BASE_ADDR + 0x5800)    // I2C2
#define I2C3_BASE_ADDR		(APB1PERIPH_BASE_ADDR + 0x5C00)    // I2C3
#define PWR_BASE_ADDR		(APB1PERIPH_BASE_ADDR + 0x7000)    // PWR

// Base addresses of peripherals which are hanging on APB2 bus

#define TIM1_BASE_ADDR		(APB2PERIPH_BASE_ADDR + 0x0000)    // TIM1
#define USART1_BASE_ADDR	(APB2PERIPH_BASE_ADDR + 0x1000)    // USART1
#define USART6_BASE_ADDR	(APB2PERIPH_BASE_ADDR + 0x1400)    // USART6
#define ADC1_BASE_ADDR		(APB2PERIPH_BASE_ADDR + 0x2000)    // ADC1
#define SDIO_BASE_ADDR		(APB2PERIPH_BASE_ADDR + 0x2C00)    // SDIO
#define SPI1_BASE_ADDR		(APB2PERIPH_BASE_ADDR + 0x3000)    // SPI1
#define SPI4_BASE_ADDR		(APB2PERIPH_BASE_ADDR + 0x3400)    // SPI4
#define SYSCFG_BASE_ADDR	(APB2PERIPH_BASE_ADDR + 0x3800)    // SYSCFG
#define EXTI_BASE_ADDR		(APB2PERIPH_BASE_ADDR + 0x3C00)    // EXTI
#define TIM9_BASE_ADDR		(APB2PERIPH_BASE_ADDR + 0x4000)    // TIM9
#define TIM10_BASE_ADDR		(APB2PERIPH_BASE_ADDR + 0x4400)    // TIM10
#define TIM11_BASE_ADDR		(APB2PERIPH_BASE_ADDR + 0x4800)    // TIM11

// NOTE: --- Utility macros ---

#define __I				volatile const    // Read only
#define __O				volatile    // Write only
#define __IO				volatile    // Read / Write

#define ENABLE				1
#define DISABLE				1
#define SET				ENABLE
#define RESET				DISABLE
#define GPIO_PIN_SET			SET
#define GPIO_PIN_RESET			RESET
#define USART_PIN_SET			SET
#define USART_PIN_RESET			RESET


// NOTE: --- Validation macros ---

#define VALIDATE_RANGE(val, min, max, err_code)		do { \
								if ((val) < (min) || (val) > (max)) { \
									return (err_code); \
								} \
							} while(0)

#define VALIDATE_PTR(ptr, err_code)			do { \
								if ((ptr) == NULL) { \
									return (err_code); \
								} \
							} while(0)

#define VALIDATE_EN_DI(EN_or_DI, err_code)		do { \
								if ((EN_or_DI) != ENABLE && \
									(EN_or_DI) != DISABLE) { \
									return (err_code); \
								} \
							} while (0)

#define VALIDATE_BIT_SET(reg, bit, err_code)		do { \
								if (!IS_BIT_SET((reg), (bit))) { \
									return (err_code); \
								} \
							} while(0)

#define VALIDATE_ENUM(val, max_val, err_code)		do { \
								if ((val) > (max_val)) { \
									return (err_code); \
								} \
							} while(0)

#define VALIDATE_IN_RANGE(val, range, len, err_code)	\
							do { \
								uint8_t found = 0; \
								for (uint32_t i = 0;i < len;i++) { \
									if ((val) == (range)[i]) { \
										found = 1;	\
										break;		\
									} \
								} \
								if (!found) return (err_code);	\
							} while(0)	\

#define VALIDATE_IRQ_NUMBER(irq, err_code)	VALIDATE_RANGE((irq), 0U, 83U, err_code)

// NOTE: --- Core Peripheral TypeDefs ---

typedef struct
{
	__IO uint32_t CTRL;
	__IO uint32_t LOAD;
	__IO uint32_t VAL;
	__IO uint32_t CALIB;
} SysTick_Type;

typedef struct
{
	__IO uint32_t ISER[8];
	uint32_t res0[24];
	__IO uint32_t ICER[8];
	uint32_t res1[24];
	__IO uint32_t ISPR[8];
	uint32_t res2[24];
	__IO uint32_t ICPR[8];
	uint32_t res3[24];
	__I uint32_t IABR[8];
	uint32_t res4[36];
	__IO uint8_t IPR[240];
	uint32_t res5[644];
	__O uint32_t STIR;

} NVIC_Type;

typedef struct
{
	__I uint32_t CPUID;
	__IO uint32_t ICSR;
	__IO uint32_t VTOR;
	__IO uint32_t AIRCR;
	__IO uint32_t SCR;
	__IO uint32_t CCR;
	__IO uint32_t SHPR1;
	__IO uint32_t SHPR2;
	__IO uint32_t SHPR3;
	__IO uint32_t SHCSR;
	__IO uint32_t CFSR;
	__IO uint32_t HFSR;
	uint32_t res0;
	__IO uint32_t MMAR;
	__IO uint32_t BFAR;
	__IO uint32_t AFSR;

} SCB_Type;
#define SCB ((SCB_Type*)0xE000ED00)

// NOTE: --- Other Peripheral TypeDefs ---

typedef struct {
	__IO uint32_t DR;
	__IO uint32_t IDR;
	__O uint32_t CR;
} CRC_TypeDef;

typedef struct {
	__IO uint32_t CR;
	__IO uint32_t CSR;
} PWR_TypeDef;

typedef struct
{
	__IO uint32_t CR;
	__IO uint32_t PLLCFGR;
	__IO uint32_t CFGR;
	__IO uint32_t CIR;
	__IO uint32_t AHB1RSTR;
	__IO uint32_t AHB2RSTR;
	uint32_t res0;
	uint32_t res1;
	__IO uint32_t APB1RSTR;
	__IO uint32_t APB2RSTR;
	uint32_t res2;
	uint32_t res3;
	__IO uint32_t AHB1ENR;
	__IO uint32_t AHB2ENR;
	uint32_t res4;
	uint32_t res5;
	__IO uint32_t APB1ENR;
	__IO uint32_t APB2ENR;
	uint32_t res6;
	uint32_t res7;
	__IO uint32_t AHB1LPENR;
	__IO uint32_t AHB2LPENR;
	uint32_t res8;
	uint32_t res9;
	__IO uint32_t APB1LPENR;
	__IO uint32_t APB2LPENR;
	uint32_t res10;
	uint32_t res11;
	__IO uint32_t BDCR;
	__IO uint32_t CSR;
	uint32_t res12;
	uint32_t res13;
	__IO uint32_t SSCGR;
	__IO uint32_t PLLI2SCFGR;
	__IO uint32_t DCKCFGR;
} RCC_TypeDef;

typedef struct
{
	__IO uint32_t MEMRMP;
	__IO uint32_t PMC;
	__IO uint32_t EXTICR[4];
	__IO uint32_t CMPCR;
} SYSCFG_Type;

typedef struct
{
	__IO uint32_t MODER;
	__IO uint32_t OTYPER;
	__IO uint32_t OSPEEDR;
	__IO uint32_t PUPDR;
	__IO uint32_t IDR;
	__IO uint32_t ODR;
	__IO uint32_t BSRR;
	__IO uint32_t LCKR;
	__IO uint32_t AFRL;
	__IO uint32_t AFRH;
} GPIO_TypeDef;

typedef struct {
	__I uint32_t LISR;
	__I uint32_t HISR;
	__O uint32_t LIFCR;
	__O uint32_t HIFCR;
	__IO uint32_t S0CR;
	__IO uint32_t S0NDTR;
	__IO uint32_t S0PAR;
	__IO uint32_t S0M0AR;
	__IO uint32_t S0M1AR;
	__IO uint32_t S0FCR;
	__IO uint32_t S1CR;
	__IO uint32_t S1NDTR;
	__IO uint32_t S1PAR;
	__IO uint32_t S1M0AR;
	__IO uint32_t S1M1AR;
	__IO uint32_t S1FCR;
	__IO uint32_t S2CR;
	__IO uint32_t S2NDTR;
	__IO uint32_t S2PAR;
	__IO uint32_t S2M0AR;
	__IO uint32_t S2M1AR;
	__IO uint32_t S2FCR;
	__IO uint32_t S3CR;
	__IO uint32_t S3NDTR;
	__IO uint32_t S3PAR;
	__IO uint32_t S3M0AR;
	__IO uint32_t S3M1AR;
	__IO uint32_t S3FCR;
	__IO uint32_t S4CR;
	__IO uint32_t S4NDTR;
	__IO uint32_t S4PAR;
	__IO uint32_t S4M0AR;
	__IO uint32_t S4M1AR;
	__IO uint32_t S4FCR;
	__IO uint32_t S5CR;
	__IO uint32_t S5NDTR;
	__IO uint32_t S5PAR;
	__IO uint32_t S5M0AR;
	__IO uint32_t S5M1AR;
	__IO uint32_t S5FCR;
	__IO uint32_t S6CR;
	__IO uint32_t S6NDTR;
	__IO uint32_t S6PAR;
	__IO uint32_t S6M0AR;
	__IO uint32_t S6M1AR;
	__IO uint32_t S6FCR;
	__IO uint32_t S7CR;
	__IO uint32_t S7NDTR;
	__IO uint32_t S7PAR;
	__IO uint32_t S7M0AR;
	__IO uint32_t S7M1AR;
	__IO uint32_t S7FCR;
} DMA_TypeDef;

typedef struct
{
	__IO uint32_t IMR;
	__IO uint32_t EMR;
	__IO uint32_t RTSR;
	__IO uint32_t FTSR;
	__IO uint32_t SWIER;
	__IO uint32_t PR;
} EXTI_Type;

// TODO: TypeDef ADC

// TODO: TypeDef TIM

// TODO: TypeDef IWDG

// TODO: TypeDef WWDG

// TODO: TypeDef RTC

typedef struct {
	__IO uint32_t TR;
	__IO uint32_t DR;
	__IO uint32_t CR;
	__IO uint32_t ISR;
	__IO uint32_t PRER;
	__IO uint32_t WUTR;
	__IO uint32_t CALIBR;
	__IO uint32_t ALRMAR;
	__IO uint32_t ALRMBR;
	__O uint32_t WPR;
	__I uint32_t SSR;
	__IO uint32_t SHIFTR;
	__I uint32_t TSTR;
	__I uint32_t TSDR;
	__I uint32_t TSSSR;
	__IO uint32_t CALR;
	__IO uint32_t TAFCR;
	__IO uint32_t ALRMASSR;
	__IO uint32_t ALRMBSSR;
	 uint32_t res0;
	__IO uint32_t BKPxR[20];
} RTC_TypeDef;

typedef struct {
	__IO uint32_t CR1;
	__IO uint32_t CR2;
	__IO uint32_t OAR1;
	__IO uint32_t OAR2;
	__IO uint32_t DR;
	__IO uint32_t SR1;
	__I uint32_t SR2;
	__IO uint32_t CCR;
	__IO uint32_t TRISE;
	__IO uint32_t FLTR;
} I2C_TypeDef;

typedef struct {
	__IO uint32_t SR;
	__IO uint32_t DR;
	__IO uint32_t BRR;
	__IO uint32_t CR1;
	__IO uint32_t CR2;
	__IO uint32_t CR3;
	__IO uint32_t GTPR;
} USART_TypeDef;

typedef struct {
	__IO uint32_t CR1;
	__IO uint32_t CR2;
	__IO uint32_t SR;
	__IO uint32_t DR;
	__IO uint32_t CRCPR;
	__I uint32_t RXCRCR;
	__I uint32_t TXCRCR;
	__IO uint32_t I2SCFGR;
	__IO uint32_t I2SPR;
} SPI_TypeDef;

// TODO: TypeDef SDIO

// TODO: TypeDef OTG_FS

// NOTE: --- TypeDef Macros ---

#define CRC				((CRC_TypeDef*)CRC_BASE_ADDR)

#define PWR				((PWR_TypeDef*)PWR_BASE_ADDR)

#define RCC				((RCC_TypeDef*)RCC_BASE_ADDR)

#define SYSCFG				((SYSCFG_Type*)SYSCFG_BASE_ADDR)

#define GPIOA				((GPIO_TypeDef*)GPIOA_BASE_ADDR)
#define GPIOB				((GPIO_TypeDef*)GPIOB_BASE_ADDR)
#define GPIOC				((GPIO_TypeDef*)GPIOC_BASE_ADDR)
#define GPIOD				((GPIO_TypeDef*)GPIOD_BASE_ADDR)
#define GPIOE				((GPIO_TypeDef*)GPIOE_BASE_ADDR)
#define GPIOH				((GPIO_TypeDef*)GPIOH_BASE_ADDR)

#define DMA1				((DMA_TypeDef*)DMA1_BASE_ADDR)
#define DMA2				((DMA_TypeDef*)DMA2_BASE_ADDR)

#define NVIC				((NVIC_Type*)NVIC_BASE_ADDR)
#define EXTI				((EXTI_Type*)EXTI_BASE_ADDR)

// TODO: TypeDef Macro ADC

// TODO: TypeDef Macro TIM

// TODO: TypeDef Macro IWDG

// TODO: TypeDef Macro WWDG

// TODO: TypeDef Macro RTC


#define I2C1				((I2C_TypeDef*)I2C1_BASE_ADDR)
#define I2C2				((I2C_TypeDef*)I2C2_BASE_ADDR)
#define I2C3				((I2C_TypeDef*)I2C3_BASE_ADDR)

#define USART1				((USART_TypeDef*)USART1_BASE_ADDR)
#define USART2				((USART_TypeDef*)USART2_BASE_ADDR)
#define USART6				((USART_TypeDef*)USART6_BASE_ADDR)

#define SPI1				((SPI_TypeDef*)SPI1_BASE_ADDR)
#define SPI2				((SPI_TypeDef*)SPI2_BASE_ADDR)
#define SPI3				((SPI_TypeDef*)SPI3_BASE_ADDR)
#define SPI4				((SPI_TypeDef*)SPI4_BASE_ADDR)

// TODO: TypeDef Macro SDIO

// TODO: TypeDef Macro OTG_FS

#define __NVIC_PRIO_BITS	4

// NOTE: --- Shortcut macros ---

// Clock enable/disable macros for GPIOx peripherals

#define GPIOA_PCLK_EN()		SET_BIT(RCC->AHB1ENR, 0)
#define GPIOB_PCLK_EN()		SET_BIT(RCC->AHB1ENR, 1)
#define GPIOC_PCLK_EN()		SET_BIT(RCC->AHB1ENR, 2)
#define GPIOD_PCLK_EN()		SET_BIT(RCC->AHB1ENR, 3)
#define GPIOE_PCLK_EN()		SET_BIT(RCC->AHB1ENR, 4)
#define GPIOH_PCLK_EN()		SET_BIT(RCC->AHB1ENR, 7)

#define GPIOA_PCLK_DI()		CLEAR_BIT(RCC->AHB1ENR, 0)
#define GPIOB_PCLK_DI()		CLEAR_BIT(RCC->AHB1ENR, 1)
#define GPIOC_PCLK_DI()		CLEAR_BIT(RCC->AHB1ENR, 2)
#define GPIOD_PCLK_DI()		CLEAR_BIT(RCC->AHB1ENR, 3)
#define GPIOE_PCLK_DI()		CLEAR_BIT(RCC->AHB1ENR, 4)
#define GPIOH_PCLK_DI()		CLEAR_BIT(RCC->AHB1ENR, 7)

// Clock enable/disable macros for I2Cx peripherals

#define I2C1_PCLK_EN()		SET_BIT(RCC->APB1ENR, 21)
#define I2C2_PCLK_EN()		SET_BIT(RCC->APB1ENR, 22)
#define I2C3_PCLK_EN()		SET_BIT(RCC->APB1ENR, 23)

#define I2C1_PCLK_DI()		CLEAR_BIT(RCC->APB1ENR, 21)
#define I2C2_PCLK_DI()		CLEAR_BIT(RCC->APB1ENR, 22)
#define I2C3_PCLK_DI()		CLEAR_BIT(RCC->APB1ENR, 23)

// Clock enable/disable macros for SPIx peripherals

#define SPI1_PCLK_EN()		SET_BIT(RCC->APB2ENR, 12)
#define SPI2_PCLK_EN()		SET_BIT(RCC->APB1ENR, 14)
#define SPI3_PCLK_EN()		SET_BIT(RCC->APB1ENR, 15)
#define SPI4_PCLK_EN()		SET_BIT(RCC->APB2ENR, 13)

#define SPI1_PCLK_DI()		CLEAR_BIT(RCC->APB2ENR, 12)
#define SPI2_PCLK_DI()		CLEAR_BIT(RCC->APB1ENR, 14)
#define SPI3_PCLK_DI()		CLEAR_BIT(RCC->APB1ENR, 15)
#define SPI4_PCLK_DI()		CLEAR_BIT(RCC->APB2ENR, 13)

// Clock enable/disable macros for USARTx peripherals

#define USART1_PCLK_EN()	SET_BIT(RCC->APB2ENR, 4)
#define USART2_PCLK_EN()	SET_BIT(RCC->APB1ENR, 17)
#define USART6_PCLK_EN()	SET_BIT(RCC->APB2ENR, 5)

#define USART1_PCLK_DI()	CLEAR_BIT(RCC->APB2ENR, 4)
#define USART2_PCLK_DI()	CLEAR_BIT(RCC->APB1ENR, 17)
#define USART6_PCLK_DI()	CLEAR_BIT(RCC->APB2ENR, 5)

// Clock enable macros for SYSCFG peripherals

#define SYSCFG_PCLK_EN()	SET_BIT(RCC->APB2ENR, 14)

#define SYSCFG_PCLK_DI()	CLEAR_BIT(RCC->APB2ENR, 14)

// Macros for reseting GPIOx peripherals

#define GPIOA_PER_RESET()	do{SET_BIT(RCC->APB1RSTR, 0); CLEAR_BIT(RCC->APB1RSTR, 0);} while(0)
#define GPIOA_CLK_DISABLE()	do{SET_BIT(RCC->APB1ENR, 0); CLEAR_BIT(RCC->APB1ENR, 0);} while(0)
#define GPIOB_PER_RESET()	do{SET_BIT(RCC->APB1RSTR, 1); CLEAR_BIT(RCC->APB1RSTR, 1);} while(0)
#define GPIOB_CLK_DISABLE()	do{SET_BIT(RCC->APB1ENR, 1); CLEAR_BIT(RCC->APB1ENR, 1);} while(0)
#define GPIOC_PER_RESET()	do{SET_BIT(RCC->APB1RSTR, 2); CLEAR_BIT(RCC->APB1RSTR, 2);} while(0)
#define GPIOC_CLK_DISABLE()	do{SET_BIT(RCC->APB1ENR, 2); CLEAR_BIT(RCC->APB1ENR, 2);} while(0)
#define GPIOD_PER_RESET()	do{SET_BIT(RCC->APB1RSTR, 3); CLEAR_BIT(RCC->APB1RSTR, 3);} while(0)
#define GPIOD_CLK_DISABLE()	do{SET_BIT(RCC->APB1ENR, 3); CLEAR_BIT(RCC->APB1ENR, 3);} while(0)
#define GPIOE_PER_RESET()	do{SET_BIT(RCC->APB1RSTR, 4); CLEAR_BIT(RCC->APB1RSTR, 4);} while(0)
#define GPIOE_CLK_DISABLE()	do{SET_BIT(RCC->APB1ENR, 4); CLEAR_BIT(RCC->APB1ENR, 4);} while(0)
#define GPIOH_PER_RESET()	do{SET_BIT(RCC->APB1RSTR,7); CLEAR_BIT(RCC->APB1RSTR, 7);} while(0)
#define GPIOH_CLK_DISABLE()	do{SET_BIT(RCC->APB1ENR, 7); CLEAR_BIT(RCC->APB1ENR, 7);} while(0)

// Macros for reseting USARTx peripherals

#define USART1_PER_RESET()	do{SET_BIT(RCC->APB2RSTR, 4); CLEAR_BIT(RCC->APB2RSTR, 4);} while(0)
#define USART1_CLK_DISABLE()	do{SET_BIT(RCC->APB2ENR, 4); CLEAR_BIT(RCC->APB2ENR, 4);} while(0)
#define USART2_PER_RESET()	do{SET_BIT(RCC->APB1RSTR, 17); CLEAR_BIT(RCC->APB1RSTR, 17);} while(0)
#define USART2_CLK_DISABLE()	do{SET_BIT(RCC->APB1ENR, 17); CLEAR_BIT(RCC->APB1ENR, 17);} while(0)
#define USART6_PER_RESET()	do{SET_BIT(RCC->APB2RSTR, 5); CLEAR_BIT(RCC->APB2RSTR, 5);} while(0)
#define USART6_CLK_DISABLE()	do{SET_BIT(RCC->APB2ENR, 5); CLEAR_BIT(RCC->APB2ENR, 5);} while(0)

// Macros for reseting SPIx peripherals

#define SPI1_PER_RESET()	do{SET_BIT(RCC->APB2RSTR, 12); CLEAR_BIT(RCC->APB2RSTR, 12);} while(0)
#define SPI1_CLK_DISABLE()	do{SET_BIT(RCC->APB2ENR, 12); CLEAR_BIT(RCC->APB2ENR, 12);} while(0)
#define SPI2_PER_RESET()	do{SET_BIT(RCC->APB1RSTR, 14); CLEAR_BIT(RCC->APB1RSTR, 14);} while(0)
#define SPI2_CLK_DISABLE()	do{SET_BIT(RCC->APB1ENR, 17); CLEAR_BIT(RCC->APB1ENR, 17);} while(0)
#define SPI3_PER_RESET()	do{SET_BIT(RCC->APB1RSTR, 15); CLEAR_BIT(RCC->APB1RSTR, 15);} while(0)
#define SPI3_CLK_DISABLE()	do{SET_BIT(RCC->APB1ENR, 15); CLEAR_BIT(RCC->APB1ENR, 15);} while(0)
#define SPI4_PER_RESET()	do{SET_BIT(RCC->APB2RSTR, 13); CLEAR_BIT(RCC->APB2RSTR, 13);} while(0)
#define SPI4_CLK_DISABLE()	do{SET_BIT(RCC->APB2ENR, 13); CLEAR_BIT(RCC->APB2ENR, 13);} while(0)

// Macros for reseting I2Cx peripherals

#define I2C1_PER_RESET()	do{SET_BIT(RCC->APB1RSTR, 21); CLEAR_BIT(RCC->APB1RSTR, 21);} while(0)
#define I2C1_CLK_DISABLE()	do{SET_BIT(RCC->APB1ENR, 21); CLEAR_BIT(RCC->APB1ENR, 21);} while(0)
#define I2C2_PER_RESET()	do{SET_BIT(RCC->APB1RSTR, 22); CLEAR_BIT(RCC->APB1RSTR, 22);} while(0)
#define I2C2_CLK_DISABLE()	do{SET_BIT(RCC->APB1ENR, 22); CLEAR_BIT(RCC->APB1ENR, 22);} while(0)
#define I2C3_PER_RESET()	do{SET_BIT(RCC->APB1RSTR, 23); CLEAR_BIT(RCC->APB1RSTR, 23);} while(0)
#define I2C3_CLK_DISABLE()	do{SET_BIT(RCC->APB1ENR, 23); CLEAR_BIT(RCC->APB1ENR, 23);} while(0)

// NOTE: --- Miscellaneous macros ---

#define GPIO_BASE_ADDR_TO_CODE(x)	((x == GPIOA) ? 0 : \
					(x == GPIOB) ? 1 : \
					(x == GPIOC) ? 2 : \
					(x == GPIOD) ? 3 : \
					(x == GPIOE) ? 4 : \
					(x == GPIOH) ? 7 : 0 )

// NOTE: --- Macros for IRQ numbers ---

// IRQ numbers

#define IRQ_NO_WWDG                 0				// Window Watchdog interrupt
#define IRQ_NO_PVD                  1				// PVD through EXTI line detection interrupt
#define IRQ_NO_TAMP_STAMP           2				// Tamper and Time stamp interrupts through EXTI line
#define IRQ_NO_RTC_WKUP             3				// RTC Wakeup interrupt through EXTI line
#define IRQ_NO_FLASH                4				// Flash global interrupt
#define IRQ_NO_RCC                  5				// RCC global interrupt
#define IRQ_NO_EXTI0                6   			// EXTI Line0 interrupt
#define IRQ_NO_EXTI1                7   			// EXTI Line1 interrupt
#define IRQ_NO_EXTI2                8   			// EXTI Line2 interrupt
#define IRQ_NO_EXTI3                9   			// EXTI Line3 interrupt
#define IRQ_NO_EXTI4                10  			// EXTI Line4 interrupt
#define IRQ_NO_DMA1_STREAM0         11  			// DMA1 Stream0 interrupt
#define IRQ_NO_DMA1_STREAM1         12  			// DMA1 Stream1 interrupt
#define IRQ_NO_DMA1_STREAM2         13  			// DMA1 Stream2 interrupt
#define IRQ_NO_DMA1_STREAM3         14  			// DMA1 Stream3 interrupt
#define IRQ_NO_DMA1_STREAM4         15  			// DMA1 Stream4 interrupt
#define IRQ_NO_DMA1_STREAM5         16  			// DMA1 Stream5 interrupt
#define IRQ_NO_DMA1_STREAM6         17  			// DMA1 Stream6 interrupt
#define IRQ_NO_ADC                  18  			// ADC1, ADC2 and ADC3 global interrupts
#define IRQ_NO_EXTI9_5              23  			// EXTI Line 5 - 9 interrupt
#define	IRQ_NO_TIM1_BRK_TIM9        24  			// TIM1 Break interrupt and TIM9 global interrupt
#define	IRQ_NO_TIM1_UP_TIM10        25  			// TIM1 Update interrupt and TIM10 global interrupt
#define IRQ_NO_TIM1_TRG_COM_TIM11   26  			// TIM1 Trigger and Commutation interrupts and TIM11 global interrupt
#define IRQ_NO_TIM1_CC              27  			// TIM1 Capture Compare interrupt
#define IRQ_NO_TIM2                 28  			// TIM2 global interrupt
#define IRQ_NO_TIM3                 29  			// TIM3 global interrupt
#define IRQ_NO_TIM4                 30  			// TIM4 global interrupt
#define IRQ_NO_I2C1_EV              31  			// I2C1 event interrupt
#define IRQ_NO_I2C1_ER              32  			// I2C1 error interrupt
#define IRQ_NO_I2C2_EV              33  			// I2C2 event interrupt
#define IRQ_NO_I2C2_ER              34  			// I2C2 error interrupt
#define IRQ_NO_SPI1                 35  			// SPI1 global interrupt
#define IRQ_NO_SPI2                 36  			// SPI2 global interrupt
#define IRQ_NO_USART1               37  			// USART1 global interrupt
#define IRQ_NO_USART2               38  			// USART2 global interrupt
#define IRQ_NO_EXTI15_10            40  			// EXTI Line[15:10] interrupts
#define IRQ_NO_RTC_ALARM            41  			// RTC Alarms (A and B) through EXTI line interrupts
#define IRQ_NO_OTG_FS_WKUP          42  			// USB On-The-Go FS Wakeup through EXTI line interrupt
#define IRQ_NO_DMA1_STREAM7         47  			// DMA1 Stream7 global interrupt
#define IRQ_NO_SDIO                 49  			// SDIO global interrupt
#define IRQ_NO_TIM5                 50  			// TIM5 global interrupt
#define IRQ_NO_SPI3                 51  			// SPI3 global interrupt
#define IRQ_NO_DMA2_STREAM0         56  			// DMA2 Stream0 global interrupt
#define IRQ_NO_DMA2_STREAM1         57  			// DMA2 Stream1 global interrupt
#define IRQ_NO_DMA2_STREAM2         58  			// DMA2 Stream2 global interrupt
#define IRQ_NO_DMA2_STREAM3	    59  			// DMA2 Stream3 global interrupt
#define IRQ_NO_DMA2_STREAM4	    60  			// DMA2 Stream4 global interrupt
#define IRQ_NO_OTG_FS		    67				// USB On The Go FS global interrupt
#define IRQ_NO_DMA2_STREAM5         68				// DMA2 Stream5 global interrupt
#define IRQ_NO_DMA2_STREAM6         69   			// DMA2 Stream6 global interrupt
#define IRQ_NO_DMA2_STREAM7         70   			// DMA2 Stream7 global interrupt
#define IRQ_NO_USART6               71   			// USART6 global interrupt
#define IRQ_NO_I2C3_EV              72   			// I2C3 event interrupt
#define IRQ_NO_I2C3_ER              73   			// I2C3 error interrupt
#define IRQ_NO_FPU                  81   			// FPU global interrupt
#define IRQ_NO_SPI4                 84   			// SPI4 global interrupt

// Macros for NVIC priorities

#define NVIC_IRQ_PRIO_0			0
#define NVIC_IRQ_PRIO_1			1
#define NVIC_IRQ_PRIO_2			2
#define NVIC_IRQ_PRIO_3			3
#define NVIC_IRQ_PRIO_4			4
#define NVIC_IRQ_PRIO_5			5
#define NVIC_IRQ_PRIO_6			6
#define NVIC_IRQ_PRIO_7			7
#define NVIC_IRQ_PRIO_8			8
#define NVIC_IRQ_PRIO_9			9
#define NVIC_IRQ_PRIO_10		10
#define NVIC_IRQ_PRIO_11		11
#define NVIC_IRQ_PRIO_12		12
#define NVIC_IRQ_PRIO_13		13
#define NVIC_IRQ_PRIO_14		14
#define NVIC_IRQ_PRIO_15		15

#endif

