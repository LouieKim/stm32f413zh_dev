/*
 * evon_loop.c
 *
 *  Created on: 2020. 12. 11.
 *  Author: hyeok0724.kim@eveon.co.kr
 */

#include "main.h"
#include "evon_gpio.h"
#include "evon_slv_mbus.h"


void func_10ms(void)
{
	static uint16_t timer_10ms = 0;
	if (++timer_10ms >= 10)
	{
		timer_10ms = 0;
		located_in_timer();
	}
}

void func_100ms(void)
{
	static uint16_t timer_100ms = 0;
	if (++timer_100ms >= 100)
	{
		timer_100ms = 0;
		gpio_out();


	}
}

void func_500ms(void)
{
	static uint16_t timer_500ms = 0;
	if (++timer_500ms >= 500)
	{
		timer_500ms = 0;

	}
}

void func_1s()
{
	static uint16_t timer_1s = 0;
	if(++timer_1s >= 1000)
	{
		timer_1s = 0;
		gpio_in();
		adc_loop();

	}
}

void evon_uart1_rx(uint8_t rx_data)
{
	if(rx_data == 'a')
	{
		g_evon_gpio_out.bit.DOOR_01 = 1;
	}
	else if(rx_data == 'b')
	{
		g_evon_gpio_out.bit.DOOR_01 = 0;
	}
}



