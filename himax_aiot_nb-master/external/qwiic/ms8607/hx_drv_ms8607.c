/*
 * hx_drv_ms8607.c
 *
 *  Created on: 2021¦~1¤ë11¤é
 *      Author: 902449
 */

#include <stdio.h>
#include <string.h>

#include "hx_drv_ms8607.h"

#define PSENSOR_I2C_ADDR	0x76
#define HSENSOR_I2C_ADDR	0x40

// Coefficients indexes for temperature and pressure computation
#define CRC_INDEX 0
#define PRESSURE_SENSITIVITY_INDEX 1
#define PRESSURE_OFFSET_INDEX 2
#define TEMP_COEFF_OF_PRESSURE_SENSITIVITY_INDEX 3
#define TEMP_COEFF_OF_PRESSURE_OFFSET_INDEX 4
#define REFERENCE_TEMPERATURE_INDEX 5
#define TEMP_COEFF_OF_TEMPERATURE_INDEX 6
#define COEFFICIENT_NUMBERS 7

// Conversion timings
#define HSENSOR_CONVERSION_TIME_12b 16
#define HSENSOR_CONVERSION_TIME_10b 5
#define HSENSOR_CONVERSION_TIME_8b 3
#define HSENSOR_CONVERSION_TIME_11b 9

// PSENSOR device commands
#define PSENSOR_RESET_COMMAND 0x1E
#define PSENSOR_START_PRESSURE_ADC_CONVERSION 0x40
#define PSENSOR_START_TEMPERATURE_ADC_CONVERSION 0x50
#define PSENSOR_READ_ADC 0x00

#define PSENSOR_CONVERSION_OSR_MASK 0x0F

#define PSENSOR_CONVERSION_TIME_OSR_256 1
#define PSENSOR_CONVERSION_TIME_OSR_512 2
#define PSENSOR_CONVERSION_TIME_OSR_1024 3
#define PSENSOR_CONVERSION_TIME_OSR_2048 5
#define PSENSOR_CONVERSION_TIME_OSR_4096 9
#define PSENSOR_CONVERSION_TIME_OSR_8192 18

// PSENSOR commands
#define PROM_ADDRESS_READ_ADDRESS_0 0xA0
#define PROM_ADDRESS_READ_ADDRESS_1 0xA2
#define PROM_ADDRESS_READ_ADDRESS_2 0xA4
#define PROM_ADDRESS_READ_ADDRESS_3 0xA6
#define PROM_ADDRESS_READ_ADDRESS_4 0xA8
#define PROM_ADDRESS_READ_ADDRESS_5 0xAA
#define PROM_ADDRESS_READ_ADDRESS_6 0xAC
#define PROM_ADDRESS_READ_ADDRESS_7 0xAE

// HSENSOR device commands
#define HSENSOR_RESET_COMMAND 0xFE
#define HSENSOR_READ_HUMIDITY_W_HOLD_COMMAND 0xE5
#define HSENSOR_READ_HUMIDITY_WO_HOLD_COMMAND 0xF5
#define HSENSOR_READ_SERIAL_FIRST_8BYTES_COMMAND 0xFA0F
#define HSENSOR_READ_SERIAL_LAST_6BYTES_COMMAND 0xFCC9
#define HSENSOR_WRITE_USER_REG_COMMAND 0xE6
#define HSENSOR_READ_USER_REG_COMMAND 0xE7

// Processing constants
#define HSENSOR_TEMPERATURE_COEFFICIENT (float)(-0.15)
#define HSENSOR_CONSTANT_A (float)(8.1332)
#define HSENSOR_CONSTANT_B (float)(1762.39)
#define HSENSOR_CONSTANT_C (float)(235.66)

// Coefficients for temperature computation
#define TEMPERATURE_COEFF_MUL (175.72)
#define TEMPERATURE_COEFF_ADD (-46.85)

// Coefficients for relative humidity computation
#define HUMIDITY_COEFF_MUL (125)
#define HUMIDITY_COEFF_ADD (-6)

