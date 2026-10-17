#ifndef STM32F401RE_USART_DRIVER_H
#define STM32F401RE_USART_DRIVER_H

#include "stm32f401xx.h"

// NOTE: --- Structures for USART ---

// Ring buffer logic

#define USART_TX_BUFFER_SIZE			128
#define USART_RX_BUFFER_SIZE			128

// Configuration structure for a USART

typedef struct {
	uint16_t buffer[USART_TX_BUFFER_SIZE];				// Transmit ring buffer
	volatile uint16_t head;						// Tx ring buffer head
	volatile uint16_t tail;						// Tx ring buffer tail
} USART_tx_ring_t;

typedef struct {
	uint16_t buffer[USART_RX_BUFFER_SIZE];				// Receive ring buffer
	volatile uint16_t head;						// Rx ring buffer head
	volatile uint16_t tail;						// Rx ring buffer tail
} USART_rx_ring_t;

typedef struct {
	uint8_t USART_mode;						// Possible values from @USART_MODE
	uint32_t USART_baud;						// Possible values from @USART_BAUD
	uint8_t USART_n_stop_bits;					// Possible values from @USART_NO_STOP_BITS
	uint8_t USART_word_len;						// Possible values from @USART_WORD_LENGTH
	uint8_t USART_parity_control;					// Possible values from @USART_PARITY_CONTROL
	uint8_t USART_hw_flow_control;					// Possible values from @USART_HW_FLOW_CONTROL
} USART_Pin_Config_t;

// Handle structure for USART
// NOTE: Does NOT support DMA
typedef struct {
	USART_TypeDef *p_USARTx;					// Holds the base address of the USART peripheral 
	USART_Pin_Config_t USART_Pin_Config;				// Holds USART peripheral configuration settings
	uint8_t *p_tx_buffer;						// Stores application Tx buffer address
	uint8_t *p_rx_buffer;						// Stores application Rx buffer address
	uint32_t tx_len;						// Tx length
	uint32_t rx_len;						// Rx length
	uint8_t tx_busy_state;						// Is transmission in  busy state
	uint8_t rx_busy_state;						// Is receiving in  busy state
	USART_tx_ring_t tx_buffer;					// TX buffer for USART
	USART_rx_ring_t rx_buffer;					// RX buffer for USART
} USART_Handle_t;

// Application events

typedef enum {
	USART_APP_EVENT_TX_CMPLT = 0,					// All bytes in the TX buffer have been sent
	USART_APP_EVENT_RX_CMPLT,					// The expected number of bytes has been received
	USART_APP_EVENT_IDLE,						// Idle line detected (no data for at least 1 frame)
	USART_APP_EVENT_CTS,						// CTS line changed (HW flow control event)
	USART_APP_EVENT_PE,						// Parity error detected on received data
	USART_APP_ERR_FE,						// Framing error (invalid/missing stop bit)
	USART_APP_ERR_NE,						// Noise error detected during reception
	USART_APP_ERR_ORE,						// Overrun error (new data overwrote unread data)
} USART_AppEvent_t;

// Function return status

typedef enum {
	USART_OK = 0,							// Success
	USART_ERROR_INVALID_STATE,					// Invalid state of a argument
	USART_ERROR_NULL_PTR,						// NULL pointer passed
	USART_ERROR_INVALID_PORT,					// Invalid USART port address
	USART_ERROR_INVALID_MODE,					// Mode value out of range
	USART_ERROR_INVALID_IRQ,					// Invalid IRQ number
	USART_ERROR_INVALID_STOP_BITS,					// Stop bits out of range
	USART_ERROR_INVALID_WORD_LENGTH,				// Word length out of range
	USART_ERROR_INVALID_PARITY_CONTROL,				// Parity control out of range
	USART_ERROR_INVALID_HARDWARE_FLOW,				// Hardware flow control out of range
	USART_ERROR_INVALID_BAUD_RATE,					// Baud rate out of range
	USART_ERROR_NOT_ENABLED,					// USART not enabled, but has to be
	USART_ERROR_TX_NOT_ENABLED,					// Tx not enabled, but has to be
	USART_ERROR_RX_NOT_ENABLED,					// Rx not enabled, but has to be
	USART_ERROR_TIMEOUT,						// USART polling timeout
	USART_BUSY							// USART Tx / Rx busy
} USART_status_t;

// USAGE: --- @USART_MODE ---

#define USART_MODE_ONLY_TX			0
#define USART_MODE_ONLY_RX			1
#define USART_MODE_TXRX				2

// USAGE: --- @USART_BAUD ---

