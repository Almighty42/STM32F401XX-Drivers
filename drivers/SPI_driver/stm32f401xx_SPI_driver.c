#include "stm32f401xx_SPI_driver.h"
#include "stm32f401xx.h"

/********************************************************************************
 *
 * TODO: Future plans for this driver:
 * 1. Add helper functions to longer functions to make the code more readable
 * 2. Implement DMA USART in future
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
 * @fn				- SPI_peri_clk_control
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

SPI_status_t SPI_peri_clk_control(SPI_TypeDef* p_SPI_x, uint8_t EN_or_DI)
{
	VALIDATE_EN_DI(EN_or_DI, SPI_ERROR_INVALID_STATE);

	SPI_status_t status = SPI_ERROR_INVALID_PORT;

	if (p_SPI_x == SPI1) {
		if (EN_or_DI == ENABLE) {
			SPI1_PCLK_EN();
		}
		else
			SPI1_PCLK_DI();
		status = SPI_OK;
	}
	else if (p_SPI_x == SPI2) {
		if (EN_or_DI == ENABLE) {
			SPI2_PCLK_EN();
		}
		else
			SPI2_PCLK_DI();
		status = SPI_OK;
	}
	else if (p_SPI_x == SPI3) {
		if (EN_or_DI == ENABLE) {
			SPI3_PCLK_EN();
		}
		else
			SPI3_PCLK_DI();
		status = SPI_OK;
	}
	else if (p_SPI_x == SPI4) {
		if (EN_or_DI == ENABLE) {
			SPI4_PCLK_EN();
		}
		else
			SPI4_PCLK_DI();
		status = SPI_OK;
	}

	return status;
}

// NOTE: @INIT_DE-INIT

/********************************************************************************
 * @fn				- SPI_init
 *
 * @brief			- Initializes a SPI peripheral
 *
 * @param[*p_USART_handle]	- Handle structure of a SPI peripheral
 *
 * @return			- Success / Failure status of the function
 *
 * @Note			- None
 *******************************************************************************/

