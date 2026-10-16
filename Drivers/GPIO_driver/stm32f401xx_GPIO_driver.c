/**
 * @file stm32f401xx_GPIO_driver.c
 * @brief Implementation of the GPIO driver. The API is documented in the
 * header.
 */

#include "stm32f401xx_GPIO_driver.h"
#include <stdint.h>
#include <sys/types.h>

/* ---- Internal helpers ------------------------------------------------ */

#define GPIO_PORT_COUNT 6U

/* GPIO register reset values from RM0368. Only ports A and B differ from 0.
 * Order matches gpio_ports[]: A, B, C, D, E, H. OTYPER, ODR, AFRL, AFRH
 * and LCKR all reset to 0. */
typedef struct
{
	uint32_t moder;
	uint32_t ospeedr;
	uint32_t pupdr;
} gpio_reset_vals_t;

static const gpio_reset_vals_t gpio_reset_vals[GPIO_PORT_COUNT] = {
    {0xA8000000U, 0x0C000000U, 0x64000000U}, /* A: PA13..PA15 SWD/JTAG */
    {0x00000280U, 0x000000C0U, 0x00000100U}, /* B: PB3, PB4 JTAG        */
    {0U, 0U, 0U},                            /* C */
    {0U, 0U, 0U},                            /* D */
    {0U, 0U, 0U},                            /* E */
    {0U, 0U, 0U},                            /* H */
};

/* Port order matches the AHB1ENR/AHB1RSTR bit positions: A..E = 0..4, H = 7. */
static GPIO_TypeDef* const gpio_ports[GPIO_PORT_COUNT] = {GPIOA, GPIOB, GPIOC,
                                                          GPIOD, GPIOE, GPIOH};
static const uint8_t gpio_ahb1_bit[GPIO_PORT_COUNT] = {0U, 1U, 2U, 3U, 4U, 7U};

/**
 * @brief Map a GPIO port pointer to its index in gpio_ports[].
 * @return Index 0..5, or -1 if @p port is not GPIOA..GPIOE or GPIOH.
 */
static int gpio_port_index(const GPIO_TypeDef* port)
{
	for (size_t i = 0U; i < GPIO_PORT_COUNT; i++) {
		if (gpio_ports[i] == port) {
			return (int)i;
		}
	}
	return -1;
}

/** True if the EXTI line is unmasked; *port_code is the port it routes to. */
static bool gpio_exti_line_routed(uint32_t line, uint32_t* port_code)
{
	if (!IS_BIT_SET(EXTI->IMR, line)) {
		return false;
	}
	*port_code = (SYSCFG->EXTICR[line / 4U] >> ((line % 4U) * 4U)) & 0xFU;
	return true;
}

/** Route and configure one EXTI line, or release it with GPIO_TRIG_NONE. */
static void gpio_exti_configure(uint32_t line, uint32_t port_code,
                                GPIO_trigger_t trig)
{
	CLEAR_BIT(EXTI->IMR, line);
	CLEAR_BIT(EXTI->RTSR, line);
	CLEAR_BIT(EXTI->FTSR, line);

	if (trig != GPIO_TRIG_NONE) {
		SYSCFG_PCLK_EN();
		MODIFY_FIELD(SYSCFG->EXTICR[line / 4U], 4U, (line % 4U) * 4U,
		             port_code);
		if (trig == GPIO_TRIG_RISING || trig == GPIO_TRIG_BOTH) {
			SET_BIT(EXTI->RTSR, line);
		}
		if (trig == GPIO_TRIG_FALLING || trig == GPIO_TRIG_BOTH) {
			SET_BIT(EXTI->FTSR, line);
		}
	}
	EXTI->PR = 1U << line;
	if (trig != GPIO_TRIG_NONE) {
		SET_BIT(EXTI->IMR, line);
	}
}

