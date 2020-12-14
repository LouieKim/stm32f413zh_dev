/*
 * modbus.h
 *
 *  Created on: Dec 10, 2020
 *      Author: hyeok
 */

#ifndef INC_MODBUS_H_
#define INC_MODBUS_H_

#include "main.h"

uint16_t CRC16 (const uint8_t *nData, uint8_t wLength);
uint8_t hex_to_num(uint8_t x);
uint8_t make8(uint16_t var, uint8_t offset);


#endif /* INC_MODBUS_H_ */
