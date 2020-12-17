/*
 * modbus.c
 *
 *  Created on: Dec 10, 2020
 *      Author: hyeok
 */

#include "evon_slv_mbus.h"
#include "usart.h"
#include "gpio.h"

#define __ring_buffer_max 40
#define __modbusReturnInterval 1

uint8_t __ring_buffer[__ring_buffer_max + 1];
uint16_t __save_pointer = 0;
uint16_t __action_pointer = __ring_buffer_max;

uint8_t __modbusSwitch = 0;
uint16_t __modbusBlankCheck = 0;

uint8_t __modbusSlaveAdr = 0;
uint16_t *__modbusWordBuffer;
uint8_t *__modbusBitBuffer;
uint8_t __modbusBroadMode = 0;
uint8_t __modbusFoundFrame = 0;
uint16_t __modbusIntervalCounter = 0;
uint8_t uchCRCHi, uchCRCLo;
uint16_t uINDEX;

uint8_t MDcoil[20] = {'\0', };
uint16_t MDregister[50] = {'\0', };

uint8_t debug8 = 0;
uint16_t debug16 = 0;

void comFlush(void)
{
	__action_pointer = __ring_buffer_max;
	__save_pointer = 0;
}

uint16_t comLen(void)
{
	uint16_t i;
	if(__save_pointer > (__action_pointer + 1))
	{
		i = __save_pointer - __action_pointer - 1;
	}
	else if(__save_pointer < (__action_pointer+1))
	{
		i = __ring_buffer_max - __action_pointer + __save_pointer;
	}
	else
	{
		i = 0;
	}
	return i;
}

void modbusFrameSearch(void);

void push_ringbuffer(uint8_t comdt)
{
	if(__modbusSwitch)
	{
		if(__modbusBlankCheck > 50)
		{
			comFlush();
		}
		__modbusBlankCheck = 0;
	}
	if(__save_pointer != __action_pointer)
	{
		__ring_buffer[__save_pointer] = comdt;
		__save_pointer++;
		if(__save_pointer > __ring_buffer_max)
		{
			__save_pointer = 0;
		}
	}
	if(__modbusSwitch) modbusFrameSearch();
}

uint8_t read_ringbuffer(void)
{
	__action_pointer++;
	if(__action_pointer > __ring_buffer_max)
	{
		__action_pointer = 0;
	}
	return (__ring_buffer[__action_pointer]);
}

void modbusMainProcessing(void);

////////====== func_10ms 안에 넣을것 ===============================
void located_in_timer()
{
	if(__modbusFoundFrame)
	{
		__modbusIntervalCounter++;
		if(__modbusIntervalCounter > __modbusReturnInterval)
		{
			__modbusIntervalCounter = 0;
			__modbusFoundFrame = 0;
			modbusMainProcessing();
		}
	}
	if(__modbusBlankCheck < 0xffff) __modbusBlankCheck++;
}
//================================================================

void comPut(uint8_t d)
{
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, GPIO_PIN_SET); //UART4 En_tx_mode
	HAL_UART_Transmit(&huart1, &d, 1, 10);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, GPIO_PIN_RESET); //UART4 En_rx_mode
}


void mb_crc_compute(uint8_t rch);

uint8_t mb_get_byte_rtu(void)
{
	return read_ringbuffer();
}

uint16_t mb_get_word_rtu(void)
{
	uint8_t wh, wl;
	wh = read_ringbuffer();
	wl = read_ringbuffer();
	return (wh<<8)+wl;
}

void mb_inc_pointer(void)
{
	(void)read_ringbuffer();
}

void mb_put_byte_rtu(uint8_t dt)
{
	comPut(dt);
	mb_crc_compute(dt);
}

uint8_t mb_ncdtb[] = {1, 2, 4, 8, 16, 32, 64, 128};

uint8_t mb_collectbits(uint16_t adr)
{
	uint8_t i, j, m = 0;
	for(i=0; i<8; i++)
	{
		j = __modbusBitBuffer[adr>>3];
		if(j & mb_ncdtb[adr & 7]) m |= mb_ncdtb[i];
		adr++;
	}
	return m;
}