#define USART_STD_BAUD_1200			1200
#define USART_STD_BAUD_2400			2400
#define USART_STD_BAUD_9600			9600
#define USART_STD_BAUD_19200			19200
#define USART_STD_BAUD_38400 			38400
#define USART_STD_BAUD_57600 			57600
#define USART_STD_BAUD_115200 			115200
#define USART_STD_BAUD_230400 			230400
#define USART_STD_BAUD_460800 			460800
#define USART_STD_BAUD_921600 			921600
#define USART_STD_BAUD_2M 			2000000
#define USART_STD_BAUD_3M 			3000000

// USAGE: --- @USART_PARITY_CONTROL ---

#define USART_PARITY_EN_ODD			2
#define USART_PARITY_EN_EVEN			1
#define USART_PARITY_DISABLE			0

// USAGE: --- @USART_WORD_LENGTH ---

#define USART_WORDLEN_8BITS			0
#define USART_WORDLEN_9BITS			1

// USAGE: --- @USART_NO_STOP_BITS ---

#define USART_STOPBITS_1			0
#define USART_STOPBITS_0_5			1
#define USART_STOPBITS_1_5			2
#define USART_STOPBITS_2			3

// USAGE: --- @USART_HW_FLOW_CONTROL ---

#define USART_HW_FLOW_CTRL_NONE			0
#define USART_HW_FLOW_CTRL_CTS			1
#define USART_HW_FLOW_CTRL_RTS			2
#define USART_HW_FLOW_CTRL_CTS_RTS		3

// NOTE: --- USART Flags ---

#define USART_FLAG_PE				0
#define USART_FLAG_FE				1
#define USART_FLAG_NF				2
#define USART_FLAG_ORE				3
#define USART_FLAG_IDLE				4
#define USART_FLAG_RXNE				5
#define USART_FLAG_TC				6
#define USART_FLAG_TXE				7
#define USART_FLAG_LBD				8
#define USART_FLAG_CTS				9

// NOTE: --- Application states ---

#define USART_READY				0
#define USART_BUSY_IN_RX			1
#define USART_BUSY_IN_TX			2

// NOTE: --- Application events ---

#define USART_EVENT_TX_CMPLT			0
#define	USART_EVENT_RX_CMPLT			1
#define	USART_EVENT_IDLE			2
#define	USART_EVENT_CTS				3
#define	USART_EVENT_PE				4
#define	USART_ERR_FE				5
#define	USART_ERR_NE				6
#define	USART_ERR_ORE				7

// NOTE: --- Bit position definitions USART_SR ---

#define USART_SR_PE				0
#define USART_SR_FE				1
#define USART_SR_NF				2
#define USART_SR_ORE				3
#define USART_SR_IDLE				4
#define USART_SR_RXNE				5
#define USART_SR_TC				6
#define USART_SR_TXE				7
#define USART_SR_LBD				8
#define USART_SR_CTS				9

// NOTE: --- USART CR1 Interrupt macros ---

#define USART_CR1_RXNEIE			5
#define USART_CR1_TCIE				6
#define USART_CR1_TXEIE				7
#define USART_CR1_UE				13

// NOTE:  --- Bit position definitions USART_CR1 ---

#define USART_CR1_SBK				0
#define USART_CR1_RWU				1
#define USART_CR1_RE				2
#define USART_CR1_TE				3
#define USART_CR1_IDLEIE			4
#define USART_CR1_RXNEIE			5
#define USART_CR1_TCIE				6
#define USART_CR1_TXEIE				7
#define USART_CR1_PEIE				8
#define USART_CR1_PS				9
#define USART_CR1_PCE				10
#define USART_CR1_WAKE				11
#define USART_CR1_M				12
#define USART_CR1_UE				13
#define USART_CR1_OVER8				15


// NOTE: --- Bit position definitions USART_CR2 ---

#define USART_CR2_ADD				0
#define USART_CR2_LBDL				5
#define USART_CR2_LBDIE				6
#define USART_CR2_LBCL				8
#define USART_CR2_CPHA				9
#define USART_CR2_CPOL				10
#define USART_CR2_CLKEN				11
#define USART_CR2_STOP_1			12
#define USART_CR2_STOP_2			13
#define USART_CR2_LINEN				14


// NOTE: --- Bit position definitions USART_CR3 ---

#define USART_CR3_EIE				0
#define USART_CR3_IREN				1
#define USART_CR3_IRLP				2
#define USART_CR3_HDSEL				3
#define USART_CR3_NACK				4
#define USART_CR3_SCEN				5
#define USART_CR3_DMAR				6
#define USART_CR3_DMAT				7
#define USART_CR3_RTSE				8
#define USART_CR3_CTSE				9
#define USART_CR3_CTSIE				10
#define USART_CR3_ONEBIT			11

// NOTE: --- USART Validation macros ---

#define VALIDATE_USART_PORT(port)		do { \
							if ((port) == NULL || \
							!((port) == USART1 || (port) == USART2 || (port) == USART6)) { \
								return USART_ERROR_INVALID_PORT; \
							} \
						} while(0)

