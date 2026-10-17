#include "stm32f401xx_I2C_driver.h"
#include "stm32f401xx.h"
#include "stm32f401xx_RCC_driver.h"

/********************************************************************************
 *
 * TODO: Future plans for this driver:
 * 1. ...
 *
 *******************************************************************************/

/********************************************************************************
 *
 * INFO: Driver map
 * @PERIPHERAL_CLOCK_SETUP
 * @INIT_DE-INIT
 * @DATA_SEND_RECEIVE_POLLING_INTERRUPT
 * @PERIPHERAL_CONTROL_API
 * @IRQ_CONFIGURATION_AND_ISR_HANDLING
 * @APPLICATION_CALLBACK
 *
 *******************************************************************************/

// NOTE: @PERIPHERAL_CLOCK_SETUP

/********************************************************************************
 * @fn				- I2C_peri_clk_control
 *
 * @brief			- Enables or disables peripheral clock for SPI
 *
 * @param[*p_USART_x]		- Base address of the SPI peripheral
 * @param[EN_or_DI]		- ENABLE or DISABLE macros
 *
 * @return			- Success / Failure status of the function
 *
 * @Note			- None
 *******************************************************************************/

I2C_status_t I2C_peri_clk_control(I2C_TypeDef* p_I2C_x, uint8_t EN_or_DI)
{
	VALIDATE_EN_DI(EN_or_DI, I2C_ERROR_INVALID_STATE);

	I2C_status_t status = I2C_ERROR_INVALID_PORT;

	if (p_I2C_x == I2C1) {
		if (EN_or_DI == ENABLE) {
			I2C1_PCLK_EN();
		}
		else
			I2C1_PCLK_DI();
		status = I2C_OK;
	}
	else if (p_I2C_x == I2C2) {
		if (EN_or_DI == ENABLE) {
			I2C2_PCLK_EN();
		}
		else
			I2C2_PCLK_DI();
		status = I2C_OK;
	}
	else if (p_I2C_x == I2C3) {
		if (EN_or_DI == ENABLE) {
			I2C3_PCLK_EN();
		}
		else
			I2C3_PCLK_DI();
		status = I2C_OK;
	}

	return status;
}

// NOTE: @INIT_DE-INIT

/********************************************************************************
 * @fn				- I2C_init
 *
 * @brief			- Initializes a I2C peripheral
 *
 * @param[*p_USART_handle]	- Handle structure of a I2C peripheral
 *
 * @return			- Success / Failure status of the function
 *
 * @Note			- None
 *******************************************************************************/

