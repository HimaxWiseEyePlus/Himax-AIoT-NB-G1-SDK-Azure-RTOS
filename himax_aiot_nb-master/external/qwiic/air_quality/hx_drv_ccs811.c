/*
 * hx_drv_ccs811.c
 *
 *  Created on: 2021/1/11
 *      Author: 904212
 */

#include "hx_drv_ccs811.h"

uint8_t CCS_811_ADDRESS = CCS_811_ADDRESS_5A;

HX_DRV_QWIIC_ERROR_E hx_drv_ccs811_initial(uint8_t i2caddress)
{
	CCS_811_ADDRESS = i2caddress == 0 ? CCS_811_ADDRESS_5A : CCS_811_ADDRESS_5B; 

	uint8_t readCheck;
	HX_DRV_QWIIC_ERROR_E retErr;

	// check HW id
	hx_drv_qwiic_set_cmd(CCS_811_ADDRESS, HW_ID_REG);
	hx_drv_qwiic_get_data(CCS_811_ADDRESS, &readCheck, 1);

	if (readCheck != 0x81)
		return HX_DRV_QWIIC_ERROR;

	// Check For Status Error
	hx_drv_qwiic_set_cmd(CCS_811_ADDRESS, STATUS_REG);
	hx_drv_qwiic_get_data(CCS_811_ADDRESS, &readCheck, 1);
	if ((readCheck & 1 << 0) == true)
		return HX_DRV_QWIIC_ERROR;

	// Check appValid
	hx_drv_qwiic_set_cmd(CCS_811_ADDRESS, STATUS_REG);
	hx_drv_qwiic_get_data(CCS_811_ADDRESS, &readCheck, 1);
	if ((readCheck & 1 << 4) == false)
		return HX_DRV_QWIIC_ERROR;

	// Write 0 bytes to APP_START_REG to start app
	retErr = hx_drv_qwiic_set_cmd(CCS_811_ADDRESS, APP_START_REG);
	if (retErr != HX_DRV_QWIIC_PASS)
	{
		return HX_DRV_QWIIC_ERROR;
	}

	// Set Drive Mode
	retErr = hx_drv_qwiic_set_cmd(CCS_811_ADDRESS, MEAS_MODE_REG);
	if (retErr == HX_DRV_QWIIC_PASS)
	{
		hx_drv_qwiic_get_data(CCS_811_ADDRESS, &readCheck, 1);
		readCheck &= ~(0b00000111 << 4);
		readCheck |= (1 << 4);
		hx_drv_qwiic_set_reg(CCS_811_ADDRESS, MEAS_MODE_REG, readCheck);
	}
	else
	{
		return HX_DRV_QWIIC_ERROR;
	}

	return HX_DRV_QWIIC_PASS;
}

HX_DRV_QWIIC_ERROR_E hx_drv_ccs811_receive_CO2_and_tVOC(uint16_t *CO2, uint16_t *tVOC)
{
	uint8_t readCheck;
	uint8_t data[4];
	hx_drv_qwiic_set_cmd(CCS_811_ADDRESS, STATUS_REG);
	hx_drv_qwiic_get_data(CCS_811_ADDRESS, &readCheck, 1);
	if (readCheck & 0x8)
	{
		hx_drv_qwiic_set_cmd(CCS_811_ADDRESS, ALG_RESULT_DATA);
		hx_drv_qwiic_get_data(CCS_811_ADDRESS, data, 4);

		*CO2 = ((uint16_t)data[0] << 8) | data[1];
		*tVOC = ((uint16_t)data[2] << 8) | data[3];

		return HX_DRV_QWIIC_PASS;
	}
	else
	{
		return HX_DRV_QWIIC_ERROR;
	}
}

HX_DRV_QWIIC_ERROR_E hx_drv_ccs811_setEnvironmentalData(float relativeHumidity, float temperature)
{
	//Check for invalid temperatures
	if ((temperature < -25) || (temperature > 50))
		return HX_DRV_QWIIC_ERROR;

	//Check for invalid humidity
	if ((relativeHumidity < 0) || (relativeHumidity > 100))
		return HX_DRV_QWIIC_ERROR;

	uint32_t rH = relativeHumidity * 1000; //42.348 becomes 42348
	uint32_t temp = temperature * 1000;	   //23.2 becomes 23200

	uint8_t envData[4];

	envData[0] = (rH + 250) / 500;
	envData[1] = 0; //CCS811 only supports increments of 0.5 so bits 7-0 will always be zero

	temp += 25000; //Add the 25C offset

	//Correct rounding
	envData[2] = (temp + 250) / 500;
	envData[3] = 0;

	hx_drv_qwiic_set_multibyte_reg(CCS_811_ADDRESS, ENV_DATA, envData, 4);

	return HX_DRV_QWIIC_PASS;
}