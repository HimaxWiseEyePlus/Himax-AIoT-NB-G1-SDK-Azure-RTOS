/*
 * wnb303r.h
 *
 *  Created on: 2020/12/5
 *      Author: 903990
 */

#ifndef EXTERNAL_NB_IOT_WNB303R_WNB303R_H_
#define EXTERNAL_NB_IOT_WNB303R_WNB303R_H_

#include "board.h"
#include "hx_drv_uart.h"

#define AT_ADD_POSTFIX

#define AT_PREFIX 			"AT"
#define AT_POSTFIX			"\r\n"
#define AT_OK_STR			"OK"
#define AT_ERROR_STR		"ERROR"

#define AT_MAX_LEN 	    1536//1024//832
#define AT_MAX_PARAMETER 8

#define AT_OK		     0
#define AT_ERROR	     -1

typedef char * AT_STRING;

typedef enum {
	AT_LIST,
	AT_READ,
	AT_WRITE,
	AT_EXECUTE
}AT_MODE;

/*driver initial/deinit*/
DEV_UART_PTR wnb303r_drv_init(USE_SS_UART_E uart_id, uint32_t baudrate);
int32_t wnb303r_drv_deinit(unsigned char uart_id);
int32_t wnb303r_drv_sw_reset(DEV_UART_PTR dev_uart_comm, AT_STRING parameter);
int32_t wnb303r_drv_read(DEV_UART_PTR dev_uart_com, char *rev_buf, uint32_t len);
int32_t wnb303r_drv_write_at_cmd(DEV_UART_PTR dev_uart_comm,AT_MODE mode, AT_STRING command, ...);
int32_t wnb303r_drv_control_sleep_mode(DEV_UART_PTR dev_uart_comm, AT_STRING parameter);

/*Network info querying*/
int32_t wnb303r_query_ip(DEV_UART_PTR dev_uart_comm);
int32_t wnb303r_query_time(DEV_UART_PTR dev_uart_comm);

/*MQTT*/
//int32_t wnb303r_MQTT_certification(DEV_UART_PTR dev_uart_comm, AT_STRING cer_type, AT_STRING cer_flag, AT_STRING cer_totalsize, AT_STRING cer_currentsize, AT_STRING cer_raw_data);
int32_t wnb303r_MQTT_certification(DEV_UART_PTR dev_uart_comm, AT_STRING cer_type, AT_STRING cer_flag, AT_STRING cer_totalsize, AT_STRING cer_currentsize);
int32_t wnb303r_MQTT_create(DEV_UART_PTR dev_uart_comm, AT_STRING remote_ip, AT_STRING remote_port, AT_STRING command_timeout_ms, AT_STRING rw_buf_size);
int32_t wnb303r_MQTT_send_connect_packet(DEV_UART_PTR dev_uart_comm, AT_STRING mqtt_id, AT_STRING version, AT_STRING client_id, AT_STRING keepalive_interval, \
											AT_STRING cleansession, AT_STRING will_flag, AT_STRING user_name, AT_STRING	password);
int32_t wnb303r_MQTT_send_subscribe_packet(DEV_UART_PTR dev_uart_comm, AT_STRING mqtt_id, AT_STRING topic, AT_STRING qos);
int32_t wnb303r_MQTT_send_publish_packet(DEV_UART_PTR dev_uart_comm, AT_STRING mqtt_id, AT_STRING topic, AT_STRING qos, AT_STRING retain_flag, AT_STRING retain_flag2, AT_STRING msg_len, AT_STRING msg);
int32_t wnb303r_MQTT_disconnect(DEV_UART_PTR dev_uart_comm,AT_STRING mqtt_id);

/*SOCKET*/
int32_t wnb303r_SOCKET_create(DEV_UART_PTR dev_uart_comm,AT_STRING internet_layer, AT_STRING transport_layer, AT_STRING protocol);
int32_t wnb303r_SOCKET_connect_to_remote(DEV_UART_PTR dev_uart_comm, AT_STRING socket_id, AT_STRING port, AT_STRING address);
int32_t wnb303r_SOCKET_send_data(DEV_UART_PTR dev_uart_comm, AT_STRING socket_id, char *buf, AT_STRING len);
int32_t wnb303r_SOCKET_transmission_mode(DEV_UART_PTR dev_uart_comm, AT_STRING socket_id, AT_STRING data_mode);
int32_t wnb303r_SOCKET_close(DEV_UART_PTR dev_uart_comm, AT_STRING socket_id);

//int32_t wnb303r_nbiot_disconnect(DEV_UART_PTR dev_uart_comm);
//extern DEV_UART_PTR dev_uart_comm;

//extern uint8_t azure_pnp_iotdps_event ;
//extern uint8_t azure_pnp_iothub_event ;
#endif /* EXTERNAL_NB_IOT_WNB303R_WNB303R_H_ */
