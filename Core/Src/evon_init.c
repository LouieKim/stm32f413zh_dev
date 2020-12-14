/*
 * evon_init.c
 *
 *  Created on: Dec 14, 2020
 *      Author: hyeok
 */

#include "main.h"
#include "evon_gpio.h"

/* USER CODE BEGIN 4 */
void function_config(void)
{
	// TIM1 1ms
	LL_TIM_EnableIT_UPDATE(TIM1);
	LL_TIM_EnableCounter(TIM1);
}

void gpio_init(void)
{

	g_evon_gpio_out.bit.DOOR_01 = 0;
	g_evon_gpio_out.bit.DOOR_02 = 0;

	g_evon_gpio_out.bit.FAN_01 = 0;
	g_evon_gpio_out.bit.FAN_02 = 0;

	g_evon_gpio_out.bit.MC_01 = 0;
	g_evon_gpio_out.bit.MC_02 = 0;

	g_evon_gpio_out.bit.RELAY_01_02 = 0;
	g_evon_gpio_out.bit.RELAY_03_04 = 0;
	g_evon_gpio_out.bit.RELAY_05_06 = 0;

	g_evon_gpio_out.bit.R_LED_01 = 0;
	g_evon_gpio_out.bit.G_LED_01 = 0;
	g_evon_gpio_out.bit.B_LED_01 = 0;

	g_evon_gpio_out.bit.R_LED_02 = 0;
	g_evon_gpio_out.bit.G_LED_02 = 0;
	g_evon_gpio_out.bit.B_LED_02 = 0;

}