SPI_status_t SPI_init(SPI_Handle_t* p_SPI_handle)
{
	VALIDATE_PTR(p_SPI_handle, SPI_ERROR_NULL_PTR);

	SPI_TypeDef* port = p_SPI_handle->p_SPIx;
	VALIDATE_SPI_PORT(port);

	volatile uint32_t* cr1 = &p_SPI_handle->p_SPIx->CR1;

	// Clock
	SPI_peri_clk_control(port, ENABLE);

	// Mode
	uint8_t mode = p_SPI_handle->SPI_Config.SPI_Device_Mode;

	VALIDATE_SPI_DEVICE_MODE(mode);

	CLEAR_BIT(*cr1, SPI_CR1_MSTR);
	if (mode == SPI_DEVICE_MODE_MASTER)
		SET_BIT(*cr1, SPI_CR1_MSTR);
	else if (mode == SPI_DEVICE_MODE_SLAVE)
		CLEAR_BIT(*cr1, SPI_CR1_MSTR);

	uint8_t bus_config = p_SPI_handle->SPI_Config.SPI_Bus_Config;

	VALIDATE_SPI_BUS_CONFIG(bus_config);

	if (bus_config == SPI_BUS_CONFIG_FD)
		CLEAR_BIT(*cr1, SPI_CR1_BIDIMODE);
	else if (bus_config == SPI_BUS_CONFIG_HD) {
		SET_BIT(*cr1, SPI_CR1_BIDIMODE);
	}
	else if (bus_config == SPI_BUS_CONFIG_SIMPLEX_RX_ONLY) {
		CLEAR_BIT(*cr1, SPI_CR1_BIDIMODE);
		SET_BIT(*cr1, SPI_CR1_RXONLY);
	}
	else if (bus_config == SPI_BUS_CONFIG_SIMPLEX_TX_ONLY) {
		CLEAR_BIT(*cr1, SPI_CR1_BIDIMODE);
		CLEAR_BIT(*cr1, SPI_CR1_RXONLY);
	}

	// Speed
	uint8_t speed = p_SPI_handle->SPI_Config.SPI_SCLK_Speed;

	VALIDATE_SPI_SCLK_SPEED(speed);

	if (speed == SPI_SCLK_SPEED_DIV_2) {
		CLEAR_BIT(*cr1, SPI_CR1_BR);
		CLEAR_BIT(*cr1, (SPI_CR1_BR + 1));
		CLEAR_BIT(*cr1, (SPI_CR1_BR + 2));
	}
	else if (speed == SPI_SCLK_SPEED_DIV_4) {
		SET_BIT(*cr1, SPI_CR1_BR);
		CLEAR_BIT(*cr1, (SPI_CR1_BR + 1));
		CLEAR_BIT(*cr1, (SPI_CR1_BR + 2));
	}
	else if (speed == SPI_SCLK_SPEED_DIV_8) {
		CLEAR_BIT(*cr1, SPI_CR1_BR);
		SET_BIT(*cr1, (SPI_CR1_BR + 1));
		CLEAR_BIT(*cr1, (SPI_CR1_BR + 2));
	}
	else if (speed == SPI_SCLK_SPEED_DIV_16) {
		SET_BIT(*cr1, SPI_CR1_BR);
		SET_BIT(*cr1, (SPI_CR1_BR + 1));
		CLEAR_BIT(*cr1, (SPI_CR1_BR + 2));
	}
	else if (speed == SPI_SCLK_SPEED_DIV_32) {
		CLEAR_BIT(*cr1, SPI_CR1_BR);
		CLEAR_BIT(*cr1, (SPI_CR1_BR + 1));
		SET_BIT(*cr1, (SPI_CR1_BR + 2));
	}
	else if (speed == SPI_SCLK_SPEED_DIV_64) {
		SET_BIT(*cr1, SPI_CR1_BR);
		CLEAR_BIT(*cr1, (SPI_CR1_BR + 1));
		SET_BIT(*cr1, (SPI_CR1_BR + 2));
	}
	else if (speed == SPI_SCLK_SPEED_DIV_128) {
		CLEAR_BIT(*cr1, SPI_CR1_BR);
		SET_BIT(*cr1, (SPI_CR1_BR + 1));
		SET_BIT(*cr1, (SPI_CR1_BR + 2));
	}
	else if (speed == SPI_SCLK_SPEED_DIV_256) {
		SET_BIT(*cr1, SPI_CR1_BR);
		SET_BIT(*cr1, (SPI_CR1_BR + 1));
		SET_BIT(*cr1, (SPI_CR1_BR + 2));
	}

	// DFF
	uint8_t dff = p_SPI_handle->SPI_Config.SPI_DFF;

	VALIDATE_SPI_DFF(dff);

	CLEAR_BIT(*cr1, SPI_CR1_DFF);
	if (dff == SPI_DFF_16BIT)
		SET_BIT(*cr1, SPI_CR1_DFF);
	else if (dff == SPI_DFF_8BIT)
		CLEAR_BIT(*cr1, SPI_CR1_DFF);

	// CPOL
	uint8_t cpol = p_SPI_handle->SPI_Config.SPI_CPOL;

	VALIDATE_SPI_CPOL(cpol);

	CLEAR_BIT(*cr1, SPI_CR1_CPOL);
	if (cpol == SPI_CPOL_HIGH)
		SET_BIT(*cr1, SPI_CR1_CPOL);
	else if (cpol == SPI_CPOL_LOW)
		CLEAR_BIT(*cr1, SPI_CR1_CPOL);

	// CPHA
	uint8_t cpha = p_SPI_handle->SPI_Config.SPI_CPHA;

	VALIDATE_SPI_CPHA(cpha);

	CLEAR_BIT(*cr1, SPI_CR1_CPHA);
	if (cpha == SPI_CPHA_HIGH)
		SET_BIT(*cr1, SPI_CR1_CPHA);
	else if (cpha == SPI_CPHA_LOW)
		CLEAR_BIT(*cr1, SPI_CR1_CPHA);

	// SSM
	uint8_t ssm = p_SPI_handle->SPI_Config.SPI_SSM;

	VALIDATE_SPI_SSM(ssm);

	CLEAR_BIT(*cr1, SPI_CR1_SSM);
	if (ssm == SPI_SSM_SW) {
		SET_BIT(*cr1, SPI_CR1_SSM);
		SET_BIT(*cr1, SPI_CR1_SSI);
	}
	else if (ssm == SPI_SSM_HW)
		CLEAR_BIT(*cr1, SPI_CR1_SSM);

	return SPI_OK;
}