void mbcmd_read_bit_rtu(uint8_t cmd)
{
	uint8_t i,j,bln;
	uint16_t stadr, ln;
	stadr = mb_get_word_rtu();
	ln = mb_get_word_rtu();
	mb_inc_pointer();
	mb_inc_pointer();

	if(__modbusBroadMode) return ;
	uchCRCHi = uchCRCLo = 0xff;
	mb_put_byte_rtu(__modbusSlaveAdr);
	mb_put_byte_rtu(cmd);
	bln = ln >> 3;
	if(ln & 7) bln++;
	mb_put_byte_rtu(bln);
	for(i=0; i<bln; i++)
	{
		j = mb_collectbits(stadr);
		mb_put_byte_rtu(j);
		stadr += 8;
	}
	i = uchCRCLo;
	mb_put_byte_rtu(uchCRCHi);
	mb_put_byte_rtu(i);
}

void mbcmd_write_bit_single_rtu()
{
	uint8_t i;
	uint16_t stadr, realadr, val;
	stadr = mb_get_word_rtu();
	val = mb_get_word_rtu();
	mb_inc_pointer();
	mb_inc_pointer();

	debug16 = stadr;
	debug8 = val;

	realadr = stadr >> 3;
	if(val) __modbusBitBuffer[realadr] |= mb_ncdtb[stadr & 7];
	else __modbusBitBuffer[realadr] &= ~mb_ncdtb[stadr & 7];

	if(__modbusBroadMode) return;

	uchCRCHi = uchCRCLo = 0xff;
	mb_put_byte_rtu(__modbusSlaveAdr);
	mb_put_byte_rtu(5);
	mb_put_byte_rtu(stadr >> 8);
	mb_put_byte_rtu(stadr);
	mb_put_byte_rtu(val >> 8);
	mb_put_byte_rtu(val);

	i = uchCRCLo;
	mb_put_byte_rtu(uchCRCHi);
	mb_put_byte_rtu(i);
}

void mbcmd_read_word_rtu(uint8_t cmd)
{
	uint8_t i;
	uint16_t stadr,ln,li,dt;
	stadr = mb_get_word_rtu();
	ln = mb_get_word_rtu();
	mb_inc_pointer();
	mb_inc_pointer();

	if(__modbusBroadMode) return;

	uchCRCHi = uchCRCLo = 0xff;
	mb_put_byte_rtu(__modbusSlaveAdr);
	mb_put_byte_rtu(cmd);
	i = ln << 1;
	mb_put_byte_rtu(i);

	for(li=0; li<ln; li++)
	{
		dt = __modbusWordBuffer[stadr++];
		mb_put_byte_rtu(dt >> 8);
		mb_put_byte_rtu(dt);
	}

	i = uchCRCLo;
	mb_put_byte_rtu(uchCRCHi);
	mb_put_byte_rtu(i);
}

void mbcmd_write_word_single_rtu()
{
	uint8_t i;
	uint16_t stadr, val;
	stadr = mb_get_word_rtu();
	val = mb_get_word_rtu();
	mb_inc_pointer();
	mb_inc_pointer();
	__modbusWordBuffer[stadr] = val;

	if(__modbusBroadMode) return;

	uchCRCHi = uchCRCLo = 0xff;
	mb_put_byte_rtu(__modbusSlaveAdr);
	mb_put_byte_rtu(6);
	mb_put_byte_rtu(stadr >> 8);
	mb_put_byte_rtu(stadr);
	mb_put_byte_rtu(val >> 8);
	mb_put_byte_rtu(val);
	i = uchCRCLo;
	mb_put_byte_rtu(uchCRCHi);
	mb_put_byte_rtu(i);
}

void mb_writebits(uint16_t adr, uint8_t val, uint8_t ln)
{
	uint8_t i, m;
	if(ln > 7) m = 8;
	else m = ln;

	for(i=0; i<m; i++)
	{
		if(val & mb_ncdtb[i])
		{
			__modbusBitBuffer[adr >> 3] |= mb_ncdtb[adr&7];
		}
		else
		{
			__modbusBitBuffer[adr >> 3] &= ~mb_ncdtb[adr&7];
		}
		adr++;
	}
}

