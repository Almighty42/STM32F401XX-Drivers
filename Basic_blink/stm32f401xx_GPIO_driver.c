/**
 * @file stm32f401xx_GPIO_driver.c
 * @brief Implementation of the GPIO driver. The API is documented in the
 * header.
 */

#include "stm32f401xx_GPIO_driver.h"
#include "stm32f401xx.h"
#include <stdbool.h>
#include <stdint.h>

/* ---- Internal helpers ------------------------------------------------ */

#define GPIO_PORT_COUNT 6U

/* Port order matches the AHB1ENR/AHB1RSTR bit positions: A..E = 0..4, H = 7. */
static GPIO_TypeDef* const gpio_ports[GPIO_PORT_COUNT] = {GPIOA, GPIOB, GPIOC,
                                                          GPIOD, GPIOE, GPIOH};
static const uint8_t gpio_ahb1_bit[GPIO_PORT_COUNT] = {0U, 1U, 2U, 3U, 4U, 7U};

/**
 * @brief Map a GPIO port pointer to its index in gpio_ports[].
 * @return Index 0..5, or -1 if @p port is not GPIOA..GPIOE or GPIOH.
 */
static uint32_t gpio_port_index(const GPIO_TypeDef* port)
{
	for (uint8_t i = 0U; i < GPIO_PORT_COUNT; i++) {
		if (gpio_ports[i] == port) {
			return (uint32_t)i;
		}
	}
	return -1;
}

/* ---- Clock and initialisation ---------------------------------------- */

/* ---- GPIO_peri_clk_control ------------------------------------------- */

/*
 * Reading AHB1ENR back after enabling gives the peripheral clock time to
 * start before the first register access. Check the STM32F401 errata sheet
 * for the exact wording of that requirement.
 */

GPIO_status_t GPIO_peri_clk_control(GPIO_TypeDef* port, bool enable)
{
	const uint32_t idx = gpio_port_index(port);
	if (idx < 0) {
		return GPIO_ERR_INVALID_PORT;
	}
	if (enable) {
		SET_BIT(RCC->AHB1ENR, gpio_ahb1_bit[idx]);
		(void)RCC->AHB1ENR;
	}
	else {
		CLEAR_BIT(RCC->AHB1ENR, gpio_ahb1_bit[idx]);
	}

	return GPIO_OK;
}

/* ---- GPIO_init ------------------------------------------------------- */

GPIO_status_t GPIO_init(GPIO_TypeDef* port, const GPIO_config_t* cfg)
{
	// Validation
	if (port == NULL || cfg == NULL) {
		return GPIO_ERR_NULL_PTR;
	}
	const uint32_t idx = gpio_port_index(port);
	if (idx < 0U) {
		return GPIO_ERR_INVALID_PORT;
	}

	const uint32_t pins = cfg->pins;
	if (pins == 0U) {
		return GPIO_ERR_INVALID_PINS;
	}
	if ((uint32_t)cfg->mode > (uint32_t)GPIO_MODE_ANALOG) {
		return GPIO_ERR_INVALID_MODE;
	}
	if ((uint32_t)cfg->otype > (uint32_t)GPIO_OTYPE_OPEN_DRAIN) {
		return GPIO_ERR_INVALID_OTYPE;
	}
	if ((uint32_t)cfg->speed > (uint32_t)GPIO_SPEED_HIGH) {
		return GPIO_ERR_INVALID_SPEED;
	}
	if ((uint32_t)cfg->pull > (uint32_t)GPIO_PULL_DOWN) {
		return GPIO_ERR_INVALID_PULL;
	}
	if (cfg->mode == GPIO_MODE_AF && (uint32_t)cfg->af > 15U) {
		return GPIO_ERR_INVALID_AF; /* also rejects GPIO_AF_NONE */
	}
	if ((uint32_t)cfg->trigger > (uint32_t)GPIO_TRIG_BOTH) {
		return GPIO_ERR_INVALID_TRIGGER;
	}

	/* EXTI line n on port P: port code equals the AHB1 bit (A..E = 0..4, H
	 * = 7). */
	const uint32_t port_code = gpio_ahb1_bit[idx];
}