#define VALIDATE_USART_MODE(mode)		VALIDATE_ENUM((mode), USART_MODE_TXRX, USART_ERROR_INVALID_MODE)
#define VALIDATE_USART_STOP_BITS(stop)		VALIDATE_ENUM((stop), USART_STOPBITS_2, USART_ERROR_INVALID_STOP_BITS)
#define VALIDATE_USART_WORD_LEN(wlen)		VALIDATE_ENUM((wlen), USART_WORDLEN_9BITS, USART_ERROR_INVALID_WORD_LENGTH)
#define VALIDATE_USART_PARITY(parity)		VALIDATE_ENUM((parity), USART_PARITY_EN_ODD, USART_ERROR_INVALID_PARITY_CONTROL)
#define VALIDATE_USART_HW_FLOW(flow)		VALIDATE_ENUM((flow), USART_HW_FLOW_CTRL_CTS_RTS, USART_ERROR_INVALID_HARDWARE_FLOW)
#define VALIDATE_USART_BAUD_RATE(baud)		\
						VALIDATE_IN_RANGE((baud), \
						((const uint32_t[]){1200U, 2400U, 9600U, 19200U, 38400U, 57600U, \
						 115200U, 230400U, 460800U, 921600U, 2000000U, 3000000U}) \
						 , 12, USART_ERROR_INVALID_BAUD_RATE \
						) \

#define VALIDATE_USART_ENABLED(port)		VALIDATE_BIT_SET((port)->CR1, USART_CR1_UE, USART_ERROR_NOT_ENABLED)
#define VALIDATE_USART_TX_ENABLED(port)		VALIDATE_BIT_SET((port)->CR1, USART_CR1_TE, USART_ERROR_TX_NOT_ENABLED)
#define VALIDATE_USART_RX_ENABLED(port)		VALIDATE_BIT_SET((port)->CR1, USART_CR1_RE, USART_ERROR_RX_NOT_ENABLED)

// NOTE: --- APIs supported by this driver ---

// WARNING: For 8-bit word length ( USART_WORDLEN_8BITS ), p_tx_buffer / p_rx_buffer may be byte aligned
// For 9-bit word length ( USART_WORDLEN_9BITS ), this driver treats the buffer as a array of uint16_t when parity
// is disabled. The application MUST ensure that p_tx_buffer / p_rx_buffer are 16-bit aligned and that the length
// passed corresponds to the number of BYTES, not words.

// WARNING: Prefer to use USART_read_byte / USART_write_byte ( ring buffer based, easier and more efficent )
// then other implementations

// Peripheral clock setup
USART_status_t USART_peri_clk_control(USART_TypeDef* p_USART_x, uint8_t EN_or_DI);

// Init / de-init
USART_status_t USART_init(USART_Handle_t *p_USART_handle);
USART_status_t USART_de_init(USART_TypeDef* p_USART_x);

// Data send / receive ( Ring buffer implementation )
uint32_t USART_read_byte(USART_Handle_t* p_USART_handle, uint8_t* out);
uint32_t USART_write_byte(USART_Handle_t* p_USART_handle, const uint8_t* data, uint32_t len);

// Data send / receive ( polling / interrupt ) 
USART_status_t USART_write_data_pl(USART_Handle_t* p_USART_handle, const uint8_t* p_tx_buffer, uint32_t len);
USART_status_t USART_read_data_pl(USART_Handle_t* p_USART_handle, uint8_t* p_rx_buffer, uint32_t len);
USART_status_t USART_write_data_it(USART_Handle_t *p_USART_handle,uint8_t *p_tx_buffer, uint32_t len);
USART_status_t USART_read_data_it(USART_Handle_t *p_USART_handle, uint8_t *p_rx_buffer, uint32_t len);

// Peripheral control API
USART_status_t USART_peri_control(USART_TypeDef* p_USART_x, uint8_t EN_or_DI);
uint8_t USART_get_flag_status(USART_TypeDef* p_USART_x, uint8_t status_flag_name);
void USART_clear_flag(USART_TypeDef* p_USART_x, uint16_t status_flag_name);

// IRQ configuration and ISR handling
USART_status_t USART_irq_interrupt_config(uint8_t irq_n, uint8_t EN_or_DI);
USART_status_t USART_irq_priority_config(uint8_t irq_n, uint32_t irq_prio);
void USART_irq_handling(USART_Handle_t *p_USART_handle);

// Utils
// USART_status_t USART_set_baud_rate(USART_TypeDef* p_USART_x, uint32_t baud_rate);

// Application callback
void USART_application_event_callback(USART_Handle_t *p_USART_handle,USART_AppEvent_t app_ev);

#endif