/********************************************************************************
 * @fn				- SPI_de_init
 *
 * @brief			- Resets a SPI peripheral
 *
 * @param[*p_SPI_x]		- Base address of the SPI peripheral
 *
 * @return			- Success / Failure status of the function
 *
 * @Note			- None
 *******************************************************************************/

SPI_status_t SPI_de_init(SPI_TypeDef* p_SPI_x)
{
	SPI_status_t status = SPI_ERROR_INVALID_PORT;

	if (p_SPI_x == SPI1) {
		SPI1_PER_RESET();
		SPI1_CLK_DISABLE();
		status = SPI_OK;
	}
	else if (p_SPI_x == SPI2) {
		SPI2_PER_RESET();
		SPI2_CLK_DISABLE();
		status = SPI_OK;
	}
	else if (p_SPI_x == SPI3) {
		SPI3_PER_RESET();
		SPI3_CLK_DISABLE();
		status = SPI_OK;
	}
	else if (p_SPI_x == SPI4) {
		SPI4_PER_RESET();
		SPI4_CLK_DISABLE();
		status = SPI_OK;
	}

	return status;
}

// NOTE: @DATA_SEND_RECEIVE_POLLING_INTERRUPT

/********************************************************************************
 * @fn				- SPI_write_data_pl
 *
 * @brief			- Write data over USART using a polling method
 *
 * @param[*p_SPI_x]		- Base address of the SPI peripheral
 * @param[*p_tx_buffer]		- Transmission buffer ( pointer )
 * @param[len]			- Number of bytes to send
 *
 * @return			- Success / Failure status of the function
 *
 * @Note			- None
 *******************************************************************************/

SPI_status_t SPI_write_data_pl(SPI_Handle_t* p_SPI_handle,
                               const uint8_t* p_tx_buffer, uint32_t len)
{
	VALIDATE_PTR(p_SPI_handle, SPI_ERROR_NULL_PTR);
	VALIDATE_PTR(p_tx_buffer, SPI_ERROR_NULL_PTR);

	while (len > 0) {
		// wait until TXE == 1
		while (!(p_SPI_handle->p_SPIx->SR & SPI_SR_TXE_STATE)) {
			// spin
		}

		if (p_SPI_handle->p_SPIx->CR1 & SPI_CR1_DFF) {
			p_SPI_handle->p_SPIx->DR = *((uint16_t*)p_tx_buffer);
			len -= 2;
			p_tx_buffer += 2;
		}
		else {
			p_SPI_handle->p_SPIx->DR = *p_tx_buffer;
			len--;
			p_tx_buffer++;
		}
	}

	// wait until BSY == 0 (last bit shifted out)
	while (p_SPI_handle->p_SPIx->SR & SPI_SR_BSY) {
		// spin
	}

	return SPI_OK;
}

/********************************************************************************
 * @fn				- SPI_read_data_pl
 *
 * @brief			- Reads data over SPI using a polling method
 *
 * @param[*p_SPI_x]		- Base address of the SPI peripheral
 * @param[*p_rx_buffer]		- Receive buffer ( pointer )
 * @param[len]			- Number of bytes to receive
 *
 * @return			- Success / Failure status of the function
 *
 * @Note			- None
 *******************************************************************************/