// Conversion timings
#define HSENSOR_CONVERSION_TIME_12b 16
#define HSENSOR_CONVERSION_TIME_10b 5
#define HSENSOR_CONVERSION_TIME_8b 3
#define HSENSOR_CONVERSION_TIME_11b 9

#define HSENSOR_RESET_TIME 15 // ms value

// HSENSOR User Register masks and bit position
#define HSENSOR_USER_REG_RESOLUTION_MASK 0x81
#define HSENSOR_USER_REG_END_OF_BATTERY_MASK 0x40
#define HSENSOR_USER_REG_ENABLE_ONCHIP_HEATER_MASK 0x4
#define HSENSOR_USER_REG_DISABLE_OTP_RELOAD_MASK 0x2
#define HSENSOR_USER_REG_RESERVED_MASK                                         \
  (~(HSENSOR_USER_REG_RESOLUTION_MASK | HSENSOR_USER_REG_END_OF_BATTERY_MASK | \
     HSENSOR_USER_REG_ENABLE_ONCHIP_HEATER_MASK |                              \
     HSENSOR_USER_REG_DISABLE_OTP_RELOAD_MASK))

// HTU User Register values
// Resolution
#define HSENSOR_USER_REG_RESOLUTION_12b 0x00
#define HSENSOR_USER_REG_RESOLUTION_11b 0x81
#define HSENSOR_USER_REG_RESOLUTION_10b 0x80
#define HSENSOR_USER_REG_RESOLUTION_8b 0x01

// End of battery status
#define HSENSOR_USER_REG_END_OF_BATTERY_VDD_ABOVE_2_25V 0x00
#define HSENSOR_USER_REG_END_OF_BATTERY_VDD_BELOW_2_25V 0x40
// Enable on chip heater
#define HSENSOR_USER_REG_ONCHIP_HEATER_ENABLE 0x04
#define HSENSOR_USER_REG_OTP_RELOAD_DISABLE 0x02

enum MS8607_pressure_resolution
{
       MS8607_pressure_resolution_osr_256 = 0,
       MS8607_pressure_resolution_osr_512,
       MS8607_pressure_resolution_osr_1024,
       MS8607_pressure_resolution_osr_2048,
       MS8607_pressure_resolution_osr_4096,
       MS8607_pressure_resolution_osr_8192
};

enum MS8607_humidity_resolution
{
       MS8607_humidity_resolution_12b = 0,
       MS8607_humidity_resolution_8b,
       MS8607_humidity_resolution_10b,
       MS8607_humidity_resolution_11b
};

enum MS8607_humidity_i2c_master_mode
{
       MS8607_i2c_hold,
       MS8607_i2c_no_hold
};

uint32_t hsensor_conversion_time = HSENSOR_CONVERSION_TIME_12b;
uint16_t eeprom_coeff[COEFFICIENT_NUMBERS + 1];
enum MS8607_pressure_resolution psensor_resolution_osr;
enum MS8607_humidity_i2c_master_mode hsensor_i2c_master_mode;

bool psensor_crc_check(uint16_t *n_prom, uint8_t crc)
{
  uint8_t cnt, n_bit;
  uint16_t n_rem, crc_read;

  n_rem = 0x00;
  crc_read = n_prom[0];
  n_prom[COEFFICIENT_NUMBERS] = 0;
  n_prom[0] = (0x0FFF & (n_prom[0])); // Clear the CRC byte

  for (cnt = 0; cnt < (COEFFICIENT_NUMBERS + 1) * 2; cnt++)
  {

    // Get next byte
    if (cnt % 2 == 1)
      n_rem ^= n_prom[cnt >> 1] & 0x00FF;
    else
      n_rem ^= n_prom[cnt >> 1] >> 8;

    for (n_bit = 8; n_bit > 0; n_bit--)
    {

      if (n_rem & 0x8000)
        n_rem = (n_rem << 1) ^ 0x3000;
      else
        n_rem <<= 1;
    }
  }
  n_rem >>= 12;
  n_prom[0] = crc_read;

  return (n_rem == crc);
}

