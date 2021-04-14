/*
 * wnb303r.c
 *
 *  Created on: 2020/12/5
 *      Author: 903990
 */


#include <wnb303r.h>
#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "stdarg.h"

#define DBG_MORE
#include "embARC_debug.h"
#include "tx_api.h"

/**
* WNB303R driver initial
*
* @param uart_id  :uart_no
* @param baudrate :serial baudrate
*
* Return Value:
* AT_OK, 	initial success
* AT_ERROR, initial fail
*/
DEV_UART_PTR wnb303r_drv_init(USE_SS_UART_E uart_id, uint32_t baudrate){

	DEV_UART_PTR dev_uart_comm = NULL;
	dev_uart_comm = hx_drv_uart_get_dev(uart_id);
	if(dev_uart_comm != NULL){
		dev_uart_comm->uart_open(baudrate);
	}

	return dev_uart_comm;
}

/**
* WNB303R driver deinit
*
* @param uart_id:uart_no
*
* Return Value:
* AT_OK, 	deinit success
* AT_ERROR, deinit fail
*/
int32_t wnb303r_drv_deinit(unsigned char uart_id){

	int ret = hx_drv_uart_deinit(uart_id);

	if(ret !=0){
		printf("wnb303r_deinit error_no=%d\n",ret);
		return AT_ERROR;
	}

	return AT_OK;
}


/**
* WNB303R read
*
* @param dev_uart_com:the dev_uart_comm is serial port object
* @param rev_buf     :
* @param len         :
*
* Return Value:
* AT_OK, 	 success
* AT_ERROR,  fail
*/
int32_t wnb303r_drv_read(DEV_UART_PTR dev_uart_com, char *rev_buf, uint32_t len)
{
	//TBD
}

/**
* WNB303R write AT command to nbiot
*
* @param dev_uart_comm:the dev_uart_comm is serial port object
* @param mode 		  :AT_MODE
* @param command      :at command
*
* Return Value:
* -1,  fail
* else,success
*/

int32_t wnb303r_drv_write_at_cmd(DEV_UART_PTR dev_uart_comm,AT_MODE mode, AT_STRING command, ...)
{
	TX_INTERRUPT_SAVE_AREA
		//printf("wnb303r_drv_write_at_cmd!!\n");
		char at_cmd[AT_MAX_LEN] = AT_PREFIX;

		va_list vl;
		char * str = command;
		if(str == NULL){
			printf("[%s]%d: command is NULL, send AT test command\r\n", __FUNCTION__, __LINE__);
		} else {
			strcat(at_cmd,"+");
			strcat(at_cmd, command);
			switch(mode){
				case AT_LIST:
					strcat(at_cmd, "=?");
					break;
				case AT_READ:
					strcat(at_cmd, "?");
					break;
				case AT_WRITE:
					strcat(at_cmd, "=");
					va_start(vl, command);
					for(int i = 0; i < AT_MAX_PARAMETER; i++){
						str = va_arg(vl, AT_STRING);
						if(str == NULL){
							break;
						}
						if(i != 0){
							strcat(at_cmd, ",");
						}
						strcat(at_cmd, str);
					}
					va_end(vl);
					break;
				case AT_EXECUTE:
				default:
					break;
			}
		}
		#ifdef AT_ADD_POSTFIX
			strcat(at_cmd, AT_POSTFIX);
		#endif /*AT_ADD_POSTFIX*/
{
		//xprintf("at_cmd:\"%s\" ,length:(%d)\r\n",at_cmd, strlen(at_cmd));
		int len = 0;
		len = dev_uart_comm->uart_write(at_cmd,strlen(at_cmd));
		//xprintf("at_cmd length:(%d)\r\n", len);
		//if( 0 > dev_uart_comm->uart_write(at_cmd,strlen(at_cmd)))
		if(0 > len)
		{
			xprintf("at cmd send fail.\n");
			return AT_ERROR;
		}
}

		return AT_OK;
}

/**
* Control light/deep sleep mode and query the sleep mode status
*
* @param dev_uart_comm  :the dev_uart_comm is serial port object
* @param parameter		:operation

* AT+SM = <op>
*
* <op>
*
* STATUS:show all current sleep locks status in GKI log once. (for debugging)
* LOCK:acquire a temporary lock to prevent system enter sleep
* UNLOCK:release the temporary lock acquired by AT+SM=LOCK command.
* LOCK_FOREVER: Acquire a lock to prevent system enter sleep. This lock will be written into NVDM, so it will still valid even after reboo t.
* UNLOCK_FOREVER: Release lock acquired by AT+SM=LOCK_FOREVER command, and clear the NVDM data.
* SHOWINFO: Show all current sleep locks status in GKI log in user defined frequency. (for debugging) if <show> is 1, the lock status will be shown in GKI log in < freq> frequency.
  The frequency means how many times the idle task be scheduled, if it reaches <freq>, show lock status. All these setting will be written into NVDM, will still take effect after reboot.
*
* Return Value:
* AT_OK, 	 success
* AT_ERROR,  fail
*/

