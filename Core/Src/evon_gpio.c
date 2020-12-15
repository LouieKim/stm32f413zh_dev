/*
 * evon_gpio.c
 *
 *  Created on: 2020. 12. 11.
 *      Author: hyeok
 */

#include "evon_gpio.h"
#include "adc.h"

#define DOOR_01_ON	 		HAL_GPIO_WritePin(GPIOF, GPIO_PIN_13, GPIO_PIN_SET)
#define DOOR_01_OFF 		HAL_GPIO_WritePin(GPIOF, GPIO_PIN_13, GPIO_PIN_RESET)
#define DOOR_02_ON			HAL_GPIO_WritePin(GPIOF, GPIO_PIN_14, GPIO_PIN_SET)
#define DOOR_02_OFF			HAL_GPIO_WritePin(GPIOF, GPIO_PIN_14, GPIO_PIN_RESET)
#define FAN_01_ON			HAL_GPIO_WritePin(GPIOC, GPIO_PIN_11, GPIO_PIN_SET)
#define FAN_01_OFF			HAL_GPIO_WritePin(GPIOC, GPIO_PIN_11, GPIO_PIN_RESET)
#define FAN_02_ON			HAL_GPIO_WritePin(GPIOC, GPIO_PIN_10, GPIO_PIN_SET)
#define FAN_02_OFF			HAL_GPIO_WritePin(GPIOC, GPIO_PIN_10, GPIO_PIN_RESET)
#define MC_01_ON			HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET)
#define MC_01_OFF			HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET)
#define MC_02_ON			HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_SET)
#define MC_02_OFF			HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_RESET)
#define RELAY_01_02_ON		HAL_GPIO_WritePin(GPIOE, GPIO_PIN_11, GPIO_PIN_SET)
#define RELAY_01_02_OFF		HAL_GPIO_WritePin(GPIOE, GPIO_PIN_11, GPIO_PIN_RESET)
#define RELAY_03_04_ON		HAL_GPIO_WritePin(GPIOE, GPIO_PIN_12, GPIO_PIN_SET)
#define RELAY_03_04_OFF		HAL_GPIO_WritePin(GPIOE, GPIO_PIN_12, GPIO_PIN_RESET)
#define RELAY_05_06_ON		HAL_GPIO_WritePin(GPIOE, GPIO_PIN_13, GPIO_PIN_SET)
#define RELAY_05_06_OFF		HAL_GPIO_WritePin(GPIOE, GPIO_PIN_13, GPIO_PIN_RESET)

#define R_LED_01_ON			HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, GPIO_PIN_SET)
#define R_LED_01_OFF		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, GPIO_PIN_RESET)
#define G_LED_01_ON			HAL_GPIO_WritePin(GPIOF, GPIO_PIN_11, GPIO_PIN_SET)
#define G_LED_01_OFF		HAL_GPIO_WritePin(GPIOF, GPIO_PIN_11, GPIO_PIN_RESET)
#define B_LED_01_ON			HAL_GPIO_WritePin(GPIOF, GPIO_PIN_12, GPIO_PIN_SET)
#define B_LED_01_OFF		HAL_GPIO_WritePin(GPIOF, GPIO_PIN_12, GPIO_PIN_RESET)

#define R_LED_02_ON			HAL_GPIO_WritePin(GPIOE, GPIO_PIN_8, GPIO_PIN_SET)
#define R_LED_02_OFF		HAL_GPIO_WritePin(GPIOE, GPIO_PIN_8, GPIO_PIN_RESET)
#define G_LED_02_ON			HAL_GPIO_WritePin(GPIOE, GPIO_PIN_9, GPIO_PIN_SET)
#define G_LED_02_OFF		HAL_GPIO_WritePin(GPIOE, GPIO_PIN_9, GPIO_PIN_RESET)
#define B_LED_02_ON			HAL_GPIO_WritePin(GPIOE, GPIO_PIN_10, GPIO_PIN_SET)
#define B_LED_02_OFF		HAL_GPIO_WritePin(GPIOE, GPIO_PIN_10, GPIO_PIN_RESET)

#define STATUS_LED_01_ON	HAL_GPIO_WritePin(GPIOG, GPIO_PIN_8, GPIO_PIN_SET)
#define STATUS_LED_01_OFF	HAL_GPIO_WritePin(GPIOG, GPIO_PIN_8, GPIO_PIN_RESET)
#define STATUS_LED_02_ON	HAL_GPIO_WritePin(GPIOG, GPIO_PIN_7, GPIO_PIN_SET)
#define STATUS_LED_02_OFF	HAL_GPIO_WritePin(GPIOG, GPIO_PIN_7, GPIO_PIN_RESET)
#define STATUS_LED_03_ON	HAL_GPIO_WritePin(GPIOG, GPIO_PIN_6, GPIO_PIN_SET)
#define STATUS_LED_03_OFF	HAL_GPIO_WritePin(GPIOG, GPIO_PIN_6, GPIO_PIN_RESET)
#define STATUS_LED_04_ON	HAL_GPIO_WritePin(GPIOG, GPIO_PIN_5, GPIO_PIN_SET)
#define STATUS_LED_04_OFF	HAL_GPIO_WritePin(GPIOG, GPIO_PIN_5, GPIO_PIN_RESET)
#define FAULT_LED_ON		HAL_GPIO_WritePin(GPIOG, GPIO_PIN_4, GPIO_PIN_SET)
#define FAULT_LED_OFF		HAL_GPIO_WritePin(GPIOG, GPIO_PIN_4, GPIO_PIN_RESET)


