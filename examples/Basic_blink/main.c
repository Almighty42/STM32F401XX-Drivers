#include "stm32f401xx.h"
#include "stm32f401xx_GPIO_driver.h"
#include <stdint.h>

#define DELAY_MS 500

volatile uint32_t time_delay = 0;

void SysTick_Init(uint32_t ticks);
void Delay(uint32_t time);

int main(void)
{
	GPIO_Handle_t gpio_led = {
	    .p_GPIOx = GPIOA,
	    .GPIO_Pin_Config.GPIO_Pin_Number = GPIO_PIN_NO_5,
	    .GPIO_Pin_Config.GPIO_Pin_Mode = GPIO_MODE_OUT,
	    .GPIO_Pin_Config.GPIO_Pin_Speed = GPIO_SPEED_FAST,
	    .GPIO_Pin_Config.GPIO_Pin_OTP_Type = GPIO_OP_TYPE_PP,
	    .GPIO_Pin_Config.GPIO_Pin_PuPd_Control = GPIO_NO_PUPD};

	// Enables RCC clock for GPIO_A
	GPIO_peri_clk_control(GPIOA, ENABLE);

	// Prepares the GPIO_A for digital output and sets other settings
	GPIO_init(&gpio_led);

	// Initialize SysTick for 16MHz
	// SysTick_Init(16000000U);

	while (1) {
		// LED TOGGLE
		GPIO_toggle_output_pin(GPIOA, GPIO_PIN_NO_5);
		Delay(DELAY_MS);
	}
}

// void SysTick_Init(uint32_t ticks)
// {
// 	// Disable SysTick IRQ and SysTick counter
// 	SysTick_Type->CTRL = 0;
// 	// Set reload register
// 	uint32_t reload = ticks / 1000U - 1U;
// 	SysTick->LOAD = reload;
// 	// Set interrupt priority of SysTick
// 	NVIC_Set_IRQ_Priority(SysTick_IRQn, (1 << __NVIC_PRIO_BITS) - 1);
// 	// Reset the SysTick counter value
// 	SysTick->VAL = 0;
// 	// Processor clock
// 	SET_BIT(SysTick->CTRL, 2);
// 	// Enables SysTick exception request
// 	SET_BIT(SysTick->CTRL, 1);
// 	// Enables SysTick timer
// 	SET_BIT(SysTick->CTRL, 0);
// }

void Delay(uint32_t time)
{
	time_delay = time;
	while (time_delay != 0)
		;
}

void SysTick_Handler(void)
{
	if (time_delay > 0)
		time_delay--;
}