int32_t wnb303r_drv_control_sleep_mode(DEV_UART_PTR dev_uart_comm, AT_STRING parameter)
{
	//AT+IPCONFIG
		if(0 > wnb303r_drv_write_at_cmd(dev_uart_comm, AT_WRITE,"SM" ,parameter, NULL))
		{
			return AT_ERROR;
		}

		return AT_OK;
}


/**
* WNB303R driver software reset
*
* @param uart_id:uart_no
* @param parameter : fixed 1
  - 1: Trigger WDT reset immediately.

* AT+ EWDT
* Return Value:
* AT_OK, 	deinit success
* AT_ERROR, deinit fail
*/
int32_t wnb303r_drv_sw_reset(DEV_UART_PTR dev_uart_comm, AT_STRING parameter)
{
	if( 0 > wnb303r_drv_write_at_cmd(dev_uart_comm, AT_WRITE,"EWDT",parameter, NULL)){
			return AT_ERROR;
	}
	return AT_OK;
}


/**
* Query the device IP address
*
* @param dev_uart_comm  :the dev_uart_comm is serial port object

* AT+IPCONFIG
*
* +IPCONFIG: fe80:0:0:0:e518:5891:50d1:19d6
* +IPCONFIG: 2001:b400:e23e:5b16:e518:5891:50d1:19d6
* +IPCONFIG: 10.193.167.134
* +IPCONFIG: 127.0.0.1
*
* Return Value:
* AT_OK, 	 success
* AT_ERROR,  fail
*/
int32_t wnb303r_query_ip(DEV_UART_PTR dev_uart_comm)
{
	//AT+IPCONFIG
	if(0 > wnb303r_drv_write_at_cmd(dev_uart_comm, AT_EXECUTE, "IPCONFIG", NULL))
	{
		return AT_ERROR;
	}

	return AT_OK;
}


/**
* Query current time
*
* @param dev_uart_comm  :the dev_uart_comm is serial port object

* AT+CCLK?
*
* AT+CCLK?
* +CCLK:2021/02/22,11:17:44GMT+8
* Return Value:
* AT_OK, 	 success
* AT_ERROR,  fail
*/
int32_t wnb303r_query_time(DEV_UART_PTR dev_uart_comm)
{
	//AT+CCLK?
	if(0 > wnb303r_drv_write_at_cmd(dev_uart_comm, AT_READ, "CCLK", NULL))
	{
		return AT_ERROR;
	}

	return AT_OK;
}

/**
* Install MQTT certification
*
* @param dev_uart_comm  :the dev_uart_comm is serial port object
* @param cer_type 	    :MQTT certification's type
* @param cer_flag       :MQTT certification's flag
* @param cer_totalsize  :MQTT certification's total size
* @param cer_currentsize:MQTT certification's size within the current command
* @param cer_raw_data   :MQTT certification ¡¦s raw data , must be HEX string
*
* AT+EMQCERT
*
* Return Value:
* AT_OK, 	 success
* AT_ERROR,  fail
*/
//int32_t wnb303r_MQTT_certification(DEV_UART_PTR dev_uart_comm, AT_STRING cer_type, AT_STRING cer_flag, AT_STRING cer_totalsize, AT_STRING cer_currentsize, AT_STRING cer_raw_data)
int32_t wnb303r_MQTT_certification(DEV_UART_PTR dev_uart_comm, AT_STRING cer_type, AT_STRING cer_flag, AT_STRING cer_totalsize, AT_STRING cer_currentsize)
{

	//AT+EMQCERT
	//if(0 > wnb303r_drv_write_at_cmd(dev_uart_comm, AT_WRITE,"EMQCERT",cer_type,cer_flag,cer_totalsize, \
									cer_currentsize,cer_raw_data, NULL))
	if(0 > wnb303r_drv_write_at_cmd(dev_uart_comm, AT_WRITE,"EMQCERT",cer_type,cer_flag,cer_totalsize, \
										cer_currentsize,NULL))
	{
		return AT_ERROR;
	}

	return AT_OK;
}

/**
* creating MQTT connection with server over TCP
*
* @param dev_uart_comm     :the dev_uart_comm is serial port object
* @param remote_ip 	       :MQTT server IP address
* @param remote_port       :MQTT server port
* @param command_timeout_ms:AT command timeout (ms)
* @param rw_buf_size       :Send buffer and read buffer size
*
* AT+EMQNEW
*
* Return Value:
* AT_OK, 	 success
* AT_ERROR,  fail
*/
int32_t wnb303r_MQTT_create(DEV_UART_PTR dev_uart_comm, AT_STRING remote_ip, AT_STRING remote_port, AT_STRING command_timeout_ms, AT_STRING rw_buf_size)
{
	//AT+EMQNEW
	if( 0 > wnb303r_drv_write_at_cmd(dev_uart_comm, AT_WRITE,"EMQNEW",remote_ip,remote_port,\
			command_timeout_ms,rw_buf_size,NULL))
	{
		return AT_ERROR;
	}

	return AT_OK;
}