/********************************************************************************
 * @fn			- GPIO_init
 *
 * @brief			- Initializes a GPIO pin
 *
 * @param[*p_GPIO_handle]	- Handle structure of a GPIO pin
 *
 * @return			- Success / Failure status of the function
 *
 * @Note			- None
 *******************************************************************************/

// GPIO_status_t GPIO_init(GPIO_Handle_t* p_GPIO_handle)
// {
// 	if (p_GPIO_handle == NULL)
// 		return GPIO_ERROR_NULL_PTR;
//
// 	GPIO_TypeDef* port = p_GPIO_handle->p_GPIOx;
// 	if (port == NULL || (port != GPIOA && port != GPIOB && port != GPIOC &&
// 	                     port != GPIOD && port != GPIOE && port != GPIOH))
// 		return GPIO_ERROR_INVALID_PORT;
//
// 	uint32_t pin_n = p_GPIO_handle->GPIO_Pin_Config.GPIO_Pin_Number;
//
// 	if (pin_n > 15U)
// 		return GPIO_ERROR_INVALID_PIN;
//
// 	// Mode
// 	uint8_t mode = p_GPIO_handle->GPIO_Pin_Config.GPIO_Pin_Mode;
// 	if (mode > GPIO_MODE_IT_RFT)
// 		return GPIO_ERROR_INVALID_MODE;
//
// 	uint32_t GPIO_pin_offset_1 = pin_n;
// 	uint32_t GPIO_pin_offset_2 = pin_n * 2U;
//
// 	if (mode <= GPIO_MODE_ANALOG) {
// 		// Interrupt disabled configuration routine
// 		CLEAR_FIELD_2BIT(p_GPIO_handle->p_GPIOx->MODER,
// 		                 GPIO_pin_offset_2);
// 		SET_BITS_BY_VAR(p_GPIO_handle->p_GPIOx->MODER,
// 		                mode << GPIO_pin_offset_2);
// 	}
// 	else {
// 		// Interrupt enabled configuration routine
// 		CLEAR_FIELD_2BIT(p_GPIO_handle->p_GPIOx->MODER,
// 		                 GPIO_pin_offset_2);
//
// 		if (mode == GPIO_MODE_IT_FT) {
// 			SET_BIT(EXTI->FTSR, GPIO_pin_offset_1);
// 			CLEAR_BIT(EXTI->RTSR, GPIO_pin_offset_1);
// 		}
// 		else if (mode == GPIO_MODE_IT_RT) {
// 			SET_BIT(EXTI->RTSR, GPIO_pin_offset_1);
// 			CLEAR_BIT(EXTI->FTSR, GPIO_pin_offset_1);
// 		}
// 		else if (mode == GPIO_MODE_IT_RFT) {
// 			SET_BIT(EXTI->FTSR, GPIO_pin_offset_1);
// 			SET_BIT(EXTI->RTSR, GPIO_pin_offset_1);
// 		}
//
// 		uint8_t exticr_n = pin_n / 4;
// 		uint8_t exticr_offset_n = pin_n % 4;
// 		uint8_t portcode =
// 		    GPIO_BASE_ADDR_TO_CODE(p_GPIO_handle->p_GPIOx);
//
// 		SYSCFG_PCLK_EN();
// 		CLEAR_FIELD_4BIT(SYSCFG->EXTICR[exticr_n],
// 		                 (exticr_offset_n * 4));
// 		SET_BITS_BY_VAR(SYSCFG->EXTICR[exticr_n],
// 		                (portcode << (exticr_offset_n * 4)));
//
// 		SYSCFG->EXTICR[exticr_n] = portcode << (exticr_offset_n * 4);
//
// 		SET_BIT(EXTI->IMR, GPIO_pin_offset_1);
// 	}
//
// 	// Speed
// 	uint8_t speed = p_GPIO_handle->GPIO_Pin_Config.GPIO_Pin_Speed;
// 	CLEAR_FIELD_2BIT(p_GPIO_handle->p_GPIOx->OSPEEDR, GPIO_pin_offset_2);
// 	SET_BITS_BY_VAR(p_GPIO_handle->p_GPIOx->OSPEEDR,
// 	                speed << GPIO_pin_offset_2);
//
// 	// PuPd
// 	uint8_t pupd = p_GPIO_handle->GPIO_Pin_Config.GPIO_Pin_PuPd_Control;
// 	CLEAR_FIELD_2BIT(p_GPIO_handle->p_GPIOx->PUPDR, GPIO_pin_offset_2);
// 	SET_BITS_BY_VAR(p_GPIO_handle->p_GPIOx->PUPDR,
// 	                pupd << GPIO_pin_offset_2);
//
// 	// OTP type
// 	uint8_t otp = p_GPIO_handle->GPIO_Pin_Config.GPIO_Pin_OTP_Type;
// 	CLEAR_BIT(p_GPIO_handle->p_GPIOx->OTYPER, GPIO_pin_offset_1);
// 	SET_BITS_BY_VAR(p_GPIO_handle->p_GPIOx->OTYPER,
// 	                otp << GPIO_pin_offset_1);
//
// 	// TODO: Alt func
//
// 	// uint8_t alt_func =
// 	// p_GPIO_handle->GPIO_Pin_Config.GPIO_Pin_Alt_Fun_Mode;
// 	//
// 	// if (alt_func == GPIO_MODE_ALT) {
// 	// 	uint8_t temp_offset = pin_n % 8;
// 	// 	temp = alt_func << (4 * temp_offset);
// 	// 	if (pin_n / 8 == 0) {
// 	// 		CLEAR_FIELD_4BIT(p_GPIO_handle->p_GPIOx->AFRL,
// 	// 		                 temp_offset);
// 	// 		SET_BITS_BY_VAR(p_GPIO_handle->p_GPIOx->AFRL, temp);
// 	// 	}
// 	// 	else {
// 	// 		CLEAR_FIELD_4BIT(p_GPIO_handle->p_GPIOx->AFRH,
// 	// 		                 temp_offset);
// 	// 		SET_BITS_BY_VAR(p_GPIO_handle->p_GPIOx->AFRH, temp);
// 	// 	}
// 	// 	temp = 0;
// 	// }
//
// 	return GPIO_OK;
// }

