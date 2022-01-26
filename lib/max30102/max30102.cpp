/** \file max30102.cpp ******************************************************
*
* Project: MAXREFDES117#
* Filename: max30102.cpp
* Description: This module is an embedded controller driver for the MAX30102
*
* Revision History:
*\n 1-18-2016 Rev 01.00 GL Initial release.
*\n 12-22-2017 Rev 02.00 Significantlly modified by Robert Fraczkiewicz
*\n to use Wire library instead of MAXIM's SoftI2C
*\n 01-24-2022 Rev +, modified by Mark Wottreng, mostly for clarity
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
/*******************************************************************************
* Copyright (C) 2016 Maxim Integrated Products, Inc., All Rights Reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a
* copy of this software and associated documentation files (the "Software"),
* to deal in the Software without restriction, including without limitation
* the rights to use, copy, modify, merge, publish, distribute, sublicense,
* and/or sell copies of the Software, and to permit persons to whom the
* Software is furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included
* in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
* OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL MAXIM INTEGRATED BE LIABLE FOR ANY CLAIM, DAMAGES
* OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
* ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
* OTHER DEALINGS IN THE SOFTWARE.
*
* Except as contained in this notice, the name of Maxim Integrated
* Products, Inc. shall not be used except as stated in the Maxim Integrated
* Products, Inc. Branding Policy.
*
* The mere transfer of this software does not imply any licenses
* of trade secrets, proprietary technology, copyrights, patents,
* trademarks, maskwork rights, or any other form of intellectual
* property whatsoever. Maxim Integrated Products, Inc. retains all
* ownership rights.
*******************************************************************************
*/
#include "max30102.h"
#include <Wire.h>
#include <algorithm.h>



bool maxim_max30102_write_reg(uint8_t uch_addr, uint8_t uch_data)
/**
* \brief        Write a value to a MAX30102 register
* \par          Details
*               This function writes a value to a MAX30102 register
*
* \param[in]    uch_addr    - register address
* \param[in]    uch_data    - register data
*
* \retval       true on success
*/
{
  Wire.beginTransmission(I2C_WRITE_ADDR);
  Wire.write(uch_addr);
  Wire.write(uch_data);
  Wire.endTransmission();
  return true;
}

bool maxim_max30102_read_reg(uint8_t uch_addr, uint8_t *puch_data)
/**
* \brief        Read a MAX30102 register
* \par          Details
*               This function reads a MAX30102 register
*
* \param[in]    uch_addr    - register address
* \param[out]   puch_data    - pointer that stores the register data
*
* \retval       true on success
*/
{
  Wire.beginTransmission(I2C_WRITE_ADDR);
  Wire.write(uch_addr);
  Wire.endTransmission();
  Wire.beginTransmission(I2C_READ_ADDR);
  Wire.requestFrom(I2C_READ_ADDR,1);
  *puch_data=Wire.read();
  Wire.endTransmission();
  return true;
}