HX_DRV_QWIIC_ERROR_E psensor_read_eeprom(void)
{
	uint8_t i;
	uint8_t read_Data[2];

	for (i = 0; i < COEFFICIENT_NUMBERS; i++)
	{
		if(hx_drv_qwiic_set_cmd(PSENSOR_I2C_ADDR,PROM_ADDRESS_READ_ADDRESS_0 + i * 2)!= HX_DRV_QWIIC_PASS)
		  return HX_DRV_QWIIC_ERROR;

		if(hx_drv_qwiic_get_data(PSENSOR_I2C_ADDR,read_Data,2)!= HX_DRV_QWIIC_PASS)
		  return HX_DRV_QWIIC_ERROR;

		eeprom_coeff[i] = read_Data[0]<<8 | read_Data[1];
	}

	if (!psensor_crc_check(eeprom_coeff, (eeprom_coeff[CRC_INDEX] & 0xF000) >> 12))
		return HX_DRV_QWIIC_ERROR;

  return HX_DRV_QWIIC_PASS;
}

HX_DRV_QWIIC_ERROR_E psensor_conversion_and_read_adc(uint8_t cmd,
                                                           uint32_t *adc)
{
	uint8_t buffer[3];

	// Send the read command
	if(hx_drv_qwiic_set_cmd(PSENSOR_I2C_ADDR,cmd)!= HX_DRV_QWIIC_PASS)
		  return HX_DRV_QWIIC_ERROR;

	// 20ms wait for conversion
	board_delay_cycle(20* BOARD_SYS_TIMER_MS_CONV);

	if(hx_drv_qwiic_set_cmd(PSENSOR_I2C_ADDR,PSENSOR_READ_ADC)!= HX_DRV_QWIIC_PASS)
		  return HX_DRV_QWIIC_ERROR;

	/* Read data */
	if(hx_drv_qwiic_get_data(PSENSOR_I2C_ADDR,buffer,3)!= HX_DRV_QWIIC_PASS)
		  return HX_DRV_QWIIC_ERROR;

	*adc = ((uint32_t)buffer[0] << 16) | ((uint32_t)buffer[1] << 8) | buffer[2];

	return HX_DRV_QWIIC_PASS;
}

//temperature and pressure is 100 times of data
HX_DRV_QWIIC_ERROR_E psensor_read_pressure_and_temperature(int32_t *temperature,
		int32_t *pressure)
{
	HX_DRV_QWIIC_ERROR_E status = HX_DRV_QWIIC_PASS;
	uint32_t adc_temperature, adc_pressure;
	int32_t dT, TEMP;
	int64_t OFF, SENS, P, T2, OFF2, SENS2;
	uint8_t cmd;

	// First read temperature
	cmd = psensor_resolution_osr * 2;
	cmd |= PSENSOR_START_TEMPERATURE_ADC_CONVERSION;
	if(psensor_conversion_and_read_adc(cmd, &adc_temperature)!= HX_DRV_QWIIC_PASS)
		return HX_DRV_QWIIC_ERROR;

	// Now read pressure
	cmd = psensor_resolution_osr * 2;
	cmd |= PSENSOR_START_PRESSURE_ADC_CONVERSION;
	if(psensor_conversion_and_read_adc(cmd, &adc_pressure)!= HX_DRV_QWIIC_PASS)
		return HX_DRV_QWIIC_ERROR;

	if (adc_temperature == 0 || adc_pressure == 0)
		return HX_DRV_QWIIC_NO_DATA;

	// Difference between actual and reference temperature = D2 - Tref
	dT = (int32_t)adc_temperature -
			((int32_t)eeprom_coeff[REFERENCE_TEMPERATURE_INDEX] << 8);

	// Actual temperature = 2000 + dT * TEMPSENS
	TEMP = 2000 + ((int64_t)dT *
			(int64_t)eeprom_coeff[TEMP_COEFF_OF_TEMPERATURE_INDEX] >>23);

	// Second order temperature compensation
	if (TEMP < 2000)
	{
		T2 = (3 * ((int64_t)dT * (int64_t)dT)) >> 33;
		OFF2 = 61 * ((int64_t)TEMP - 2000) * ((int64_t)TEMP - 2000) / 16;
		SENS2 = 29 * ((int64_t)TEMP - 2000) * ((int64_t)TEMP - 2000) / 16;

		if (TEMP < -1500)
		{
			OFF2 += 17 * ((int64_t)TEMP + 1500) * ((int64_t)TEMP + 1500);
			SENS2 += 9 * ((int64_t)TEMP + 1500) * ((int64_t)TEMP + 1500);
		}
	}
	else
	{
		T2 = (5 * ((int64_t)dT * (int64_t)dT)) >> 38;
		OFF2 = 0;
		SENS2 = 0;
	}

	// OFF = OFF_T1 + TCO * dT
	OFF = ((int64_t)(eeprom_coeff[PRESSURE_OFFSET_INDEX]) << 17) +
		(((int64_t)(eeprom_coeff[TEMP_COEFF_OF_PRESSURE_OFFSET_INDEX]) * dT) >>6);
	OFF -= OFF2;

	// Sensitivity at actual temperature = SENS_T1 + TCS * dT
	SENS =
	  ((int64_t)eeprom_coeff[PRESSURE_SENSITIVITY_INDEX] << 16) +
	  (((int64_t)eeprom_coeff[TEMP_COEFF_OF_PRESSURE_SENSITIVITY_INDEX] * dT) >> 7);
	SENS -= SENS2;

	// Temperature compensated pressure = D1 * SENS - OFF
	P = (((adc_pressure * SENS) >> 21) - OFF) >> 15;

	//*temperature = ((float)TEMP - T2) / 100;
	//*pressure = (float)P / 100;
	*temperature = TEMP -(int32_t)T2;
	*pressure = (int32_t)P;

	return status;
}