/********************************************************************************
 * @fn			- GPIO_deinit
 *
 * @brief			- Resets a GPIO pin
 *
 * @param[*p_GPIO_handle]	- Handle structure of a GPIO pin
 *
 * @return			- Success / Failure status of the function
 *
 * @Note			- None
 *******************************************************************************/

// GPIO_status_t GPIO_de_init(GPIO_TypeDef* p_GPIOx)
// {
// 	GPIO_status_t status = GPIO_ERROR_INVALID_PORT;
//
// 	if (p_GPIOx == GPIOA) {
// 		GPIOA_REG_RESET();
// 		status = GPIO_OK;
// 	}
// 	else if (p_GPIOx == GPIOB) {
// 		GPIOB_REG_RESET();
// 		status = GPIO_OK;
// 	}
// 	else if (p_GPIOx == GPIOC) {
// 		GPIOC_REG_RESET();
// 		status = GPIO_OK;
// 	}
// 	else if (p_GPIOx == GPIOD) {
// 		GPIOD_REG_RESET();
// 		status = GPIO_OK;
// 	}
// 	else if (p_GPIOx == GPIOE) {
// 		GPIOE_REG_RESET();
// 		status = GPIO_OK;
// 	}
// 	else if (p_GPIOx == GPIOH) {
// 		GPIOH_REG_RESET();
// 		status = GPIO_OK;
// 	}
//
// 	return status;
// }