SPI_status_t SPI_read_data_pl(SPI_Handle_t* p_SPI_handle, uint8_t* p_rx_buffer,
                              uint32_t len);

/********************************************************************************
 * @fn				- SPI_write_data_it
 *
 * @brief			- Enables or disables peripheral clock for USART
 *
 * @param[*p_SPI_handle]	- Handle structure of a SPI peripheral
 * @param[*p_tx_buffer]		- Transmission buffer ( pointer )
 * @param[len]			- Number of bytes to send
 *
 * @return			- Success / Failure status of the function
 *
 * @Note			- None
 *******************************************************************************/

SPI_status_t SPI_write_data_it(SPI_Handle_t* p_SPI_handle, uint8_t* p_tx_buffer,
                               uint32_t len)
{
	VALIDATE_PTR(p_SPI_handle, SPI_ERROR_NULL_PTR);
	VALIDATE_PTR(p_tx_buffer, SPI_ERROR_NULL_PTR);

	VALIDATE_SPI_PORT(p_SPI_handle->p_SPIx);
	VALIDATE_SPI_ENABLED(p_SPI_handle->p_SPIx);

	if (len == 0U)
		return SPI_OK;

	uint8_t tx_state = p_SPI_handle->tx_busy_state;

	if (tx_state != SPI_BUSY_IN_TX) {
		p_SPI_handle->tx_len = len;
		p_SPI_handle->p_tx_buffer = p_tx_buffer;
		p_SPI_handle->tx_busy_state = SPI_BUSY_IN_TX;
		p_SPI_handle->op_mode = SPI_OP_TX_ONLY;

		// Enabling interrupt for TXE
		SET_BIT(p_SPI_handle->p_SPIx->CR2, SPI_CR2_TXEIE);
	}
	else
		return SPI_BUSY;

	return SPI_OK;
}

/********************************************************************************
 * @fn				- SPI_read_data_it
 *
 * @brief			- Reads data over SPI via interrupts
 *
 * @param[*p_SPI_handle]	- Handle structure of a SPI peripheral
 * @param[*p_rx_buffer]		- Receive buffer ( pointer )
 * @param[len]			- Number of bytes to receive
 *
 * @return			- Success / Failure status of the function
 *
 * @Note			- None
 *******************************************************************************/

SPI_status_t SPI_read_data_it(SPI_Handle_t* p_SPI_handle, uint8_t* p_rx_buffer,
                              uint32_t len)
{
	VALIDATE_PTR(p_SPI_handle, SPI_ERROR_NULL_PTR);
	VALIDATE_PTR(p_rx_buffer, SPI_ERROR_NULL_PTR);

	VALIDATE_SPI_PORT(p_SPI_handle->p_SPIx);
	VALIDATE_SPI_ENABLED(p_SPI_handle->p_SPIx);

	if (len == 0U)
		return SPI_OK;

	uint8_t rx_state = p_SPI_handle->rx_busy_state;

	if (rx_state != SPI_BUSY_IN_RX) {
		p_SPI_handle->rx_len = len;
		p_SPI_handle->p_rx_buffer = p_rx_buffer;
		p_SPI_handle->rx_busy_state = SPI_BUSY_IN_RX;
		p_SPI_handle->op_mode = SPI_OP_RX_ONLY;

		// Enabling interrupt for RXNEIE
		SET_BIT(p_SPI_handle->p_SPIx->CR2, SPI_CR2_RXNEIE);
	}
	else
		return SPI_BUSY;

	return SPI_OK;
}

/********************************************************************************
 * @fn				- SPI_transfer_data_it
 *
 * @brief			- Sends and receives over SPI via interrupts
 *
 * @param[*p_SPI_handle]	- Handle structure of a SPI peripheral
 * @param[*p_tx_buffer]		- Transfer buffer ( pointer )
 * @param[*p_rx_buffer]		- Receive buffer ( pointer )
 * @param[len]			- Number of bytes to receive
 *
 * @return			- Success / Failure status of the function
 *
 * @Note			- Used for Full-duplex communication
 *******************************************************************************/

