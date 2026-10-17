# USART Driver

USART driver for the STM32F401xx series of microcontrollers, tested on the
STM32F401RE. This driver acts as a simple HAL-style API over the USART
peripheral, allowing easier configuration and control.

TX / RX operations are implemented with multiple options:
polling-based, interrupt-based, and a ring-buffer-based implementation.

### Ring buffer based TX / RX
Provides a convenient way to asynchronously interface with the USART.
The application enqueues and dequeues data in software, while the ISR
shifts data between the peripheral and a fixed-size ring buffer at its
own pace, reducing the risk of data loss during bursts and improving
reliability for logging, CLI shells, or higher-throughput protocols.

### Driver APIs
- USART peripheral initialize / de-initialize functions
- Data TX / RX logic with support for
  - Polling-based TX / RX
  - Interrupt-based TX / RX
  - Ring-buffer-based TX / RX (using interrupts at the low level)
- IRQ configuration / ISR handling
- Application event callback

### Also included:
- Configuration structures
- Handle structures
- Application states / events
- Return status codes
- Validation macros
- Driver map ( for easier navigation of the driver )

## Usage example

```c
// GPIO initialization...

// USART handle definition
USART_Handle_t usart2;

// Initializing USART handle
usart2.p_USARTx = USART2;
usart2.USART_Pin_Config.USART_baud = USART_STD_BAUD_115200;
usart2.USART_Pin_Config.USART_hw_flow_control = USART_HW_FLOW_CTRL_NONE;
usart2.USART_Pin_Config.USART_mode = USART_MODE_TXRX;
usart2.USART_Pin_Config.USART_n_stop_bits = USART_STOPBITS_1;
usart2.USART_Pin_Config.USART_word_len = USART_WORDLEN_8BITS;
usart2.USART_Pin_Config.USART_parity_control = USART_PARITY_DISABLE;

USART_init(&usart2);
USART_irq_interrupt_config(IRQ_NO_USART2, ENABLE);
USART_peri_control(USART2, ENABLE);

...

// ISR
void USART2_IRQHandler(void)
{
	USART_irq_handling(&usart2);
}

// Very simple printf clone for USART using ring buffer TX / RX
static void USART_write(char str[])
{
	uint32_t len = str_len((uint8_t*)str);
	uint32_t sent = 0;

	while (sent < len) {
		sent += USART_write_byte(&usart2, (const uint8_t*)&str[sent],
		                         len - sent);
	}
}

```

## TODOs:
- Add helper functions to longer functions to make the code more readable
- Implement synchronous communication
- Implement printf clone for USART
- Implement DMA USART in future

