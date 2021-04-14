/*
 * hx_drv_bme280.c
 *
 *  Created on: 2021/1/12
 *      Author: 904212
 */

#include "hx_drv_bme280.h"

struct BME280_SensorCalibration hx_drv_bme280_calibration;
int32_t t_fine = 0;
uint8_t BME280_I2C_ADDRESS = BME280_I2C_ADDRESS_0X77;

uint8_t hx_drv_bme280_readRegister(uint8_t offset)
{
	uint8_t readCheck;
	hx_drv_qwiic_set_cmd(BME280_I2C_ADDRESS, offset);
	hx_drv_qwiic_get_data(BME280_I2C_ADDRESS, &readCheck, 1);
	return readCheck;
}

HX_DRV_QWIIC_ERROR_E hx_drv_bme280_initial(uint8_t i2caddress)
{
	BME280_I2C_ADDRESS = i2caddress == 0 ? BME280_I2C_ADDRESS_0X76 : BME280_I2C_ADDRESS_0X77;

	uint8_t readCheck;

	uint8_t setStandbyTime = 0;
	uint8_t setFilter = 4;
	uint8_t setPressureOverSample = 5;
	uint8_t setHumidityOverSample = 5;
	uint8_t setTempOverSample = 5;

	// check HW id
	hx_drv_qwiic_set_cmd(BME280_I2C_ADDRESS, BME280_CHIP_ID_REG);
	hx_drv_qwiic_get_data(BME280_I2C_ADDRESS, &readCheck, 1);
	if (readCheck != 0x58 && readCheck != 0x60)
		return HX_DRV_QWIIC_ERROR;

	//Reading all compensation data, range 0x88:A1, 0xE1:E7
	hx_drv_bme280_calibration.dig_T1 = ((uint16_t)((hx_drv_bme280_readRegister(BME280_DIG_T1_MSB_REG) << 8) + hx_drv_bme280_readRegister(BME280_DIG_T1_LSB_REG)));
	hx_drv_bme280_calibration.dig_T2 = ((int16_t)((hx_drv_bme280_readRegister(BME280_DIG_T2_MSB_REG) << 8) + hx_drv_bme280_readRegister(BME280_DIG_T2_LSB_REG)));
	hx_drv_bme280_calibration.dig_T3 = ((int16_t)((hx_drv_bme280_readRegister(BME280_DIG_T3_MSB_REG) << 8) + hx_drv_bme280_readRegister(BME280_DIG_T3_LSB_REG)));

	hx_drv_bme280_calibration.dig_P1 = ((uint16_t)((hx_drv_bme280_readRegister(BME280_DIG_P1_MSB_REG) << 8) + hx_drv_bme280_readRegister(BME280_DIG_P1_LSB_REG)));
	hx_drv_bme280_calibration.dig_P2 = ((int16_t)((hx_drv_bme280_readRegister(BME280_DIG_P2_MSB_REG) << 8) + hx_drv_bme280_readRegister(BME280_DIG_P2_LSB_REG)));
	hx_drv_bme280_calibration.dig_P3 = ((int16_t)((hx_drv_bme280_readRegister(BME280_DIG_P3_MSB_REG) << 8) + hx_drv_bme280_readRegister(BME280_DIG_P3_LSB_REG)));
	hx_drv_bme280_calibration.dig_P4 = ((int16_t)((hx_drv_bme280_readRegister(BME280_DIG_P4_MSB_REG) << 8) + hx_drv_bme280_readRegister(BME280_DIG_P4_LSB_REG)));
	hx_drv_bme280_calibration.dig_P5 = ((int16_t)((hx_drv_bme280_readRegister(BME280_DIG_P5_MSB_REG) << 8) + hx_drv_bme280_readRegister(BME280_DIG_P5_LSB_REG)));
	hx_drv_bme280_calibration.dig_P6 = ((int16_t)((hx_drv_bme280_readRegister(BME280_DIG_P6_MSB_REG) << 8) + hx_drv_bme280_readRegister(BME280_DIG_P6_LSB_REG)));
	hx_drv_bme280_calibration.dig_P7 = ((int16_t)((hx_drv_bme280_readRegister(BME280_DIG_P7_MSB_REG) << 8) + hx_drv_bme280_readRegister(BME280_DIG_P7_LSB_REG)));
	hx_drv_bme280_calibration.dig_P8 = ((int16_t)((hx_drv_bme280_readRegister(BME280_DIG_P8_MSB_REG) << 8) + hx_drv_bme280_readRegister(BME280_DIG_P8_LSB_REG)));
	hx_drv_bme280_calibration.dig_P9 = ((int16_t)((hx_drv_bme280_readRegister(BME280_DIG_P9_MSB_REG) << 8) + hx_drv_bme280_readRegister(BME280_DIG_P9_LSB_REG)));

	hx_drv_bme280_calibration.dig_H1 = ((uint8_t)(hx_drv_bme280_readRegister(BME280_DIG_H1_REG)));
	hx_drv_bme280_calibration.dig_H2 = ((int16_t)((hx_drv_bme280_readRegister(BME280_DIG_H2_MSB_REG) << 8) + hx_drv_bme280_readRegister(BME280_DIG_H2_LSB_REG)));
	hx_drv_bme280_calibration.dig_H3 = ((uint8_t)(hx_drv_bme280_readRegister(BME280_DIG_H3_REG)));
	hx_drv_bme280_calibration.dig_H4 = ((int16_t)((hx_drv_bme280_readRegister(BME280_DIG_H4_MSB_REG) << 4) + (hx_drv_bme280_readRegister(BME280_DIG_H4_LSB_REG) & 0x0F)));
	hx_drv_bme280_calibration.dig_H5 = ((int16_t)((hx_drv_bme280_readRegister(BME280_DIG_H5_MSB_REG) << 4) + ((hx_drv_bme280_readRegister(BME280_DIG_H4_LSB_REG) >> 4) & 0x0F)));
	hx_drv_bme280_calibration.dig_H6 = ((int8_t)hx_drv_bme280_readRegister(BME280_DIG_H6_REG));

	// setStandbyTime = 0
	hx_drv_qwiic_set_cmd(BME280_I2C_ADDRESS, BME280_CONFIG_REG);
	hx_drv_qwiic_get_data(BME280_I2C_ADDRESS, &readCheck, 1);
	readCheck &= ~((1 << 7) | (1 << 6) | (1 << 5)); //Clear the 7/6/5 bits
	readCheck |= (setStandbyTime << 5);				//Align with bits 7/6/5
	hx_drv_qwiic_set_reg(BME280_I2C_ADDRESS, BME280_CONFIG_REG, readCheck);

	// setFilter = 4, coefficients = 16
	hx_drv_qwiic_set_cmd(BME280_I2C_ADDRESS, BME280_CONFIG_REG);
	hx_drv_qwiic_get_data(BME280_I2C_ADDRESS, &readCheck, 1);
	readCheck &= ~((1 << 4) | (1 << 3) | (1 << 2)); //Clear the 4/3/2 bits
	readCheck |= (setFilter << 2);					//Align with bits 4/3/2
	hx_drv_qwiic_set_reg(BME280_I2C_ADDRESS, BME280_CONFIG_REG, readCheck);

	hx_drv_bme280_setMode(MODE_SLEEP); //Config will only be writeable in sleep mode, so first go to sleep mode
	// setPressureOverSample = 5

	hx_drv_qwiic_set_cmd(BME280_I2C_ADDRESS, BME280_CTRL_MEAS_REG);
	hx_drv_qwiic_get_data(BME280_I2C_ADDRESS, &readCheck, 1);
	readCheck &= ~((1 << 4) | (1 << 3) | (1 << 2)); //Clear bits 432
	readCheck |= setPressureOverSample << 2;		//Align overSampleAmount to bits 4/3/2
	hx_drv_qwiic_set_reg(BME280_I2C_ADDRESS, BME280_CTRL_MEAS_REG, readCheck);

	// setHumidityOverSample = 5
	hx_drv_qwiic_set_cmd(BME280_I2C_ADDRESS, BME280_CTRL_HUMIDITY_REG);
	hx_drv_qwiic_get_data(BME280_I2C_ADDRESS, &readCheck, 1);
	readCheck &= ~((1 << 2) | (1 << 1) | (1 << 0)); //Clear bits 2/1/0
	readCheck |= setHumidityOverSample << 0;		//Align overSampleAmount to bits 2/1/0
	hx_drv_qwiic_set_reg(BME280_I2C_ADDRESS, BME280_CTRL_HUMIDITY_REG, readCheck);

	// setTempOverSample = 5

	hx_drv_qwiic_set_cmd(BME280_I2C_ADDRESS, BME280_CTRL_MEAS_REG);
	hx_drv_qwiic_get_data(BME280_I2C_ADDRESS, &readCheck, 1);
	readCheck &= ~((1 << 7) | (1 << 6) | (1 << 5)); //Clear bits 765
	readCheck |= setTempOverSample << 5;			//Align overSampleAmount to bits 7/6/5
	hx_drv_qwiic_set_reg(BME280_I2C_ADDRESS, BME280_CTRL_MEAS_REG, readCheck);

	hx_drv_bme280_setMode(MODE_NORMAL);

	return HX_DRV_QWIIC_PASS;
}