/** Release every EXTI line in @p pins that is currently owned by this port. */
static void gpio_exti_release(uint32_t pins, uint32_t port_code)
{
	for (uint32_t n = 0U; n < 16U; n++) {
		uint32_t routed;
		if ((pins & (1U << n)) != 0U &&
		    gpio_exti_line_routed(n, &routed) && routed == port_code) {
			gpio_exti_configure(n, port_code, GPIO_TRIG_NONE);
		}
	}
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
	const int idx = gpio_port_index(port);
	if (idx < 0) {
		return GPIO_ERR_INVALID_PORT;
	}
	if (enable) {
		SET_BIT(RCC->AHB1ENR, gpio_ahb1_bit[idx]);
		(void)RCC->AHB1ENR; /* read-back: clock start-up delay */
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
	const int idx = gpio_port_index(port);
	if (idx < 0) {
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

	if (cfg->trigger != GPIO_TRIG_NONE) {
		if (cfg->mode != GPIO_MODE_INPUT) {
			return GPIO_ERR_INVALID_TRIGGER;
		}
		if ((pins & (pins - 1U)) != 0U) {
			return GPIO_ERR_EXTI_NOT_SINGLE;
		}
		uint32_t routed;
		if (gpio_exti_line_routed((uint32_t)__builtin_ctz(pins),
		                          &routed) &&
		    routed != port_code) {
			return GPIO_ERR_EXTI_LINE_BUSY;
		}
	}

	(void)GPIO_peri_clk_control(port, true);

	for (uint32_t n = 0U; n < 16U; n++) {
		if ((pins & (1U << n)) == 0U) {
			continue;
		}

		if (cfg->mode == GPIO_MODE_OUTPUT ||
		    cfg->mode == GPIO_MODE_AF) {
			MODIFY_FIELD(port->OTYPER, 1U, n, cfg->otype);
			MODIFY_FIELD(port->OSPEEDR, 2U, n * 2U, cfg->speed);
		}
		MODIFY_FIELD(port->PUPDR, 2U, n * 2U, cfg->pull);

		if (cfg->mode == GPIO_MODE_AF) {
			volatile uint32_t* afr =
			    (n < 8U) ? &port->AFRL : &port->AFRH;
			MODIFY_FIELD(*afr, 4U, (n % 8U) * 4U, cfg->af);
		}
		MODIFY_FIELD(port->MODER, 2U, n * 2U, cfg->mode);

		uint32_t routed;
		if (cfg->trigger != GPIO_TRIG_NONE) {
			gpio_exti_configure(n, port_code, cfg->trigger);
		}
		else if (gpio_exti_line_routed(n, &routed) &&
		         routed == port_code) {
			gpio_exti_configure(n, port_code, GPIO_TRIG_NONE);
		}
	}

	return GPIO_OK;
}

/* ---- GPIO_pin_deinit ------------------------------------------------- */

/*
 * Order: release EXTI first so no edge fires during teardown, then restore
 * type/speed/pull/AF, then MODER, then clear the output latch. Pins locked
 * with GPIO_lock_pins() ignore these writes until the next reset.
 */

GPIO_status_t GPIO_pin_deinit(GPIO_TypeDef* port, uint16_t pins)
{
	if (port == NULL) {
		return GPIO_ERR_NULL_PTR;
	}

	const int idx = gpio_port_index(port);
	if (idx < 0) {
		return GPIO_ERR_INVALID_PORT;
	}
	if (pins == 0U) {
		return GPIO_ERR_INVALID_PINS;
	}

	const gpio_reset_vals_t* rv = &gpio_reset_vals[idx];
	gpio_exti_release(pins, gpio_ahb1_bit[idx]);

	for (uint32_t n = 0U; n < 16U; n++) {
		if ((pins & (1U << n)) == 0U) {
			continue;
		}
		const uint32_t sh2 = n * 2U;
		volatile uint32_t* afr = (n < 8U) ? &port->AFRL : &port->AFRH;
		MODIFY_FIELD(*afr, 4U, (n % 8U) * 4U, 0U);
		MODIFY_FIELD(port->OTYPER, 1U, n, 0U);
		MODIFY_FIELD(port->OSPEEDR, 2U, sh2, rv->ospeedr >> sh2);
		MODIFY_FIELD(port->PUPDR, 1U, sh2, rv->pupdr >> sh2);
		MODIFY_FIELD(port->MODER, 1U, sh2, rv->moder >> sh2);
	}

	port->BSRR = (uint32_t)pins << 16U;
	return GPIO_OK;
}

GPIO_status_t GPIO_port_deinit(GPIO_TypeDef* port)
{
	if (port == NULL) {
		return GPIO_ERR_NULL_PTR;
	}

	const int idx = gpio_port_index(port);
	if (idx < 0) {
		return GPIO_ERR_INVALID_PORT;
	}

	gpio_exti_release(GPIO_PIN_ALL, gpio_ahb1_bit[idx]);

	SET_BIT(RCC->AHB1RSTR, gpio_ahb1_bit[idx]);
	CLEAR_BIT(RCC->AHB1RSTR, gpio_ahb1_bit[idx]);
	CLEAR_BIT(RCC->AHB1ENR, gpio_ahb1_bit[idx]);

	return GPIO_OK;
}

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