#define DOOR_01_IN			HAL_GPIO_ReadPin(GPIOF, GPIO_PIN_3)
#define DOOR_02_IN			HAL_GPIO_ReadPin(GPIOF, GPIO_PIN_4)
#define DSEN_01_IN			HAL_GPIO_ReadPin(GPIOF, GPIO_PIN_1)
#define DSEN_02_IN			HAL_GPIO_ReadPin(GPIOF, GPIO_PIN_2)
#define MC_01_IN			HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_15)
#define MC_02_IN			HAL_GPIO_ReadPin(GPIOF, GPIO_PIN_0)
#define RELAY_01_IN			HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_2)
#define RELAY_02_IN			HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_3)
#define RELAY_03_IN			HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_4)
#define RELAY_04_IN			HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_5)
#define RELAY_05_IN			HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_6)
#define RELAY_06_IN			HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13)

#define EMERGENCY_IN		HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_8)



uGPO g_evon_gpio_out;
uGPI g_evon_gpio_in;


//포인터로 Parameter 받기
uint8_t gpio_out()
{
	//Config 상에서 변경만으로 알 수 있도록 해야함
	(g_evon_gpio_out.bit.DOOR_01) ? DOOR_01_ON : DOOR_01_OFF;
	(g_evon_gpio_out.bit.DOOR_02) ? DOOR_02_ON : DOOR_02_OFF;

	(g_evon_gpio_out.bit.FAN_01) ? FAN_01_ON : FAN_01_OFF;
	(g_evon_gpio_out.bit.FAN_02) ? FAN_02_ON : FAN_02_OFF;

	(g_evon_gpio_out.bit.MC_01) ? MC_01_ON : MC_01_OFF;
	(g_evon_gpio_out.bit.MC_02) ? MC_02_ON : MC_02_OFF;

	(g_evon_gpio_out.bit.RELAY_01_02) ? RELAY_01_02_ON : RELAY_01_02_OFF;
	(g_evon_gpio_out.bit.RELAY_03_04) ? RELAY_03_04_ON : RELAY_03_04_OFF;
	(g_evon_gpio_out.bit.RELAY_05_06) ? RELAY_05_06_ON : RELAY_05_06_OFF;

	(g_evon_gpio_out.bit.R_LED_01) ? R_LED_01_ON : R_LED_01_OFF;
	(g_evon_gpio_out.bit.G_LED_01) ? G_LED_01_ON : G_LED_01_OFF;
	(g_evon_gpio_out.bit.B_LED_01) ? B_LED_01_ON : B_LED_01_OFF;

	(g_evon_gpio_out.bit.R_LED_02) ? R_LED_02_ON : R_LED_02_OFF;
	(g_evon_gpio_out.bit.G_LED_02) ? G_LED_02_ON : G_LED_02_OFF;
	(g_evon_gpio_out.bit.B_LED_02) ? B_LED_02_ON : B_LED_02_OFF;

	(g_evon_gpio_out.bit.STATUS_LED_01) ? STATUS_LED_01_ON : STATUS_LED_01_OFF;
	(g_evon_gpio_out.bit.STATUS_LED_02) ? STATUS_LED_02_ON : STATUS_LED_02_OFF;
	(g_evon_gpio_out.bit.STATUS_LED_03) ? STATUS_LED_03_ON : STATUS_LED_03_OFF;
	(g_evon_gpio_out.bit.STATUS_LED_04) ? STATUS_LED_04_ON : STATUS_LED_04_OFF;
	(g_evon_gpio_out.bit.FAULT_LED) ? FAULT_LED_ON : FAULT_LED_OFF;

	return 1;
}


uint8_t gpio_in()
{
	g_evon_gpio_in.bit.DOOR_01 = DOOR_01_IN;
	g_evon_gpio_in.bit.DOOR_02 = DOOR_02_IN;
	g_evon_gpio_in.bit.DSEN_01 = DSEN_01_IN;
	g_evon_gpio_in.bit.DSEN_02 = DSEN_02_IN;
	g_evon_gpio_in.bit.MC_01 = MC_01_IN;
	g_evon_gpio_in.bit.MC_02 = MC_02_IN;
	g_evon_gpio_in.bit.RELAY_01 = RELAY_01_IN;
	g_evon_gpio_in.bit.RELAY_02 = RELAY_02_IN;
	g_evon_gpio_in.bit.RELAY_03 = RELAY_03_IN;
	g_evon_gpio_in.bit.RELAY_04 = RELAY_04_IN;
	g_evon_gpio_in.bit.RELAY_05 = RELAY_05_IN;
	g_evon_gpio_in.bit.RELAY_06 = RELAY_06_IN;
	g_evon_gpio_in.bit.EMERGENCY = EMERGENCY_IN;

	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, GPIO_PIN_SET); //UART4 En_tx_mode
	//HAL_UART_Transmit(&huart1,  &g_evon_gpio_in.all, 4, 10);

	HAL_UART_Transmit(&huart1, &adc_val[3], 2, 10);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, GPIO_PIN_RESET); //UART4 En_rx_mode*/

	return 1;
}