I2C_status_t I2C_init(I2C_Handle_t* p_I2C_handle)
{
	VALIDATE_PTR(p_I2C_handle, I2C_ERROR_NULL_PTR);

	I2C_TypeDef* port = p_I2C_handle->p_I2Cx;
	VALIDATE_I2C_PORT(port);

	volatile uint32_t* cr1 = &p_I2C_handle->p_I2Cx->CR1;
	volatile uint32_t* cr2 = &p_I2C_handle->p_I2Cx->CR2;
	volatile uint32_t* oar1 = &p_I2C_handle->p_I2Cx->OAR1;
	volatile uint32_t* ccr = &p_I2C_handle->p_I2Cx->CCR;

	// Clock
	I2C_peri_clk_control(port, ENABLE);

	// Speed
	uint8_t speed = p_I2C_handle->I2C_Config.I2C_SCL_Speed;

	uint16_t ccr_val = 0;
	if (speed <= I2C_SCL_SPEED_SM) {
		CLEAR_BIT(*ccr, I2C_CCR_FS);
		ccr_val = RCC_get_pclk_1_val() / (2 * speed);
		ccr_val &= 0xFFF;
		SET_BITS_BY_VAR(*ccr, ccr_val);
	}
	else if (speed <= I2C_SCL_SPEED_FM2K || speed <= I2C_SCL_SPEED_FM4K) {
		SET_BIT(*ccr, I2C_CCR_FS);

		// Duty cycle
		uint8_t duty_cycle = p_I2C_handle->I2C_Config.I2C_FM_Duty_Cycle;
		if (duty_cycle == I2C_FM_DUTY_CYCLE_16_9) {
			SET_BIT(*ccr, I2C_CCR_DUTY);
			ccr_val = RCC_get_pclk_1_val() / (25 * speed);
		}
		else if (duty_cycle == I2C_FM_DUTY_CYCLE_2) {
			CLEAR_BIT(*ccr, I2C_CCR_DUTY);
			ccr_val = RCC_get_pclk_1_val() / (3 * speed);
		}
		ccr_val &= 0xFFF;
		SET_BITS_BY_VAR(*ccr, ccr_val);
	}

	// Frequency
	uint32_t freq_val = RCC_get_pclk_1_val() / 100000U;
	SET_BITS_BY_VAR(*cr2, freq_val);

	// Device address
	SET_BITS_BY_VAR(*oar1,
	                p_I2C_handle->I2C_Config.I2C_Device_Address << 1);
	// Required by software
	SET_BIT(*oar1, 14);

	// ACKing
	uint8_t ack = p_I2C_handle->I2C_Config.I2C_ACK_Control;

	VALIDATE_I2C_ACK_CONTROL(ack);

	if (ack == I2C_ACK_CONTROL_ENABLE)
		SET_BIT(*cr1, I2C_CR1_ACK);
	else if (ack == I2C_ACK_CONTROL_DISABLE)
		CLEAR_BIT(*cr1, I2C_CR1_ACK);

	// Slew rate
	uint8_t trise_val;
	volatile uint32_t* trise = &p_I2C_handle->p_I2Cx->TRISE;
	if (speed <= I2C_SCL_SPEED_SM) {
		trise_val = (RCC_get_pclk_1_val() / 1000000) + 1;
	}
	else if (speed <= I2C_SCL_SPEED_FM2K || speed <= I2C_SCL_SPEED_FM4K) {
		trise_val = ((RCC_get_pclk_1_val() * 300) / 1000000000) + 1;
	}
	SET_BITS_BY_VAR(*trise, trise_val);

	return I2C_OK;
}

/********************************************************************************
 * @fn				- I2C_de_init
 *
 * @brief			- Resets a I2C peripheral
 *
 * @param[*p_USART_x]		- Base address of the I2C peripheral
 *
 * @return			- Success / Failure status of the function
 *
 * @Note			- None
 *******************************************************************************/

I2C_status_t I2C_de_init(I2C_TypeDef* p_I2C_x)
{
	I2C_status_t status = I2C_ERROR_INVALID_PORT;

	if (p_I2C_x == I2C1) {
		I2C1_PER_RESET();
		I2C1_CLK_DISABLE();
		status = I2C_OK;
	}
	else if (p_I2C_x == I2C2) {
		I2C2_PER_RESET();
		I2C2_CLK_DISABLE();
		status = I2C_OK;
	}
	else if (p_I2C_x == I2C3) {
		I2C3_PER_RESET();
		I2C3_CLK_DISABLE();
		status = I2C_OK;
	}

	return status;
}

// NOTE: @DATA_SEND_RECEIVE_POLLING_INTERRUPT

// NOTE: @PERIPHERAL_CONTROL_API

/********************************************************************************
 * @fn				- I2C_peri_control
 *
 * @brief			- Sets I2C peripheral control
 *
 * @param[*p_I2C_x]		- Base address of the I2C peripheral
 * @param[EN_or_DI]		- ENABLE or DISABLE macros
 *
 * @return			- Success / Failure status of the function
 *
 * @Note			- None
 *******************************************************************************/

I2C_status_t I2C_peri_control(I2C_TypeDef* p_I2C_x, uint8_t EN_or_DI)
{
	VALIDATE_EN_DI(EN_or_DI, I2C_ERROR_INVALID_STATE);

	VALIDATE_I2C_PORT(p_I2C_x);

	if (EN_or_DI == ENABLE) {
		SET_BIT(p_I2C_x->CR1, I2C_CR1_PE);
	}
	else {
		CLEAR_BIT(p_I2C_x->CR1, I2C_CR1_PE);
	}

	return I2C_OK;
}