bool maxim_max30102_init() // ------------------------- INIT --------------------------
/**
* \brief        Initialize the MAX30102
* \par          Details
*               This function initializes the MAX30102
*
* \param        None
*
* \retval       true on success
*/
{

  // ------------------------
#define sda_pin 5 // D1 -> pin 5
#define scl_pin 4 // D2 -> pin 4
 // ----------------------------
    Wire.begin(sda_pin, scl_pin);
    Wire.setClock(400000L);

    maxim_max30102_reset(); // resets the MAX30102
    delay(1000);

    uint8_t uch_dummy;
    maxim_max30102_read_reg(REG_INTR_STATUS_1, &uch_dummy); // Reads/clears the interrupt status register
    /*
    for register values and meaning: https://datasheets.maximintegrated.com/en/ds/MAX30102.pdf
    */

    if (!maxim_max30102_write_reg(REG_INTR_ENABLE_1, 0b1'1'0'00000)) // fifo almost full int on, new sample int, ambient light cancellation int
        return false;
    if (!maxim_max30102_write_reg(REG_INTR_ENABLE_2, 0x000000'0'0)) // 0
        return false;
    if (!maxim_max30102_write_reg(REG_FIFO_WRITE_POINTER, 0x0)) // 0, FIFO_WR_PTR[4:0]
        return false;
    if (!maxim_max30102_write_reg(REG_OVERFLOW_COUNTER, 0x0)) // 0, OVF_COUNTER[4:0]
        return false;
    if (!maxim_max30102_write_reg(REG_FIFO_READ_POINTER, 0x0)) // 0, FIFO_RD_PTR[4:0]
        return false;
    if (!maxim_max30102_write_reg(REG_FIFO_CONFIG, 0b0100'0'010)) // fifo almost full = 0100 => 28 unread data samples, fifo rollover=false, sample avg = 4
        return false;
    if (!maxim_max30102_write_reg(REG_MODE_CONFIG, 0b00000'011)) // 010 for Red only(heart rate), 011 for SpO2 mode, 111 multimode LED
        return false;
    if (!maxim_max30102_write_reg(REG_SPO2_CONFIG, 0b0'01'001'11)) // SPO2_ADC range = 4096, SPO2 sample rate (100 Hz), LED pulseWidth (411uS)
        return false;
    if (!maxim_max30102_write_reg(REG_LED1_PULSE_AMPLITUDE, 60)) // led pulse amplitude 36 => 7mA
        return false;
    if (!maxim_max30102_write_reg(REG_LED2_PULSE_AMPLITUDE, 60)) // led2 amplitude
        return false;
    /*
    if (!maxim_max30102_write_reg(0x11, 0b0'010'0'001)) // multimode led control, red then ir
        return false;
    if (!maxim_max30102_write_reg(0x12, 0b0'010'0'001)) // multimode led control, red then ir
        return false;
    */

    return true;  
}

bool maxim_max30102_read_fifo(uint32_t* pointer_red_led_data, uint32_t* pointer_ir_led_data)
/**
 * \brief        Read a set of samples from the MAX30102 FIFO register
 * \par          Details
 *               This function reads a set of samples from the MAX30102 FIFO register
 *
 * \param[out]   *pointer_red_led_data   - pointer that stores the red LED reading data
 * \param[out]   *pointer_ir_led_data    - pointer that stores the IR LED reading data
 *
 * \retval       true on success
 */
{
    uint32_t un_temp;
    uint8_t uch_temp;
    *pointer_ir_led_data = 0;
    *pointer_red_led_data = 0;
    maxim_max30102_read_reg(REG_INTR_STATUS_1, &uch_temp);
    maxim_max30102_read_reg(REG_INTR_STATUS_2, &uch_temp);
    Wire.beginTransmission(I2C_WRITE_ADDR); // 57
    Wire.write(REG_FIFO_DATA); // 7
    Wire.endTransmission();
    Wire.beginTransmission(I2C_READ_ADDR);
    Wire.requestFrom(I2C_READ_ADDR, 0b0110); // 6
    // red
    // data is read 1 Byte (8 bits) at a time from sensor
    un_temp = Wire.read(); // read 8 bits
    un_temp <<= 16; // shift bits left 16
    *pointer_red_led_data += un_temp; // data = data + un_temp
    un_temp = Wire.read(); // read 8 bits
    un_temp <<= 8; // shift left 8
    *pointer_red_led_data += un_temp;
    un_temp = Wire.read(); // read last 8 bits
    *pointer_red_led_data += un_temp;
    // ir
    un_temp = Wire.read(); // read 8 bits
    un_temp <<= 16; // shift 16 bits
    *pointer_ir_led_data += un_temp; // add them
    un_temp = Wire.read(); // read next 8 bits
    un_temp <<= 8; // shift data left by 8 bits
    *pointer_ir_led_data += un_temp;
    un_temp = Wire.read();
    *pointer_ir_led_data += un_temp;
    //
    Wire.endTransmission();
    *pointer_red_led_data &= 0b000000111111111111111111; // Mask MSB [23:18], zero out [23:18]
    *pointer_ir_led_data &= 0b000000111111111111111111; // Mask MSB [23:18], zero out bits 23 -> 18
    return true;
}

bool maxim_max30102_reset()
/**
* \brief        Reset the MAX30102
* \par          Details
*               This function resets the MAX30102
*
* \param        None
*
* \retval       true on success
*/
{
    if(!maxim_max30102_write_reg(REG_MODE_CONFIG,0x40))
        return false;
    else
        return true;    
}

bool maxim_max30102_read_temperature(int8_t *integer_part, uint8_t *fractional_part)
{
  maxim_max30102_write_reg(REG_TEMP_CONFIG,0b0000000'1); // Enabling TEMP_EN
  delay(1); // Let the processor do its work
  // For proper conversion, read the integer part as uint8_t
  uint8_t temp;
  maxim_max30102_read_reg(REG_TEMP_INTEGER, &temp); // 2's complement integer part of the temperature in degrees Celsius
  *integer_part = temp;
  maxim_max30102_read_reg(REG_TEMP_FRACTION, fractional_part); // Fractional part of the temperature in 1/16-th degree Celsius
  return true;
}