HX_DRV_QWIIC_ERROR_E hx_drv_bme280_setMode(int8_t mode)
{
	if (mode > 0b11)
		mode = 0; //Error check. Default to sleep mode

	uint8_t controlData;
	hx_drv_qwiic_set_cmd(BME280_I2C_ADDRESS, BME280_CTRL_MEAS_REG);
	hx_drv_qwiic_get_data(BME280_I2C_ADDRESS, &controlData, 1);
	controlData &= ~((1 << 1) | (1 << 0)); //Clear the mode[1:0] bits
	controlData |= mode;				   //Set
	hx_drv_qwiic_set_reg(BME280_I2C_ADDRESS, BME280_CTRL_MEAS_REG, controlData);

	return HX_DRV_QWIIC_PASS;
}

HX_DRV_QWIIC_ERROR_E hx_drv_bme280_receive_TempC(float *data)
{
	uint8_t buffer[3];
	if (hx_drv_qwiic_set_cmd(BME280_I2C_ADDRESS, BME280_TEMPERATURE_MSB_REG) != HX_DRV_QWIIC_PASS)
	{
		return HX_DRV_QWIIC_ERROR;
	}
	if (hx_drv_qwiic_get_data(BME280_I2C_ADDRESS, buffer, 3) != HX_DRV_QWIIC_PASS)
	{
		return HX_DRV_QWIIC_ERROR;
	}
	int32_t adc_T = ((uint32_t)buffer[0] << 12) | ((uint32_t)buffer[1] << 4) | ((buffer[2] >> 4) & 0x0F);

	//By datasheet, calibrate
	int64_t var1, var2;

	var1 = ((((adc_T >> 3) - ((int32_t)hx_drv_bme280_calibration.dig_T1 << 1))) * ((int32_t)hx_drv_bme280_calibration.dig_T2)) >> 11;
	var2 = (((((adc_T >> 4) - ((int32_t)hx_drv_bme280_calibration.dig_T1)) * ((adc_T >> 4) - ((int32_t)hx_drv_bme280_calibration.dig_T1))) >> 12) *
			((int32_t)hx_drv_bme280_calibration.dig_T3)) >>
		   14;
	t_fine = var1 + var2;
	float output = (t_fine * 5 + 128) >> 8;

	//settings.tempCorrection = 0.f; // correction of temperature - added to the result
	*data = output / 100 + 0.f;

	return HX_DRV_QWIIC_PASS;
}