HX_DRV_QWIIC_ERROR_E hsensor_read_user_register(uint8_t *value)
{
	uint8_t buffer[1] ={0};

	// Send the Read Register Command
	if(hx_drv_qwiic_set_cmd(HSENSOR_I2C_ADDR,HSENSOR_READ_USER_REG_COMMAND)!= HX_DRV_QWIIC_PASS)
		  return HX_DRV_QWIIC_ERROR;

	if(hx_drv_qwiic_get_data(HSENSOR_I2C_ADDR,buffer,1)!= HX_DRV_QWIIC_PASS)
		  return HX_DRV_QWIIC_ERROR;

  *value = buffer[0];

  return HX_DRV_QWIIC_PASS;
}

HX_DRV_QWIIC_ERROR_E hsensor_write_user_register(uint8_t value)
{
  uint8_t reg;

  if(hsensor_read_user_register(&reg)!=HX_DRV_QWIIC_PASS)
	  return HX_DRV_QWIIC_ERROR;

  // Clear bits of reg that are not reserved
  reg &= HSENSOR_USER_REG_RESERVED_MASK;
  // Set bits from value that are not reserved
  reg |= (value & ~HSENSOR_USER_REG_RESERVED_MASK);

  if(hx_drv_qwiic_set_reg(HSENSOR_I2C_ADDR,HSENSOR_WRITE_USER_REG_COMMAND,reg)!=HX_DRV_QWIIC_PASS)
	  return HX_DRV_QWIIC_ERROR;

  return HX_DRV_QWIIC_PASS;
}