/**
* MQTT connection packet
*
* @param dev_uart_comm     :the dev_uart_comm is serial port object
* @param mqtt_id 	       :mqtt_id from AT+EMQNEW¡¦s response
* @param version           :MQTT version, can be 3 or 4
* @param client_id		   :client ID, would be unique
* @param keepalive_interval:keep alive interval, means ping interval
* Suggest that query your cooperation platform about the recommended settings as the value of keepalive_interval
* @param cleansession      :clean session, can be 0 or 1.
* @param will_flag		   :will flag, can be 0 or 1. (when nbiot exception disconnect, server will send Last Will and Testament to topic)
* @param user_name 		   :user name (option)
* @param password		   :password (option)
*
*AT+EMQCON
*
* Return Value:
* AT_OK, 	 success
* AT_ERROR,  fail
*/
int32_t wnb303r_MQTT_send_connect_packet(DEV_UART_PTR dev_uart_comm, AT_STRING mqtt_id, AT_STRING version, AT_STRING client_id, AT_STRING keepalive_interval, \
					AT_STRING cleansession, AT_STRING will_flag, AT_STRING user_name, AT_STRING	password)
{
	if( 0 > wnb303r_drv_write_at_cmd(dev_uart_comm, AT_WRITE,"EMQCON",mqtt_id,version,client_id, \
	keepalive_interval,cleansession,will_flag, user_name, password, NULL)){
		return AT_ERROR;
	}

//	if(user_name !="" & password!="")
//	{
//		if( 0 > wnb303r_drv_write_at_cmd(dev_uart_comm, AT_WRITE,"EMQCON",mqtt_id,version,client_id, \
//		keepalive_interval,cleansession,will_flag, user_name, password, NULL)){
//			return AT_ERROR;
//		}
//	}
//	else{
//		//AT+EMQCON
//		if( 0 > wnb303r_drv_write_at_cmd(dev_uart_comm, AT_WRITE,"EMQCON",mqtt_id,version,client_id, \
//				keepalive_interval,cleansession,will_flag,NULL)){
//			return AT_ERROR;
//		}
//	}

	return AT_OK;
}

/**
* MQTT subscribe packet
*
* @param dev_uart_comm:the dev_uart_comm is serial port object
* @param mqtt_id 	  :mqtt_id from AT+EMQNEW¡¦s response
* @param topic        :topic of subscribe message.
* @param qos		  :message QoS(Quality of Service) ,can be 0, 1 or 2
* 	level 0¡Gat most once
	level 1¡Gat last once
	level 2¡Gexactly once
* AT+EMQSUB
*
* Return Value:
* AT_OK, 	 success
* AT_ERROR,  fail
*/
int32_t wnb303r_MQTT_send_subscribe_packet(DEV_UART_PTR dev_uart_comm, AT_STRING mqtt_id, AT_STRING topic, AT_STRING qos)
{
	//AT+EMQSUB
	if( 0 > wnb303r_drv_write_at_cmd(dev_uart_comm, AT_WRITE,"EMQSUB",mqtt_id,topic,qos,NULL))
	{
		return AT_ERROR;
	}

	return AT_OK;
}

/**
* MQTT publish packet
*
* @param dev_uart_comm:the dev_uart_comm is serial port object
* @param mqtt_id 	  :mqtt_id from AT+EMQNEW¡¦s response
* @param topic        :topic of publish message
* @param qos		  :message ¡¦s QoS level , can be 0, 1 or2
* 	level 0¡Gat most once
	level 1¡Gat last once
	level 2¡Gexactly once
* @param retain_flag  :retained flag, can be 0 or 1
* @param retain_flag2 :retained flag, fix 0
* @param msg_len  ¡@¡@:length of publish message
* @param msg		  :publish message
*
* AT+EMQPUB
*
* Return Value:
* AT_OK, 	 success
* AT_ERROR,  fail
*/
int32_t wnb303r_MQTT_send_publish_packet(DEV_UART_PTR dev_uart_comm, AT_STRING mqtt_id, AT_STRING topic, AT_STRING qos, AT_STRING retain_flag, AT_STRING retain_flag2, AT_STRING msg_len, AT_STRING msg)
{
	//AT+EMQPUB
	if( 0 > wnb303r_drv_write_at_cmd(dev_uart_comm, AT_WRITE,"EMQPUB",mqtt_id,topic,qos, \
			retain_flag, retain_flag2, msg_len,msg,NULL))
	{
		return AT_ERROR;
	}

	return AT_OK;
}

