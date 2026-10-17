#include "stm32f401xx_RCC_driver.h"
#include <stdint.h>

/********************************************************************************
 *
 * TODO: Future plans for this driver:
 * Nothing so far
 *
 *******************************************************************************/

uint16_t AHB_Prescaler[8] = {2, 4, 8, 16, 64, 128, 256, 512};
uint8_t APBx_Prescaler[4] = {2, 4, 8, 16};

/********************************************************************************
 * @fn			        - RCC_get_pclk_1_val
 *
 * @brief			- Returns PCLK1 value
 *
 * @return			- PCLK1 Value
 *
 * @Note			- None
 *******************************************************************************/

uint32_t RCC_get_pclk_1_val(void)
{
	uint32_t pclk_1, system_clk;
	uint8_t clk_src, temp, ahb_p, apb1p;

	clk_src = ((RCC->CFGR >> 2) & 0x3);

	if (clk_src == 0) {
		system_clk = 16000000;
	}
	else if (clk_src == 1) {
		system_clk = 8000000;
	}
	else if (clk_src == 2) {
		system_clk = RCC_get_pll_output_clk();
	}

	/* AHBP */
	temp = ((RCC->CFGR >> 4) & 0xF);

	if (temp < 8) {
		ahb_p = 1;
	}
	else {
		ahb_p = AHB_Prescaler[temp - 8];
	}

	/* APB1 */
	temp = ((RCC->CFGR >> 10) & 0x7);

	if (temp < 4) {
		apb1p = 1;
	}
	else {
		apb1p = APBx_Prescaler[temp - 4];
	}

	pclk_1 = (system_clk / ahb_p) / apb1p;

	return pclk_1;
}

/********************************************************************************
 * @fn				- RCC_get_pclk_2_val
 *
 * @brief			- Returns PCLK2 value
 *
 * @return			- PCLK2 Value
 *
 * @Note			- None
 *******************************************************************************/

uint32_t RCC_get_pclk_2_val(void)
{
	uint32_t system_clk = 0, temp, pclk_2;
	uint8_t clk_src = ((RCC->CFGR >> 2) & 0x3);
	uint8_t ahb_p, apb2_p;

	if (clk_src == 0) {
		system_clk = 16000000;
	}
	else if (clk_src == 1) {
		system_clk = 8000000;
	}
	else if (clk_src == 2) {
		system_clk = RCC_get_pll_output_clk();
	}

	/* AHBP */
	temp = ((RCC->CFGR >> 4) & 0xF);

	if (temp < 8) {
		ahb_p = 1;
	}
	else {
		ahb_p = AHB_Prescaler[temp - 8];
	}

	/* APB2 */
	temp = ((RCC->CFGR >> 13) & 0x7);

	if (temp < 4) {
		apb2_p = 1;
	}
	else {
		apb2_p = APBx_Prescaler[temp - 4];
	}

	pclk_2 = (system_clk / ahb_p) / apb2_p;

	return pclk_2;
}

/*****************************************************************
 * @fn          - RCC_get_pll_output_clk
 *
 * @brief       - Returns PLL output value
 *
 * @Note        - None
 *
 *******************************************************************************/

uint32_t RCC_get_pll_output_clk(void)
{
	// TODO:
	return 0;
}
