/*
 * hx_drv_qwiic.c
 *
 *  Created on: 2021¦~1¤ë11¤é
 *      Author: 902449
 */

#include "embARC.h"
#include "embARC_debug.h"

#include "hx_drv_qwiic.h"

#define HX_QWIIC_IIC_M_ID         SS_IIC_1_ID
#define HX_QWIIC_I2C_ADDR_BYTE    1               /**< I2C Master Register address length*/
#define HX_QWIIC_I2C_DATA_BYTE    1               /**< I2C Master Register value length*/

#define HX_QWIIC_I2C_RETRY_TIME  3  /**< If I2C Master set fail, maximum retry time for imu setting*/


HX_DRV_QWIIC_ERROR_E hx_drv_qwiic_set_reg(uint8_t slv_addr, uint8_t addr, uint8_t data)
{
    uint8_t regAddr[HX_QWIIC_I2C_ADDR_BYTE]  = {addr};
    uint8_t wBuffer[HX_QWIIC_I2C_DATA_BYTE] = {data};
    int32_t retI2C = 0;

    for(int i = 0;i<HX_QWIIC_I2C_RETRY_TIME;i++) {
      retI2C = hx_drv_i2cm_write_data(HX_QWIIC_IIC_M_ID, slv_addr, regAddr, HX_QWIIC_I2C_ADDR_BYTE, wBuffer, HX_QWIIC_I2C_DATA_BYTE);
      if(retI2C == E_OK)
        return HX_DRV_QWIIC_PASS;
    }

    return HX_DRV_QWIIC_ERROR;
}

HX_DRV_QWIIC_ERROR_E hx_drv_qwiic_set_multibyte_reg(uint8_t slv_addr, uint8_t addr, uint8_t *data, uint8_t data_len)
{
    uint8_t regAddr[HX_QWIIC_I2C_ADDR_BYTE]  = {addr};
    int32_t retI2C = 0;

    for(int i = 0;i<HX_QWIIC_I2C_RETRY_TIME;i++) {
      retI2C = hx_drv_i2cm_write_data(HX_QWIIC_IIC_M_ID, slv_addr, regAddr, HX_QWIIC_I2C_ADDR_BYTE, data, data_len);
      if(retI2C == E_OK)
        return HX_DRV_QWIIC_PASS;
    }

    return HX_DRV_QWIIC_ERROR;
}

HX_DRV_QWIIC_ERROR_E hx_drv_qwiic_get_reg(uint8_t slv_addr, uint8_t addr, uint8_t *data)
{
    uint8_t regAddr[HX_QWIIC_I2C_ADDR_BYTE]  = {addr};
    uint8_t rBuffer[HX_QWIIC_I2C_DATA_BYTE] = {0x00};
    int32_t retI2C = 0;

    *data = 0;

    retI2C = hx_drv_i2cm_writeread(HX_QWIIC_IIC_M_ID, slv_addr, regAddr, HX_QWIIC_I2C_ADDR_BYTE, rBuffer, HX_QWIIC_I2C_DATA_BYTE);
    if(retI2C < E_OK)
    {
        return HX_DRV_QWIIC_ERROR;
    }

   	*data = rBuffer[0];

    return HX_DRV_QWIIC_PASS;
}

HX_DRV_QWIIC_ERROR_E hx_drv_qwiic_set_cmd(uint8_t slv_addr, uint8_t cmd)
{
    uint8_t regAddr[HX_QWIIC_I2C_ADDR_BYTE]  = {cmd};
    int32_t retI2C = 0;

    for(int i = 0;i<HX_QWIIC_I2C_RETRY_TIME;i++) {
      retI2C = hx_drv_i2cm_write_data(HX_QWIIC_IIC_M_ID, slv_addr, regAddr, HX_QWIIC_I2C_ADDR_BYTE, NULL, 0);
      if(retI2C >= E_OK)
        return HX_DRV_QWIIC_PASS;
    }

    return HX_DRV_QWIIC_ERROR;
}

HX_DRV_QWIIC_ERROR_E hx_drv_qwiic_get_data(uint8_t slv_addr, uint8_t *data, uint8_t data_len)
{
	if(hx_drv_i2cm_read_data(HX_QWIIC_IIC_M_ID, slv_addr, data, data_len) < E_OK)
		return HX_DRV_QWIIC_ERROR;
	else
		return HX_DRV_QWIIC_PASS;
}