HX_DRV_QWIIC_ERROR_E hx_drv_bme280_receive_FloatPressure(float *data)
{
	// Returns pressure in Pa as unsigned 32 bit integer in Q24.8 format (24 integer bits and 8 fractional bits).
	// Output value of ??24674867?? represents 24674867/256 = 96386.2 Pa = 963.862 hPa
	uint8_t buffer[3];
	if (hx_drv_qwiic_set_cmd(BME280_I2C_ADDRESS, BME280_PRESSURE_MSB_REG) != HX_DRV_QWIIC_PASS)
	{
		return HX_DRV_QWIIC_ERROR;
	}
	if (hx_drv_qwiic_get_data(BME280_I2C_ADDRESS, buffer, 3) != HX_DRV_QWIIC_PASS)
	{
		return HX_DRV_QWIIC_ERROR;
	}
	int32_t adc_P = ((uint32_t)buffer[0] << 12) | ((uint32_t)buffer[1] << 4) | ((buffer[2] >> 4) & 0x0F);

	int64_t var1, var2, p_acc;
	var1 = ((int64_t)t_fine) - 128000;
	var2 = var1 * var1 * (int64_t)hx_drv_bme280_calibration.dig_P6;
	var2 = var2 + ((var1 * (int64_t)hx_drv_bme280_calibration.dig_P5) << 17);
	var2 = var2 + (((int64_t)hx_drv_bme280_calibration.dig_P4) << 35);
	var1 = ((var1 * var1 * (int64_t)hx_drv_bme280_calibration.dig_P3) >> 8) + ((var1 * (int64_t)hx_drv_bme280_calibration.dig_P2) << 12);
	var1 = (((((int64_t)1) << 47) + var1)) * ((int64_t)hx_drv_bme280_calibration.dig_P1) >> 33;
	if (var1 == 0)
	{
		return 0; // avoid exception caused by division by zero
	}
	p_acc = 1048576 - adc_P;
	p_acc = (((p_acc << 31) - var2) * 3125) / var1;
	var1 = (((int64_t)hx_drv_bme280_calibration.dig_P9) * (p_acc >> 13) * (p_acc >> 13)) >> 25;
	var2 = (((int64_t)hx_drv_bme280_calibration.dig_P8) * p_acc) >> 19;
	p_acc = ((p_acc + var1 + var2) >> 8) + (((int64_t)hx_drv_bme280_calibration.dig_P7) << 4);

	//*data = p_acc / 256.0;
	*data = (uint32_t)p_acc >> 8;

	return HX_DRV_QWIIC_PASS;
}