/********************************************************************************
 * @fn			- GPIO_read_input_pin
 *
 * @brief			- Returns value of a GPIO input pin
 *
 * @param[*p_GPIOx]		- GPIO pin register structure
 * @param[pin_n]		- Pin number
 *
 * @return			- Value of pin
 *
 * @Note			- None
 *******************************************************************************/

// uint8_t GPIO_read_input_pin(GPIO_TypeDef* p_GPIOx, uint8_t pin_n)
// {
// 	return (uint8_t)((p_GPIOx->IDR >> pin_n) & 0x00000001);
// }

/********************************************************************************
 * @fn			- GPIO_read_input_port
 *
 * @brief			- Returns value of a GPIO input port
 *
 * @param[*p_GPIOx]		- GPIO pin register structure
 *
 * @return			- Value of port
 *
 * @Note			- None
 *******************************************************************************/

// uint16_t GPIO_read_input_port(GPIO_TypeDef* p_GPIOx)
// {
// 	return (uint16_t)p_GPIOx->IDR;
// }

/********************************************************************************
 * @fn			- GPIO_write_output_pin
 *
 * @brief			- Writes to a GPIO output pin
 *
 * @param[*p_GPIOx]		- GPIO pin register structure
 * @param[pin_n]		- Pin number
 * @param[val]			- Value to write
 *
 * @return			- None
 *
 * @Note			- None
 *******************************************************************************/

// void GPIO_write_output_pin(GPIO_TypeDef* p_GPIOx, uint8_t pin_n, uint8_t val)
// {
// 	if (val == GPIO_PIN_SET)
// 		SET_BIT(p_GPIOx->ODR, pin_n);
// 	else
// 		CLEAR_BIT(p_GPIOx->ODR, pin_n);
// }

/********************************************************************************
 * @fn			- GPIO_write_output_port
 *
 * @brief			- Writes to a GPIO output port
 *
 * @param[*p_GPIOx]		- GPIO pin register structure
 * @param[val]		- Value to write
 *
 * @return			- None
 *
 * @Note			- None
 *******************************************************************************/

// void GPIO_write_output_port(GPIO_TypeDef* p_GPIOx, uint8_t val)
// {
// 	p_GPIOx->ODR = val;
// }

/********************************************************************************
 * @fn				- GPIO_toggle_output_pin
 *
 * @brief			- Toggles GPIO output pin
 *
 * @param[*p_GPIOx]		- GPIO pin register structure
 * @param[pin_n]		- Pin number
 *
 * @return			- None
 *
 * @Note			- None
 *******************************************************************************/

// void GPIO_toggle_output_pin(GPIO_TypeDef* p_GPIOx, uint8_t pin_n)
// {
// 	TOGGLE_BIT(p_GPIOx->ODR, pin_n);
// }

/********************************************************************************
 * @fn			- GPIO_irq_interrupt_config
 *
 * @brief			- Configures GPIO IRQ interrupt
 *
 * @param[irq_n]		- IRQ number
 * @param[EN_or_DI]		- ENABLE or DISABLE macros
 *
 * @return			- Success / Failure status of the function
 *
 * @Note			- None
 *******************************************************************************/

// GPIO_status_t GPIO_irq_interrupt_config(uint8_t irq_n, uint8_t EN_or_DI)
// {
// 	if (EN_or_DI != ENABLE && EN_or_DI != DISABLE)
// 		return GPIO_ERROR_INVALID_STATE;
//
// 	if (irq_n >= 84)
// 		return GPIO_ERROR_INVALID_IRQ;
//
// 	SYSCFG_PCLK_EN();
//
// 	if (EN_or_DI == ENABLE) {
// 		if (irq_n < 32)
// 			SET_BIT(NVIC->ISER[0], irq_n);
// 		else if (irq_n >= 32 && irq_n < 64)
// 			SET_BIT(NVIC->ISER[1], (irq_n % 32));
// 		else if (irq_n >= 64 && irq_n < 96)
// 			SET_BIT(NVIC->ISER[2], (irq_n % 64));
// 	}
// 	else {
// 		if (irq_n < 32)
// 			SET_BIT(NVIC->ICER[0], irq_n);
// 		else if (irq_n >= 32 && irq_n < 64)
// 			SET_BIT(NVIC->ICER[1], (irq_n % 32));
// 		else if (irq_n >= 64 && irq_n < 96)
// 			SET_BIT(NVIC->ICER[2], (irq_n % 64));
// 	}
//
// 	return GPIO_OK;
// }

