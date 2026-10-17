#ifndef STM32F401RE_I2C_DRIVER_H
#define STM32F401RE_I2C_DRIVER_H

#include "stm32f401xx.h"

// NOTE: --- Structures for I2C ---

// Configuration structure for I2C 

typedef struct {
        uint32_t I2C_SCL_Speed;                                         // Possible values from @I2C_SCL_SPEED
        uint8_t I2C_Device_Address;                                     // Possible values provided by the programmer
        uint8_t I2C_ACK_Control;                                        // Possible values from @I2C_ACK_CONTROL
        uint8_t I2C_FM_Duty_Cycle;                                      // Possible values from @I2C_FM_DUTY_CYCLE
} I2C_Config_t;

// Handle structure for I2C

typedef struct {
        I2C_TypeDef* p_I2Cx;                                            // Holds the base address of the I2C peripheral 
        I2C_Config_t I2C_Config;                                        // Holds I2C peripheral configuration settings
} I2C_Handle_t;

typedef enum {
	I2C_APP_EVENT = 0                                                  // NOTE: TEMPORARY
} I2C_AppEvent_t;

// Function return status

typedef enum {
	I2C_OK = 0,							// Success
	I2C_ERROR_INVALID_STATE,					// Invalid state of a argument
	I2C_ERROR_NULL_PTR,						// NULL pointer passed
	I2C_ERROR_INVALID_PORT,						// Invalid I2C port address
	I2C_ERROR_INVALID_IRQ,						// Invalid IRQ number
	I2C_ERROR_INVALID_SCL_SPEED,					// SCL speed value out of range 
	I2C_ERROR_INVALID_ACK_CONTROL,					// ACK control value out of range 
	I2C_ERROR_INVALID_FM_DUTY_CYCLE,				// FM duty cycle value out of range 
	I2C_BUSY							// I2C Tx / Rx busy
} I2C_status_t;

// USAGE: --- @I2C_SCL_SPEED ---

#define I2C_SCL_SPEED_SM                        100000
#define I2C_SCL_SPEED_FM4K                      400000
#define I2C_SCL_SPEED_FM2K                      200000


// USAGE: --- @I2C_ACK_CONTROL ---

#define I2C_ACK_CONTROL_ENABLE                  1
#define I2C_ACK_CONTROL_DISABLE                 0

// USAGE: --- @I2C_FM_DUTY_CYCLE ---

#define I2C_FM_DUTY_CYCLE_16_9                  1
#define I2C_FM_DUTY_CYCLE_2                     0

// NOTE: --- I2C Flags ( CR1 ) ---

#define I2C_CR1_FLAG_SMB_ALERT			15
#define I2C_CR1_FLAG_TIMEOUT			14
#define I2C_CR1_FLAG_PEC_ERR			12
#define I2C_CR1_FLAG_OVR			11
#define I2C_CR1_FLAG_AF				10
#define I2C_CR1_FLAG_ARLO			9
#define I2C_CR1_FLAG_BERR			8
#define I2C_CR1_FLAG_TXE			7
#define I2C_CR1_FLAG_RXNE			6
#define I2C_CR1_FLAG_STOPF			4
#define I2C_CR1_FLAG_ADD10			3
#define I2C_CR1_FLAG_BTF			2
#define I2C_CR1_FLAG_ADDR			1
#define I2C_CR1_FLAG_SB				0

// NOTE: --- I2C Flags ( CR2 ) ---

#define I2C_CR2_FLAG_PEC			8
#define I2C_CR2_FLAG_DUALF			7
#define I2C_CR2_FLAG_SMB_HOST			6
#define I2C_CR2_FLAG_SMBDE_FAULT		5
#define I2C_CR2_FLAG_GENCALL			4
#define I2C_CR2_FLAG_TRA			2
#define I2C_CR2_FLAG_BUSY			1
#define I2C_CR2_FLAG_MSL			0

// NOTE: --- I2C Validation macros ---

#define VALIDATE_I2C_PORT(port)		do { \
							if ((port) == NULL || \
							!((port) == I2C1 || (port) == I2C2 || (port) == I2C3)) { \
								return I2C_ERROR_INVALID_PORT; \
							} \
						} while(0)