SPI_status_t SPI_transfer_data_it(SPI_Handle_t* p_SPI_handle,
                                  uint8_t* p_tx_buffer, uint8_t* p_rx_buffer,
                                  uint32_t len)
{
	VALIDATE_PTR(p_SPI_handle, SPI_ERROR_NULL_PTR);
	// VALIDATE_PTR(p_tx_buffer, SPI_ERROR_NULL_PTR);
	// VALIDATE_PTR(p_rx_buffer, SPI_ERROR_NULL_PTR);

	VALIDATE_SPI_PORT(p_SPI_handle->p_SPIx);
	VALIDATE_SPI_ENABLED(p_SPI_handle->p_SPIx);

	if (len == 0U)
		return SPI_OK;

	uint8_t tx_state = p_SPI_handle->tx_busy_state;
	uint8_t rx_state = p_SPI_handle->rx_busy_state;

	if (tx_state != SPI_READY || rx_state != SPI_READY)
		return SPI_BUSY;

	// For DFF 16 bit, checks if number is even
	if (p_SPI_handle->SPI_Config.SPI_DFF == SPI_DFF_16BIT && (len & 0x1))
		return SPI_ERROR_INVALID_LEN;

	p_SPI_handle->p_tx_buffer = p_tx_buffer;
	p_SPI_handle->tx_len = len;
	p_SPI_handle->tx_busy_state = SPI_BUSY_IN_TX;

	if (p_rx_buffer) {
		p_SPI_handle->p_rx_buffer = p_rx_buffer;
		p_SPI_handle->rx_len = len;
		p_SPI_handle->rx_busy_state = SPI_BUSY_IN_RX;

		if (p_tx_buffer)
			p_SPI_handle->op_mode = SPI_OP_TX_RX;
		else
			p_SPI_handle->op_mode = SPI_OP_RX_ONLY;

		SET_BIT(p_SPI_handle->p_SPIx->CR2, SPI_CR2_RXNEIE);
	}
	else {
		p_SPI_handle->p_rx_buffer = NULL;
		p_SPI_handle->rx_len = 0;
		p_SPI_handle->rx_busy_state = SPI_READY;
		p_SPI_handle->op_mode = SPI_OP_TX_ONLY;

		CLEAR_BIT(p_SPI_handle->p_SPIx->CR2, SPI_CR2_RXNEIE);
	}

	if (!p_tx_buffer && !p_rx_buffer)
		return SPI_ERROR_INVALID_STATE;

	// Enabling interrupt for TXE
	SET_BIT(p_SPI_handle->p_SPIx->CR2, SPI_CR2_TXEIE);

	return SPI_OK;
}

// NOTE: @PERIPHERAL_CONTROL_API

/********************************************************************************
 * @fn				- SPI_peri_control
 *
 * @brief			- Sets SPI peripheral control
 *
 * @param[*p_SPI_x]		- Base address of the SPI peripheral
 * @param[EN_or_DI]		- ENABLE or DISABLE macros
 *
 * @return			- Success / Failure status of the function
 *
 * @Note			- None
 *******************************************************************************/

SPI_status_t SPI_peri_control(SPI_TypeDef* p_SPI_x, uint8_t EN_or_DI)
{
	VALIDATE_EN_DI(EN_or_DI, SPI_ERROR_INVALID_STATE);

	VALIDATE_SPI_PORT(p_SPI_x);

	if (EN_or_DI == ENABLE) {
		SET_BIT(p_SPI_x->CR1, SPI_CR1_SPE);
	}
	else {
		CLEAR_BIT(p_SPI_x->CR1, SPI_CR1_SPE);
	}

	return SPI_OK;
}

// NOTE: IRQ_CONFIGURATION_AND_ISR_HANDLING