/********************************************************************************
 * @fn			- GPIO_irq_priority_config
 *
 * @brief			- Configures GPIO IRQ priority
 *
 * @param[irq_n]		- IRQ number
 * @param[irq_prio]		- IRQ priority
 *
 * @return			- Success / Failure status of the function
 *
 * @Note			- None
 *******************************************************************************/

// GPIO_status_t GPIO_irq_priority_config(uint8_t irq_n, uint8_t irq_prio)
// {
// 	if (irq_n >= 84)
// 		return GPIO_ERROR_INVALID_IRQ;
//
// 	// IPR_n[0-59]
// 	uint8_t ipr_n = irq_n / 4;
// 	// IPR_n IP_n[0-4]
// 	uint8_t ipr_n_section = (irq_n % 4) * 8;
// 	uint8_t prio_field = (irq_prio & ((1U << __NVIC_PRIO_BITS) - 1U))
// 	                     << (8U - __NVIC_PRIO_BITS);
//
// 	uint32_t ipr = NVIC->IPR[ipr_n];
// 	CLEAR_BYTE(ipr, ipr_n_section);
// 	ipr |= (prio_field << ipr_n_section);
// 	NVIC->IPR[ipr_n] = ipr;
//
// 	return GPIO_OK;
// }

/********************************************************************************
 * @fn			- GPIO_irq_handling
 *
 * @brief			- Handles GPIO EXTI interrupts
 *
 * @param[pin_n]		- Pin number
 *
 * @return			- None
 *
 * @Note			- None
 *******************************************************************************/

// void GPIO_irq_handling(uint8_t pin_n)
// {
// 	if (IS_BIT_SET(EXTI->PR, pin_n))
// 		SET_BIT(EXTI->PR, pin_n);
// }

/********************************************************************************
 * @fn				- GPIO_lock_pin
 *
 * @brief			- Locks GPIO configuration of a port
 *
 * @param[*p_GPIOx]		- Base address of the GPIO peripheral
 * @param[pin_n]		- Pin number
 *
 * @return			- Success / Failure status of the function
 *
 * @Note			- None
 *******************************************************************************/

// GPIO_status_t GPIO_lock_pin(GPIO_TypeDef* p_GPIOx, uint8_t pin_n)
// {
// 	if (p_GPIOx == NULL || (p_GPIOx != GPIOA && p_GPIOx != GPIOB &&
// 	                           p_GPIOx != GPIOC && p_GPIOx != GPIOD &&
// 	                           p_GPIOx != GPIOE && p_GPIOx != GPIOH))
// 		return GPIO_ERROR_INVALID_PORT;
//
// 	if (pin_n > 15U)
// 		return GPIO_ERROR_INVALID_PIN;
//
// 	SET_BIT(p_GPIOx->LCKR, pin_n);
// 	SET_BIT(p_GPIOx->LCKR, 16);
//
// 	CLEAR_BIT(p_GPIOx->LCKR, 16);
//
// 	SET_BIT(p_GPIOx->LCKR, 16);
//
// 	(void)p_GPIOx->LCKR;
//
// 	if (!IS_BIT_SET(p_GPIOx->LCKR, 16))
// 		return GPIO_ERROR_LOCK_FAILED;
//
// 	return GPIO_OK;
// }