#define VALIDATE_I2C_SCL_SPEED(data)		VALIDATE_ENUM((data), I2C_SCL_SPEED_FM4K, I2C_ERROR_INVALID_SCL_SPEED)
#define VALIDATE_I2C_ACK_CONTROL(data)		VALIDATE_ENUM((data), I2C_ACK_CONTROL_ENABLE, I2C_ERROR_INVALID_ACK_CONTROL)
#define VALIDATE_I2C_FM_DUTY_CYCLE(data)	VALIDATE_ENUM((data), I2C_FM_DUTY_CYCLE_16_9, I2C_ERROR_INVALID_FM_DUTY_CYCLE)

// NOTE: --- Bit position definitions I2C_SR1 ---

#define I2C_SR1_SMB_ALERT			15
#define I2C_SR1_TIMEOUT				14
#define I2C_SR1_PEC_ERR				12
#define I2C_SR1_OVR				11
#define I2C_SR1_AF				10
#define I2C_SR1_ARLO				9
#define I2C_SR1_BERR				8
#define I2C_SR1_TxE				7
#define I2C_SR1_RxNE				6
#define I2C_SR1_STOPF				4
#define I2C_SR1_ADD10				3
#define I2C_SR1_BTF				2
#define I2C_SR1_ADDR				1
#define I2C_SR1_SB				0

// NOTE: --- Bit position definitions I2C_SR2 ---

#define I2C_SR2_PEC				8
#define I2C_SR2_DUALF				7
#define I2C_SR2_SMB_HOST			6
#define I2C_SR2_SMBDE_FAULT			5
#define I2C_SR2_GENCALL				4
#define I2C_SR2_TRA				2
#define I2C_SR2_BUSY				1
#define I2C_SR2_MSL				0

// NOTE: --- Bit position definitions I2C_CR1 ---

#define I2C_CR1_SWRST				15
#define I2C_CR1_ALERT				13
#define I2C_CR1_PEC				12
#define I2C_CR1_POS				11
#define I2C_CR1_ACK				10
#define I2C_CR1_STOP				9
#define I2C_CR1_START				8
#define I2C_CR1_NO_STRETCH			7
#define I2C_CR1_ENGC				6
#define I2C_CR1_ENPEC				5
#define I2C_CR1_ENARP				4
#define I2C_CR1_SMB_TYPE			3
#define I2C_CR1_SMBUS				1
#define I2C_CR1_PE				0

// NOTE: --- Bit position definitions I2C_CR2 ---

#define I2C_CR2_LAST				12
#define I2C_CR2_DMAEN				11
#define I2C_CR2_ITBUFEN				10
#define I2C_CR2_ITEVTEN				9
#define I2C_CR2_ITERREN				8
#define I2C_CR2_FREQ				0

// NOTE: --- Bit position definitions I2C_OAR1 ---

#define I2C_OAR1_ADD_MODE                       15
#define I2C_OAR1_ADD8                           8
#define I2C_OAR1_ADD1                           1
#define I2C_OAR1_ADD0                           0

// NOTE: --- Bit position definitions I2C_CCR ---

#define I2C_CCR_FS                              15
#define I2C_CCR_DUTY                            14
#define I2C_CCR_CCR                             0

// Peripheral clock setup
I2C_status_t I2C_peri_clk_control(I2C_TypeDef* p_I2C_x, uint8_t EN_or_DI);

// Init / de-init
I2C_status_t I2C_init(I2C_Handle_t *p_I2C_handle);
I2C_status_t I2C_de_init(I2C_TypeDef* p_I2C_x);

// Data send / receive ( polling / interrupt ) 

// Peripheral control API
I2C_status_t I2C_peri_control(I2C_TypeDef* p_I2C_x, uint8_t EN_or_DI);
uint8_t I2C_get_flag_status(I2C_TypeDef* p_I2C_x, uint8_t status_flag_name);

// IRQ configuration and ISR handling
I2C_status_t I2C_irq_interrupt_config(uint8_t irq_n, uint8_t EN_or_DI);
I2C_status_t I2C_irq_priority_config(uint8_t irq_n, uint32_t irq_prio);

// Application callback
void I2C_application_event_callback(I2C_Handle_t *p_I2C_handle,I2C_AppEvent_t app_ev);

#endif