/**
* MQTT disconnect
*
* @param dev_uart_comm:the dev_uart_comm is serial port object
* @param mqtt_id 	  :mqtt_id from AT+EMQNEW¡¦s response
*
* AT+EMQDISCON
*
* Return Value:
* AT_OK, 	 success
* AT_ERROR,  fail
*/
int32_t wnb303r_MQTT_disconnect(DEV_UART_PTR dev_uart_comm,AT_STRING mqtt_id)
{
	//AT+EMQDISCON
	if( 0 > wnb303r_drv_write_at_cmd(dev_uart_comm, AT_WRITE,"EMQDISCON",mqtt_id,NULL))
	{
		return AT_ERROR;
	}

	return AT_OK;
}


/**
* WNB303R create TCP/UDP socket
*
* @param dev_uart_comm	:the dev_uart_comm is serial port object
* @param internet_layer :Internet Protocol version
* @param transport_layer:remote port
* @param protocol  		:remote address
*
* AT+ESOC
*
* Return Value:
* AT_OK ,  success
* AT_ERROR, fail
*/
int32_t wnb303r_SOCKET_create(DEV_UART_PTR dev_uart_comm,AT_STRING internet_layer, AT_STRING transport_layer, AT_STRING protocol)
{
	//AT+ESOC
	if( 0 > wnb303r_drv_write_at_cmd(dev_uart_comm, AT_WRITE,"ESOC",internet_layer, transport_layer, protocol, NULL))
	{
		return AT_ERROR;
	}

	return AT_OK;
}

/**
* WNB303R connect socket to remote
*
* @param dev_uart_comm:the dev_uart_comm is serial port object
* @param socket_id    :socket id, wnb303r_create_socket() response.
* @param port         :remote port
* @param address      :remote address
*
* AT+ESOCON
*
* Return Value:
* AT_OK ,  success
* AT_ERROR, fail
*/
int32_t wnb303r_SOCKET_connect_to_remote(DEV_UART_PTR dev_uart_comm, AT_STRING socket_id, AT_STRING port, AT_STRING address)
{
	//AT+ESOCON
	if( 0 > wnb303r_drv_write_at_cmd(dev_uart_comm, AT_WRITE,"ESOCON",socket_id, port, address, NULL))
	{
		return AT_ERROR;
	}

	return AT_OK;

}


/**
* WNB303R send data to remote
*
* @param dev_uart_comm:the dev_uart_comm is serial port object
* @param socket_id    :socket id, wnb303r_create_socket() response.
* @param buf          :data buffer
* @param len          :data length
*
*
* AT+ESOSEND
*
* Return Value:
* AT_OK ,  success
* AT_ERROR, fail
*/
int32_t wnb303r_SOCKET_send_data(DEV_UART_PTR dev_uart_comm, AT_STRING socket_id, char *buf, AT_STRING len)
{
	//AT+ESOSEND
	if( 0 > wnb303r_drv_write_at_cmd(dev_uart_comm, AT_WRITE,"ESOSEND",socket_id, len, buf, NULL))
	{
		return AT_ERROR;
	}

	return AT_OK;
}

/**
* WNB303R change a connect socket AT channel into transparent mode.
*
* @param dev_uart_comm:the dev_uart_comm is serial port object
* @param socket_id    :socket id, wnb303r_create_socket() response.
* @param data_mode    :data mode
* 0: close data mode
* 1: create data mode(transparent mode)
*
* AT+ESODATAMODE
*
* Return Value:
* AT_OK ,  success
* AT_ERROR, fail
*/
int32_t wnb303r_SOCKET_transmission_mode(DEV_UART_PTR dev_uart_comm, AT_STRING socket_id, AT_STRING data_mode)
{
	//AT+ESODATAMODE
	if( 0 > wnb303r_drv_write_at_cmd(dev_uart_comm, AT_WRITE,"ESODATAMODE",socket_id, data_mode, NULL))
	{
		return AT_ERROR;
	}

	return AT_OK;
}

/**
* WNB303R change a connect socket AT channel into transparent mode.
*
* @param dev_uart_comm:the dev_uart_comm is serial port object
* @param socket_id    :socket id, wnb303r_create_socket() response.
* @param data_mode    :data length
*
* AT+ESOCL
*
* Return Value:
* AT_OK ,  success
* AT_ERROR, fail
*/
int32_t wnb303r_SOCKET_close(DEV_UART_PTR dev_uart_comm, AT_STRING socket_id)
{
	//AT+ESOCL
	if( 0 > wnb303r_drv_write_at_cmd(dev_uart_comm, AT_WRITE,"ESOCL",socket_id, NULL))
	{
		return AT_ERROR;
	}

	return AT_OK;

}