HX_DRV_QWIIC_ERROR_E set_humidity_resolution(enum MS8607_humidity_resolution res)
{
	uint8_t reg_value, tmp = 0;
	uint32_t conversion_time = HSENSOR_CONVERSION_TIME_12b;

	if (res == MS8607_humidity_resolution_12b)
	{
		tmp = HSENSOR_USER_REG_RESOLUTION_12b;
		conversion_time = HSENSOR_CONVERSION_TIME_12b;
	}
	else if (res == MS8607_humidity_resolution_10b)
	{
		tmp = HSENSOR_USER_REG_RESOLUTION_10b;
		conversion_time = HSENSOR_CONVERSION_TIME_10b;
	}
	else if (res == MS8607_humidity_resolution_8b)
	{
		tmp = HSENSOR_USER_REG_RESOLUTION_8b;
		conversion_time = HSENSOR_CONVERSION_TIME_8b;
	}
	else if (res == MS8607_humidity_resolution_11b)
	{
		tmp = HSENSOR_USER_REG_RESOLUTION_11b;
		conversion_time = HSENSOR_CONVERSION_TIME_11b;
	}

	if(hsensor_read_user_register(&reg_value)!= HX_DRV_QWIIC_PASS)
	  return HX_DRV_QWIIC_ERROR;

	// Clear the resolution bits
	reg_value &= ~HSENSOR_USER_REG_RESOLUTION_MASK;
	reg_value |= tmp & HSENSOR_USER_REG_RESOLUTION_MASK;

	//disable heater
	reg_value &= ~HSENSOR_USER_REG_ONCHIP_HEATER_ENABLE;

	hsensor_conversion_time = conversion_time;

	return hsensor_write_user_register(reg_value);

}

HX_DRV_QWIIC_ERROR_E disable_humidity_heater(void)
{
	uint8_t reg_value;

	if(hsensor_read_user_register(&reg_value)!= HX_DRV_QWIIC_PASS)
		return HX_DRV_QWIIC_ERROR;

	reg_value &= ~HSENSOR_USER_REG_ONCHIP_HEATER_ENABLE;


	return hsensor_write_user_register(reg_value);
}

HX_DRV_QWIIC_ERROR_E hsensor_crc_check(uint16_t value, uint8_t crc)
{
	uint32_t polynom = 0x988000; // x^8 + x^5 + x^4 + 1
	uint32_t msb = 0x800000;
	uint32_t mask = 0xFF8000;
	uint32_t result = (uint32_t)value << 8; // Pad with zeros as specified in spec

	while (msb != 0x80)
	{
		// Check if msb of current value is 1 and apply XOR mask
		if (result & msb)
			result = ((result ^ polynom) & mask) | (result & ~mask);

		// Shift by one
		msb >>= 1;
		mask >>= 1;
		polynom >>= 1;
	}

	return (result == crc) ? HX_DRV_QWIIC_PASS : HX_DRV_QWIIC_ERROR;
}

HX_DRV_QWIIC_ERROR_E hsensor_humidity_conversion_and_read_adc(uint16_t *adc)
{
	uint16_t _adc;
	uint8_t buffer[3];
	uint8_t crc;

	if(hsensor_i2c_master_mode == MS8607_i2c_hold) {
		if(hx_drv_qwiic_set_cmd(HSENSOR_I2C_ADDR,HSENSOR_READ_HUMIDITY_W_HOLD_COMMAND)!= HX_DRV_QWIIC_PASS)
				  return HX_DRV_QWIIC_ERROR;
	}
	else {
		if(hx_drv_qwiic_set_cmd(HSENSOR_I2C_ADDR,HSENSOR_READ_HUMIDITY_WO_HOLD_COMMAND)!= HX_DRV_QWIIC_PASS)
				  return HX_DRV_QWIIC_ERROR;
		// delay depending on resolution
		board_delay_cycle(hsensor_conversion_time* BOARD_SYS_TIMER_MS_CONV);
	}

	if(hx_drv_qwiic_get_data(HSENSOR_I2C_ADDR,buffer,3)!= HX_DRV_QWIIC_PASS)
	  return HX_DRV_QWIIC_ERROR;

	_adc = (buffer[0] << 8) | buffer[1];
	crc = buffer[2];

	// compute CRC
	if(hsensor_crc_check(_adc, crc)!= HX_DRV_QWIIC_PASS)
		  return HX_DRV_QWIIC_ERROR;

	*adc = _adc;

	return HX_DRV_QWIIC_PASS;
}