void mbcmd_write_bit_multiple_rtu()
{
	uint8_t i,j,cnt;
	uint16_t stadr,ln,lnsave,stadr_save;
	stadr_save = stadr = mb_get_word_rtu();
	lnsave = ln = mb_get_word_rtu();
	cnt = mb_get_byte_rtu();
	for(i=0; i<cnt; i++)
	{
		j = mb_get_byte_rtu();
		mb_writebits(stadr, j, ln);
		stadr += 8;
		ln -= 8;
	}
	mb_inc_pointer();
	mb_inc_pointer();

	if(__modbusBroadMode) return;

	uchCRCHi = uchCRCLo = 0xff;
	mb_put_byte_rtu(__modbusSlaveAdr);
	mb_put_byte_rtu(15);
	mb_put_byte_rtu(stadr_save >> 8);
	mb_put_byte_rtu(stadr_save);
	mb_put_byte_rtu(lnsave >> 8);
	mb_put_byte_rtu(lnsave);
	i = uchCRCLo;
	mb_put_byte_rtu(uchCRCHi);
	mb_put_byte_rtu(i);
}

void mbcmd_write_word_multiple_rtu()
{
	uint8_t j;
	uint16_t stadr, realadr, val, ln, i;
	realadr = stadr = mb_get_word_rtu();
	ln = mb_get_word_rtu();
	mb_inc_pointer();
	for(i=0; i<ln; i++)
	{
		val = mb_get_word_rtu();
		__modbusWordBuffer[realadr++] = val;
	}
	mb_inc_pointer();
	mb_inc_pointer();

	if(__modbusBroadMode) return;

	uchCRCHi = uchCRCLo = 0xff;
	mb_put_byte_rtu(__modbusSlaveAdr);
	mb_put_byte_rtu(stadr >> 8);
	mb_put_byte_rtu(stadr);
	mb_put_byte_rtu(ln >> 8);
	mb_put_byte_rtu(ln);
	j = uchCRCLo;
	mb_put_byte_rtu(uchCRCHi);
	mb_put_byte_rtu(j);

}

const uint8_t auchCRCH[] = {
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01,
0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81,
0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01,
0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01,
0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01,
0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,0x40};

const uint8_t auchCRCL[] = {
0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 0xC6, 0x06, 0x07, 0xC7, 0x05, 0xC5, 0xC4,
0x04, 0xCC, 0x0C, 0x0D, 0xCD, 0x0F, 0xCF, 0xCE, 0x0E, 0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09,
0x08, 0xC8, 0xD8, 0x18, 0x19, 0xD9, 0x1B, 0xDB, 0xDA, 0x1A, 0x1E, 0xDE, 0xDF, 0x1F, 0xDD,
0x1D, 0x1C, 0xDC, 0x14, 0xD4, 0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6, 0xD2, 0x12, 0x13, 0xD3,
0x11, 0xD1, 0xD0, 0x10, 0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3, 0xF2, 0x32, 0x36, 0xF6, 0xF7,
0x37, 0xF5, 0x35, 0x34, 0xF4, 0x3C, 0xFC, 0xFD, 0x3D, 0xFF, 0x3F, 0x3E, 0xFE, 0xFA, 0x3A,
0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38, 0x28, 0xE8, 0xE9, 0x29, 0xEB, 0x2B, 0x2A, 0xEA, 0xEE,
0x2E, 0x2F, 0xEF, 0x2D, 0xED, 0xEC, 0x2C, 0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26,
0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0, 0xA0, 0x60, 0x61, 0xA1, 0x63, 0xA3, 0xA2,
0x62, 0x66, 0xA6, 0xA7, 0x67, 0xA5, 0x65, 0x64, 0xA4, 0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F,
0x6E, 0xAE, 0xAA, 0x6A, 0x6B, 0xAB, 0x69, 0xA9, 0xA8, 0x68, 0x78, 0xB8, 0xB9, 0x79, 0xBB,
0x7B, 0x7A, 0xBA, 0xBE, 0x7E, 0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C, 0xB4, 0x74, 0x75, 0xB5,
0x77, 0xB7, 0xB6, 0x76, 0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71, 0x70, 0xB0, 0x50, 0x90, 0x91,
0x51, 0x93, 0x53, 0x52, 0x92, 0x96, 0x56, 0x57, 0x97, 0x55, 0x95, 0x94, 0x54, 0x9C, 0x5C,
0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E, 0x5A, 0x9A, 0x9B, 0x5B, 0x99, 0x59, 0x58, 0x98, 0x88,
0x48, 0x49, 0x89, 0x4B, 0x8B, 0x8A, 0x4A, 0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C,
0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86, 0x82, 0x42, 0x43, 0x83, 0x41, 0x81, 0x80,0x40};

