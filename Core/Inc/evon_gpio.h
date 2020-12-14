/*
 * evon_gpio.h
 *
 *  Created on: 2020. 12. 11.
 *      Author: hyeok
 */

#ifndef INC_EVON_GPIO_H_
#define INC_EVON_GPIO_H_

#include "main.h"
#include "usart.h"

struct _EVON_GPIO_OUT
{
	uint32_t DOOR_01:1;
	uint32_t DOOR_02:1;
	uint32_t FAN_01:1;
	uint32_t FAN_02:1;
	uint32_t MC_01:1;
	uint32_t MC_02:1;
	uint32_t RELAY_01_02:1;
	uint32_t RELAY_03_04:1;
	uint32_t RELAY_05_06:1;
	uint32_t R_LED_01:1;
	uint32_t G_LED_01:1;
	uint32_t B_LED_01:1;
	uint32_t R_LED_02:1;
	uint32_t G_LED_02:1;
	uint32_t B_LED_02:1;
	uint32_t STATUS_LED_01:1;
	uint32_t STATUS_LED_02:1;
	uint32_t STATUS_LED_03:1;
	uint32_t STATUS_LED_04:1;
	uint32_t FAULT_LED:1;
};

typedef union UGPO
{
	uint32_t          	 	 all;
	struct _EVON_GPIO_OUT    bit;
}uGPO;
extern uGPO g_evon_gpio_out;


struct _EVON_GPIO_IN
{
	uint32_t DOOR_01:1;
	uint32_t DOOR_02:1;
	uint32_t DSEN_01:1;
	uint32_t DSEN_02:1;
	uint32_t MC_01:1;
	uint32_t MC_02:1;
	uint32_t RELAY_01:1;
	uint32_t RELAY_02:1;
	uint32_t RELAY_03:1;
	uint32_t RELAY_04:1;
	uint32_t RELAY_05:1;
	uint32_t RELAY_06:1;
	uint32_t EMERGENCY:1;
};

typedef union UGPI
{
	uint32_t          	 	 all;
	struct _EVON_GPIO_IN    bit;
}uGPI;
extern uGPI g_evon_gpio_in;


uint8_t gpio_out(void);
uint8_t gpio_in(void);

#endif /* INC_EVON_GPIO_H_ */