HX_DRV_QWIIC_ERROR_E hx_drv_ms8607_initial() {

	// reset
	if(hx_drv_qwiic_set_cmd(PSENSOR_I2C_ADDR, PSENSOR_RESET_COMMAND)!= HX_DRV_QWIIC_PASS)
		return HX_DRV_QWIIC_ERROR;

	board_delay_cycle(30* BOARD_SYS_TIMER_MS_CONV);

	if(psensor_read_eeprom()!= HX_DRV_QWIIC_PASS)
		return HX_DRV_QWIIC_ERROR;

	if(hx_drv_qwiic_set_cmd(HSENSOR_I2C_ADDR, HSENSOR_RESET_COMMAND)!= HX_DRV_QWIIC_PASS)
		return HX_DRV_QWIIC_ERROR;

	psensor_resolution_osr = MS8607_pressure_resolution_osr_8192;
	hsensor_i2c_master_mode = MS8607_i2c_no_hold;

	//set humidity_resolution and disable heater
	set_humidity_resolution(MS8607_humidity_resolution_12b);
	//after humidity sensor reset, it takes about 15ms to be back online
	//disable_humidity_heater();

	return HX_DRV_QWIIC_PASS;
}


HX_DRV_QWIIC_ERROR_E hx_drv_ms8607_reset_psensor() {

	// reset
	if(hx_drv_qwiic_set_cmd(PSENSOR_I2C_ADDR, PSENSOR_RESET_COMMAND)!= HX_DRV_QWIIC_PASS)
		return HX_DRV_QWIIC_ERROR;

	psensor_resolution_osr = MS8607_pressure_resolution_osr_8192;


	return HX_DRV_QWIIC_PASS;
}

HX_DRV_QWIIC_ERROR_E hx_drv_ms8607_reset_hsensor() {

	// reset
	if(hx_drv_qwiic_set_cmd(HSENSOR_I2C_ADDR, HSENSOR_RESET_COMMAND)!= HX_DRV_QWIIC_PASS)
		return HX_DRV_QWIIC_ERROR;

	hsensor_conversion_time = HSENSOR_CONVERSION_TIME_12b;

	return HX_DRV_QWIIC_PASS;
}



//temperature and pressure is 100 times of data
HX_DRV_QWIIC_ERROR_E hx_drv_ms8607_receive_t_p(int32_t* t_data, int32_t* p_data) {

	int32_t p=0,t=0;
	if(psensor_read_pressure_and_temperature(&t,&p)!= HX_DRV_QWIIC_PASS)
		return HX_DRV_QWIIC_ERROR;

	*t_data = t;
	*p_data = p;

	return HX_DRV_QWIIC_PASS;
}


HX_DRV_QWIIC_ERROR_E hx_drv_ms8607_receive_h(float *humidity)
{
	uint16_t adc;

	if(hsensor_humidity_conversion_and_read_adc(&adc)!= HX_DRV_QWIIC_PASS)
		return HX_DRV_QWIIC_ERROR;

	// Perform conversion function
	*humidity =
	  (float)adc * HUMIDITY_COEFF_MUL / (1UL << 16) + HUMIDITY_COEFF_ADD;

	return HX_DRV_QWIIC_PASS;
}


#if 0


HX_DRV_QWIIC_ERROR_E hx_drv_ms8607_receive_pressure(float* data) {
//HX_DRV_QWIIC_ERROR_E hx_drv_ms8607_receive_pressure(int32_t* data) {

	float p=0,t=0;
	//int32_t p=0,t=0;

	if(psensor_read_pressure_and_temperature(&t,&p)!= HX_DRV_QWIIC_PASS)
		return HX_DRV_QWIIC_ERROR;

	*data = p;
	return HX_DRV_QWIIC_PASS;
}

HX_DRV_QWIIC_ERROR_E hx_drv_ms8607_receive_temperature(float* data) {
//HX_DRV_QWIIC_ERROR_E hx_drv_ms8607_receive_temperature(int32_t* data) {

	float p=0,t=0;
	//int32_t p=0,t=0;
	if(psensor_read_pressure_and_temperature(&t,&p)!= HX_DRV_QWIIC_PASS)
		return HX_DRV_QWIIC_ERROR;

	*data = t;

	return HX_DRV_QWIIC_PASS;
}
#endif
