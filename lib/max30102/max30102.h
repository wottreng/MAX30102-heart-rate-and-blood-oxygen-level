/** \file max30102.h ******************************************************

*
* --------------------------------------------------------------------
*
* This code follows the following naming conventions:
*
* char              ch_pmod_value
* char (array)      s_pmod_s_string[16]
* float             f_pmod_value
* int32_t           n_pmod_value
* int32_t (array)   an_pmod_value[16]
* int16_t           w_pmod_value
* int16_t (array)   aw_pmod_value[16]
* uint16_t          uw_pmod_value
* uint16_t (array)  auw_pmod_value[16]
* uint8_t           uch_pmod_value
* uint8_t (array)   auch_pmod_buffer[16]
* uint32_t          un_pmod_value
* int32_t *         pn_pmod_value
*
* ------------------------------------------------------------------------- */

#ifndef MAX30102_H_
#define MAX30102_H_

#include <Arduino.h>

//
//#define I2C_WRITE_ADDR 0xAE
//#define I2C_READ_ADDR 0xAF
#define I2C_WRITE_ADDR 0x57 // 7-bit version of the above
#define I2C_READ_ADDR 0x57 // 7-bit version of the above

//register addresses
#define REG_INTR_STATUS_1 0x00
#define REG_INTR_STATUS_2 0x01
#define REG_INTR_ENABLE_1 0x02
#define REG_INTR_ENABLE_2 0x03
#define REG_FIFO_WRITE_POINTER 0x04
#define REG_OVERFLOW_COUNTER 0x05
#define REG_FIFO_READ_POINTER 0x06
#define REG_FIFO_DATA 0x07
#define REG_FIFO_CONFIG 0x08
#define REG_MODE_CONFIG 0x09
#define REG_SPO2_CONFIG 0x0A
#define REG_LED1_PULSE_AMPLITUDE 0x0C
#define REG_LED2_PULSE_AMPLITUDE 0x0D
//#define REG_PILOT_PA 0x10
#define REG_MULTI_LED_CONTROL1 0x11
#define REG_MULTI_LED_CONTROL2 0x12
#define REG_TEMP_INTEGER 0x1F
#define REG_TEMP_FRACTION 0x20
#define REG_TEMP_CONFIG 0x21
//#define REG_PROX_INT_THRESH 0x30
#define REG_REV_ID 0xFE
#define REG_PART_ID 0xFF
//

bool maxim_max30102_init();

bool maxim_max30102_read_fifo(uint32_t *pun_red_led, uint32_t *pun_ir_led); 

bool maxim_max30102_write_reg(uint8_t uch_addr, uint8_t uch_data);
bool maxim_max30102_read_reg(uint8_t uch_addr, uint8_t *puch_data);
bool maxim_max30102_reset(void);
bool maxim_max30102_read_temperature(int8_t *integer_part, uint8_t *fractional_part);
#endif /*  MAX30102_H_ */