HX_DRV_QWIIC_ERROR_E hx_drv_bme280_receive_IntPressure(uint32_t *data)
{
	uint8_t buffer[3];
	if (hx_drv_qwiic_set_cmd(BME280_I2C_ADDRESS, BME280_PRESSURE_MSB_REG) != HX_DRV_QWIIC_PASS)
	{
		return HX_DRV_QWIIC_ERROR;
	}
	if (hx_drv_qwiic_get_data(BME280_I2C_ADDRESS, buffer, 3) != HX_DRV_QWIIC_PASS)
	{
		return HX_DRV_QWIIC_ERROR;
	}
	int32_t adc_P = ((uint32_t)buffer[0] << 12) | ((uint32_t)buffer[1] << 4) | ((buffer[2] >> 4) & 0x0F);

	int32_t var1, var2;
	uint32_t p;
	var1 = (((int32_t)t_fine) >> 1) - (int32_t)64000;
	var2 = (((var1 >> 2) * (var1 >> 2)) >> 11) * ((int32_t)hx_drv_bme280_calibration.dig_P6);
	var2 = var2 + ((var1 * ((int32_t)hx_drv_bme280_calibration.dig_P5)) << 1);
	var2 = (var2 >> 2) + (((int32_t)hx_drv_bme280_calibration.dig_P4) << 16);
	var1 = (((hx_drv_bme280_calibration.dig_P3 * (((var1 >> 2) * (var1 >> 2)) >> 13)) >> 3) + ((((int32_t)hx_drv_bme280_calibration.dig_P2) * var1) >> 1)) >> 18;
	var1 = ((((32768 + var1)) * ((int32_t)hx_drv_bme280_calibration.dig_P1)) >> 15);
	if (var1 == 0)
	{
		*data = 0;
		return HX_DRV_QWIIC_PASS; // avoid exception caused by division by zero
	}
	p = (((uint32_t)(((int32_t)1048576) - adc_P) - (var2 >> 12))) * 3125;
	if (p < 0x80000000)
	{
		p = (p << 1) / ((uint32_t)var1);
	}
	else
	{
		p = (p / (uint32_t)var1) * 2;
	}
	var1 = (((int32_t)hx_drv_bme280_calibration.dig_P9) * ((int32_t)(((p >> 3) * (p >> 3)) >> 13))) >> 12;
	var2 = (((int32_t)(p >> 2)) * ((int32_t)hx_drv_bme280_calibration.dig_P8)) >> 13;
	p = (uint32_t)((int32_t)p + ((var1 + var2 + hx_drv_bme280_calibration.dig_P7) >> 4));

	*data = p;

	return HX_DRV_QWIIC_PASS;
}

HX_DRV_QWIIC_ERROR_E hx_drv_bme280_receive_FloatHumidity(float *data)
{

	// Returns humidity in %RH as unsigned 32 bit integer in Q22. 10 format (22 integer and 10 fractional bits).
	// Output value of ??47445?? represents 47445/1024 = 46. 333 %RH
	uint8_t buffer[2];
	if (hx_drv_qwiic_set_cmd(BME280_I2C_ADDRESS, BME280_HUMIDITY_MSB_REG) != HX_DRV_QWIIC_PASS)
	{
		return HX_DRV_QWIIC_ERROR;
	}
	if (hx_drv_qwiic_get_data(BME280_I2C_ADDRESS, buffer, 2) != HX_DRV_QWIIC_PASS)
	{
		return HX_DRV_QWIIC_ERROR;
	}

	int32_t adc_H = ((uint32_t)buffer[0] << 8) | ((uint32_t)buffer[1]);

	int32_t var1;
	var1 = (t_fine - ((int32_t)76800));
	var1 = (((((adc_H << 14) - (((int32_t)hx_drv_bme280_calibration.dig_H4) << 20) - (((int32_t)hx_drv_bme280_calibration.dig_H5) * var1)) +
			  ((int32_t)16384)) >>
			 15) *
			(((((((var1 * ((int32_t)hx_drv_bme280_calibration.dig_H6)) >> 10) * (((var1 * ((int32_t)hx_drv_bme280_calibration.dig_H3)) >> 11) + ((int32_t)32768))) >> 10) + ((int32_t)2097152)) *
				  ((int32_t)hx_drv_bme280_calibration.dig_H2) +
			  8192) >>
			 14));
	var1 = (var1 - (((((var1 >> 15) * (var1 >> 15)) >> 7) * ((int32_t)hx_drv_bme280_calibration.dig_H1)) >> 4));
	var1 = (var1 < 0 ? 0 : var1);
	var1 = (var1 > 419430400 ? 419430400 : var1);
	*data = (float)(var1 >> 12) / 1024.0;
	return HX_DRV_QWIIC_PASS;
}