/********************************************************************************
 * @fn				- I2C_get_flag_status
 *
 * @brief			- Returns flag status of a I2C flag
 *
 * @param[*p_I2C_x]		- Base address of the I2C peripheral
 * @param[status_flag_name]	- Flag name
 *
 * @return			- 1 if flag set, 0 if not set
 *
 * @Note			- None
 *******************************************************************************/

uint8_t I2C_get_flag_status(I2C_TypeDef* p_I2C_x, uint8_t status_flag_name)
{
	return IS_BIT_SET(p_I2C_x->SR1, status_flag_name) ||
	       IS_BIT_SET(p_I2C_x->SR2, status_flag_name);
}

// NOTE: IRQ_CONFIGURATION_AND_ISR_HANDLING

/********************************************************************************
 * @fn				- I2C_irq_interrupt_config
 *
 * @brief			- Configures I2C IRQ interrupt
 *
 * @param[irq_n]		- IRQ number
 * @param[EN_or_DI]		- ENABLE or DISABLE macros
 *
 * @return			- Success / Failure status of the function
 *
 * @Note			- None
 *******************************************************************************/

I2C_status_t I2C_irq_interrupt_config(uint8_t irq_n, uint8_t EN_or_DI)
{
	VALIDATE_EN_DI(EN_or_DI, I2C_ERROR_INVALID_STATE);

	VALIDATE_IRQ_NUMBER(irq_n, I2C_ERROR_INVALID_IRQ);

	if (EN_or_DI == ENABLE) {
		if (irq_n < 32)
			SET_BIT(NVIC->ISER[0], irq_n);
		else if (irq_n >= 32 && irq_n < 64)
			SET_BIT(NVIC->ISER[1], (irq_n % 32));
		else if (irq_n >= 64 && irq_n < 96)
			SET_BIT(NVIC->ISER[2], (irq_n % 64));
	}
	else {
		if (irq_n < 32)
			SET_BIT(NVIC->ICER[0], irq_n);
		else if (irq_n >= 32 && irq_n < 64)
			SET_BIT(NVIC->ICER[1], (irq_n % 32));
		else if (irq_n >= 64 && irq_n < 96)
			SET_BIT(NVIC->ICER[2], (irq_n % 64));
	}

	return I2C_OK;
}

/********************************************************************************
 * @fn				- I2C_irq_priority_config
 *
 * @brief			- Configures I2C IRQ priority
 *
 * @param[irq_n]		- IRQ number
 * @param[irq_prio]		- IRQ priority
 *
 * @return			- Success / Failure status of the function
 *
 * @Note			- None
 *******************************************************************************/

I2C_status_t I2C_irq_priority_config(uint8_t irq_n, uint32_t irq_prio)
{
	VALIDATE_IRQ_NUMBER(irq_n, I2C_ERROR_INVALID_IRQ);

	uint8_t ipr_x = irq_n / 4;
	uint8_t ipr_x_section = (irq_n % 4) * 8;
	uint32_t prio_field = (irq_prio & ((1U << __NVIC_PRIO_BITS) - 1U))
	                      << (8U - __NVIC_PRIO_BITS);
	uint32_t ipr = NVIC->IPR[ipr_x];
	CLEAR_BYTE(ipr, ipr_x_section);
	ipr |= (prio_field << ipr_x_section);
	NVIC->IPR[ipr_x] = ipr;

	return I2C_OK;
}

// NOTE: @APPLICATION_CALLBACK

/********************************************************************************
 * @fn				- I2C_application_event_callback
 *
 * @brief			- Application event callback function
 *
 * @param[*p_I2C_handle]	- Handle structure of a I2C peripheral
 * @param[app_ev]		- Application event
 *
 * @return			- None
 *
 * @Note			- None
 *******************************************************************************/

__attribute__((weak)) void
I2C_application_event_callback(I2C_Handle_t* p_I2C_handle,
                               I2C_AppEvent_t app_ev)
{
	/* This is a week implementation. The application may override
	 * this function. */
}