/********************************************************************************
 * @fn				- SPI_irq_interrupt_config
 *
 * @brief			- Configures SPI IRQ interrupt
 *
 * @param[irq_n]		- IRQ number
 * @param[EN_or_DI]		- ENABLE or DISABLE macros
 *
 * @return			- Success / Failure status of the function
 *
 * @Note			- None
 *******************************************************************************/

SPI_status_t SPI_irq_interrupt_config(uint8_t irq_n, uint8_t EN_or_DI)
{
	VALIDATE_EN_DI(EN_or_DI, SPI_ERROR_INVALID_STATE);

	VALIDATE_IRQ_NUMBER(irq_n, SPI_ERROR_INVALID_IRQ);

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

	return SPI_OK;
}

/********************************************************************************
 * @fn				- SPI_irq_priority_config
 *
 * @brief			- Configures SPI IRQ priority
 *
 * @param[irq_n]		- IRQ number
 * @param[irq_prio]		- IRQ priority
 *
 * @return			- Success / Failure status of the function
 *
 * @Note			- None
 *******************************************************************************/

SPI_status_t SPI_irq_priority_config(uint8_t irq_n, uint32_t irq_prio)
{
	VALIDATE_IRQ_NUMBER(irq_n, SPI_ERROR_INVALID_IRQ);

	uint8_t ipr_x = irq_n / 4;
	uint8_t ipr_x_section = (irq_n % 4) * 8;
	uint32_t prio_field = (irq_prio & ((1U << __NVIC_PRIO_BITS) - 1U))
	                      << (8U - __NVIC_PRIO_BITS);
	uint32_t ipr = NVIC->IPR[ipr_x];
	CLEAR_BYTE(ipr, ipr_x_section);
	ipr |= (prio_field << ipr_x_section);
	NVIC->IPR[ipr_x] = ipr;

	return SPI_OK;
}

/********************************************************************************
 * @fn				- SPI_irq_handling
 *
 * @brief			- Handles SPI interrupts
 *
 * @param[*p_SPI_handle]	- Handle structure of a SPI peripheral
 *
 * @return			- None
 *
 * @Note			- None
 *******************************************************************************/