void mb_crc_compute(uint8_t rch)
{
	uINDEX = uchCRCHi ^ rch;
	uchCRCHi = uchCRCLo ^ auchCRCH[uINDEX];
	uchCRCLo = auchCRCL[uINDEX];
}

uint16_t getCrc(uint8_t *targetArray, uint16_t datalength)
{
	uint16_t d;
	uchCRCHi = uchCRCLo = 0xff;
	for(d=0; d<datalength; d++)
	{
		mb_crc_compute(targetArray[d]);
	}
	d = (uchCRCHi << 8) + uchCRCLo;
	return d;
}

void startSimpleModbus(uint8_t modslaveadr, uint16_t *modbusBufferRegister, uint8_t *modbusBufferCoil)
{
	__modbusSlaveAdr = modslaveadr;
	__modbusSwitch = 1;
	__modbusWordBuffer = modbusBufferRegister;
	__modbusBitBuffer = modbusBufferCoil;
}

void modbusFrameSearch(void)
{
	uint8_t uch, ucl, bh, bl;
	uint16_t mclen, i;
	uint16_t uix;
	uint16_t save_pointer;
	mclen = comLen();

	if(mclen < 8) return;

	uch = ucl = 0xff;

	save_pointer = __action_pointer;
	for(i=0; i<mclen-2; i++)
	{
		uix = uch^mb_get_byte_rtu();
		uch = ucl^auchCRCH[uix];
		ucl = auchCRCL[uix];
	}
	bh = mb_get_byte_rtu();
	bl = mb_get_byte_rtu();
	__action_pointer = save_pointer;

	if((bh == uch) && (bl == ucl)) __modbusFoundFrame = 1;
}

void modbusMainProcessing(void)
{
	uint8_t fc,sladr;
	sladr = mb_get_byte_rtu();
	if(sladr == 0)
	{
		__modbusBroadMode = 1;
	}
	else
	{
		if(sladr != __modbusSlaveAdr)
		{
			comFlush();
			return;
		}
	}

	fc = mb_get_byte_rtu();

	switch(fc)
	{
		case 1:
		case 2:
			mbcmd_read_bit_rtu(fc);
			break;
		case 3:
		case 4:
			mbcmd_read_word_rtu(fc);
			break;
		case 5:
			mbcmd_write_bit_single_rtu();
			break;
		case 6:
			mbcmd_write_word_single_rtu();
			break;
		case 15:
			mbcmd_write_bit_multiple_rtu();
			break;
		case 16:
			mbcmd_write_word_multiple_rtu();
			break;
		default:
			comFlush();
			return;
	}
}

uint8_t coil(uint16_t MDBSadr)
{
	MDBSadr--;
	if(MDcoil[MDBSadr>>3] & mb_ncdtb[MDBSadr & 7]) return 1;
	return 0;
}

void coilSet(uint16_t MDBSadr, uint8_t MDBSdata)
{
	uint16_t arrayindex;
	MDBSadr--;
	arrayindex = MDBSadr>>3;
	if(MDBSdata == 0)
	{
		MDcoil[arrayindex] &= ~mb_ncdtb[MDBSadr & 7];
	}
	else
	{
		MDcoil[arrayindex] |= mb_ncdtb[MDBSadr & 7];
	}
}

uint16_t getReg(uint16_t MDBSadr)
{
	return MDregister[MDBSadr - 40001];
}

void setReg(uint16_t MDBSadr, uint16_t MDBSdata)
{
	MDregister[MDBSadr - 40001] = MDBSdata;
}

void delay(uint16_t i)
{
	uint16_t j, k;
	for(j=0; j<i; j++)
	{
		for(k=0; k<300; k++)
		{
			//wdt_reset();
		}
	}
}
