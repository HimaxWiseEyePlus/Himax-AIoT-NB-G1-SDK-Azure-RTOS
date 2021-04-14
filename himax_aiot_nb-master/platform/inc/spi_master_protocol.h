
#ifndef INC_SPI_M_PROTOCOL_H_
#define INC_SPI_M_PROTOCOL_H_

#include "dev_common.h"
#include "spi_protocol.h"

typedef void (*spi_mst_protocol_cb_t) (void );

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief	spi master 1 open device
 *
 * \return	error id.
 */
extern int hx_drv_spi_mst_open();
/**
 * \brief	spi master 1 use protocol to send image out. data type in the size packet will be "JPG = 0x01"
 *
 * \param[in]	SRAM_addr	 address of image data to transfer
 * \param[in]	img_size	 image data size
 *
 * \return	error id.
 */


extern int hx_drv_spi_mst_open_speed(uint32_t clk_hz);
extern int hx_drv_spi_mst_protocol_write(uint32_t SRAM_addr, uint32_t img_size);
extern int hx_drv_spi_mst_protocol_write_cus(uint32_t SRAM_addr, uint32_t img_size, uint32_t header_len);
/**
 * \brief	spi master 1 use protocol to send data out. data type should be specified
 *
 * \param[in]	SRAM_addr	 address of data to transfer
 * \param[in]	img_size	 data size
 * \param[in]	data_type	 data size
 *
 * \return	error id.
 */
extern int hx_drv_spi_mst_protocol_write_sp(uint32_t SRAM_addr, uint32_t img_size, SPI_CMD_DATA_TYPE data_type);
extern int hx_drv_spi_mst_protocol_write_halt();
//TODO: implement master 1 read back function
extern int hx_drv_spi_mst_protocol_read(uint32_t SRAM_addr, uint32_t *img_size);
extern int hx_drv_spi_mst_protocol_register_tx_cb(spi_mst_protocol_cb_t aRWritecb);
extern int hx_drv_spi_mst_protocol_register_rx_cb(spi_mst_protocol_cb_t aReadcb);

extern int hx_drv_spi_flash_open(uint8_t dev_no);
extern int hx_drv_spi_flash_open_speed(uint32_t clk_hz);
extern int hx_drv_spi_flash_close(uint8_t dev_no);
extern int hx_drv_spi_flash_protocol_eraseall(uint8_t dev_no);
extern int hx_drv_spi_flash_protocol_erase_sector(uint8_t dev_no, uint32_t flash_addr);
extern int hx_drv_spi_flash_protocol_write(uint8_t dev_no,uint32_t flash_addr, uint32_t SRAM_addr, uint32_t len, uint8_t mode);
extern int hx_drv_spi_flash_protocol_read(uint8_t dev_no,uint32_t flash_addr, uint32_t SRAM_addr, uint32_t len, uint8_t mode);

#ifdef __cplusplus
}
#endif
#endif