void SPI_irq_handling(SPI_Handle_t* p_SPI_handle)
{
	volatile uint32_t* sr = &p_SPI_handle->p_SPIx->SR;
	volatile uint32_t* cr2 = &p_SPI_handle->p_SPIx->CR2;
	volatile uint32_t* dr = &p_SPI_handle->p_SPIx->DR;

	// TXE interrupt

	// Check for TXE flag
	uint32_t txe_state = IS_BIT_SET(*sr, SPI_SR_TXE_STATE);
	// Check for TXEIE flag
	uint32_t txeie_state = IS_BIT_SET(*cr2, SPI_CR2_TXEIE);

	if (txe_state && txeie_state) {
		if (p_SPI_handle->tx_len > 0) {
			uint8_t dff = p_SPI_handle->SPI_Config.SPI_DFF;
			if (p_SPI_handle->op_mode == SPI_OP_RX_ONLY) {
				// Sending dummy data to start SCK
				if (dff == SPI_DFF_8BIT) {
					*dr = 0xFF;
					p_SPI_handle->tx_len--;
				}
				else {
					*dr = 0xFFFF;
					p_SPI_handle->tx_len -= 2;
				}
			}
			else {
				// TX ONLY or TX_RX
				if (dff == SPI_DFF_8BIT) {
					*dr = *(p_SPI_handle->p_tx_buffer);
					p_SPI_handle->p_tx_buffer++;
					p_SPI_handle->tx_len--;
				}
				else if (dff == SPI_DFF_16BIT) {
					// TODO: TEST THIS, MIGHT CAUSE A ISSUE
					*dr = *(uint16_t*)(p_SPI_handle
					                       ->p_tx_buffer);
					p_SPI_handle->p_tx_buffer += 2;
					p_SPI_handle->tx_len -= 2;
				}
			}
		}
		else {
			p_SPI_handle->tx_busy_state = SPI_READY;
			CLEAR_BIT(*cr2, SPI_CR2_TXEIE);

			if (p_SPI_handle->op_mode == SPI_OP_TX_ONLY)
				SPI_application_event_callback(
				    p_SPI_handle, SPI_APP_EVENT_TX_CMPLT);
		}
	}

	// RXNE interrupt

	// Check for RXNE flag
	uint32_t rxne_state = IS_BIT_SET(*sr, SPI_SR_RXNE_STATE);
	// Check for RXNEIE flag
	uint32_t rxneie_state = IS_BIT_SET(*cr2, SPI_CR2_RXNEIE);

	if (rxne_state && rxneie_state) {
		if (p_SPI_handle->rx_len > 0) {
			uint8_t dff = p_SPI_handle->SPI_Config.SPI_DFF;
			if (p_SPI_handle->p_rx_buffer) {
				if (dff == SPI_DFF_8BIT) {
					uint8_t data = (uint8_t)(*dr & 0xFF);
					*(p_SPI_handle->p_rx_buffer) = data;
					p_SPI_handle->p_rx_buffer++;
					p_SPI_handle->rx_len--;
				}
				else if (dff == SPI_DFF_16BIT) {
					uint16_t data =
					    (uint16_t)(*dr & 0xFFFF);
					*(uint16_t*)(p_SPI_handle
					                 ->p_rx_buffer) = data;
					p_SPI_handle->p_rx_buffer += 2;
					p_SPI_handle->rx_len -= 2;
				}
			}
			else {
				(void)*dr;
				if (dff == SPI_DFF_8BIT)
					p_SPI_handle->rx_len--;
				else
					p_SPI_handle->rx_len -= 2;
			}
		}
		else {
			p_SPI_handle->rx_busy_state = SPI_READY;
			CLEAR_BIT(*cr2, SPI_CR2_RXNEIE);

			if (p_SPI_handle->op_mode == SPI_OP_TX_RX ||
			    p_SPI_handle->op_mode == SPI_OP_RX_ONLY)
				SPI_application_event_callback(
				    p_SPI_handle, SPI_APP_EVENT_RX_CMPLT);
		}
	}

	// Error interrupt

	uint32_t errie_state = IS_BIT_SET(*cr2, SPI_CR2_ERRIE);
	if (errie_state) {
		// OVR
		uint32_t ovr_state = IS_BIT_SET(*sr, SPI_SR_OVR_STATE);
		if (ovr_state) {
			(void)*dr;
			(void)*sr;
			SPI_application_event_callback(p_SPI_handle,
			                               SPI_APP_ERR_OVR);
		}
		// MODF
		uint32_t modf_state = IS_BIT_SET(*sr, SPI_SR_MODF_STATE);
		if (modf_state) {
			(void)*sr;
			SPI_application_event_callback(p_SPI_handle,
			                               SPI_APP_ERR_MODF);
		}
		// CRCERR
		uint32_t crcerr_state = IS_BIT_SET(*sr, SPI_SR_CRCERR_STATE);
		if (crcerr_state) {
			CLEAR_BIT(*sr, SPI_SR_CRCERR_STATE);
			SPI_application_event_callback(p_SPI_handle,
			                               SPI_APP_ERR_CRC);
		}
	}
}

// NOTE: @APPLICATION_CALLBACK

/********************************************************************************
 * @fn				- SPI_application_event_callback
 *
 * @brief			- Application event callback function
 *
 * @param[*p_SPI_handle]	- Handle structure of a SPI peripheral
 * @param[app_ev]		- Application event
 *
 * @return			- None
 *
 * @Note			- None
 *******************************************************************************/

__attribute__((weak)) void
SPI_application_event_callback(SPI_Handle_t* p_SPI_handle,
                               SPI_app_event_t app_ev)
{
	/* This is a week implementation. The application may override
	 * this function. */
}
