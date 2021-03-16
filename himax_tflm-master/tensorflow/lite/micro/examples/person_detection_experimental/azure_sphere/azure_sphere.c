/**************************************************************************/
/*                                                                        */
/*       Copyright (c) Microsoft Corporation. All rights reserved.        */
/*                                                                        */
/*       This software is licensed under the Microsoft Software License   */
/*       Terms for Microsoft Azure RTOS. Full text of the license can be  */
/*       found in the LICENSE file at https://aka.ms/AzureRTOS_EULA       */
/*       and in the root directory of this software.                      */
/*                                                                        */
/**************************************************************************/

#if 1
#include <addons/azure_iot/nx_azure_iot_hub_client.h>
#include <addons/azure_iot/nx_azure_iot_json_reader.h>
#include <addons/azure_iot/nx_azure_iot_json_writer.h>
#include <addons/azure_iot/nx_azure_iot_provisioning_client.h>
#include <azure_sphere/nx_azure_iot_cert.h>
#include <azure_sphere/nx_azure_iot_ciphersuites.h>
#include <azure_sphere/sample_config.h>
#include <nx_api.h>
#include "azure/core/az_span.h"
#include "azure/core/az_version.h"
#include "tx_port.h"
#include "hx_drv_iomux.h"
#include "common.h"
#define SAMPLE_DHCP_DISABLE
#ifndef SAMPLE_DHCP_DISABLE
#include <nxd_dhcp_client.h>
#endif /* SAMPLE_DHCP_DISABLE */
#include <nxd_dns.h>
#include <nx_secure_tls_api.h>

#include <azure_sphere/sample_config.h>

#define NBIOT_SERVICE_ENABLE

#ifdef NBIOT_SERVICE_ENABLE
#include "wnb303r.h"

#include <stdio.h>
#include <string.h>
#include <time.h>

#define NETWORK_EN
/* Define AZ IoT Provisioning Client topic format.  */
#define NX_AZURE_IOT_PROVISIONING_CLIENT_REG_SUB_TOPIC                "$dps/registrations/res/#"
#define NX_AZURE_IOT_PROVISIONING_CLIENT_PAYLOAD_START                "{\"registrationId\" : \""
#define NX_AZURE_IOT_PROVISIONING_CLIENT_QUOTE                        "\""
#define NX_AZURE_IOT_PROVISIONING_CLIENT_CUSTOM_PAYLOAD                ", \"payload\" : "
#define NX_AZURE_IOT_PROVISIONING_CLIENT_PAYLOAD_END                  "}"
#define NX_AZURE_IOT_PROVISIONING_CLIENT_POLICY_NAME                  "registration"

/* Useragent e.g: DeviceClientType=c%2F1.0.0-preview.1%20%28nx%206.0%3Bazrtos%206.0%29 */
#define NX_AZURE_IOT_HUB_CLIENT_STR(C)          #C
#define NX_AZURE_IOT_HUB_CLIENT_TO_STR(x)       NX_AZURE_IOT_HUB_CLIENT_STR(x)
#define NX_AZURE_IOT_HUB_CLIENT_USER_AGENT      "DeviceClientType=c%2F" AZ_SDK_VERSION_STRING "%20%28nx%20" \
                                                NX_AZURE_IOT_HUB_CLIENT_TO_STR(NETXDUO_MAJOR_VERSION) "." \
                                                NX_AZURE_IOT_HUB_CLIENT_TO_STR(NETXDUO_MINOR_VERSION) "%3Bazrtos%20"\
                                                NX_AZURE_IOT_HUB_CLIENT_TO_STR(THREADX_MAJOR_VERSION) "." \
                                                NX_AZURE_IOT_HUB_CLIENT_TO_STR(THREADX_MINOR_VERSION) "%29"

#define AZURE_CONNECT_RETRY_MAX_TIMES	10
#define AZURE_ATCMD_RETRY_MAX_TIMES		10
//#define ALGO_EN
//static DEV_UART_PTR dev_uart_comm;
DEV_UART_PTR dev_uart_comm;

#endif

static SAMPLE_CONTEXT sample_context;

/* Define Azure RTOS TLS info.  */
static NX_SECURE_X509_CERT root_ca_cert;
static NX_AZURE_IOT_PROVISIONING_CLIENT dps_client;
static UCHAR nx_azure_iot_tls_metadata_buffer[NX_AZURE_IOT_TLS_METADATA_BUFFER_SIZE];
static NX_AZURE_IOT_HUB_CLIENT hub_client;
//static UCHAR buffer_ptr[1536];
static UCHAR *buffer_ptr;
static UINT buffer_size = 1536;
VOID *buffer_context;
INT nbiot_service_get_dps_key(ULONG expiry_time_secs, UCHAR *resource_dps_sas_token);
INT nbiot_service_get_iothub_key(ULONG expiry_time_secs, UCHAR *resource_dps_sas_token);

/* Define the prototypes for AZ IoT.  */
static NX_AZURE_IOT nx_azure_iot;

/* Include the sample.  */
//extern VOID sample_entry(NX_IP* ip_ptr, NX_PACKET_POOL* pool_ptr, NX_DNS* dns_ptr, UINT (*unix_time_callback)(ULONG *unix_time));
extern VOID sample_entry(NX_IP *ip_ptr, NX_PACKET_POOL *pool_ptr, NX_DNS *dns_ptr, UINT (*unix_time_callback)(ULONG *unix_time), SAMPLE_CONTEXT *sample_context);
int get_time_flag = 0;

/* Define the helper thread for running Azure SDK on ThreadX (THREADX IoT Platform).  */
#ifndef SAMPLE_HELPER_STACK_SIZE
#define SAMPLE_HELPER_STACK_SIZE        (4096)
#endif /* SAMPLE_HELPER_STACK_SIZE  */

#ifndef NBIOT_SERVICE_STACK_SIZE
#define NBIOT_SERVICE_STACK_SIZE        (4096)
#endif /* SAMPLE_HELPER_STACK_SIZE  */

#ifndef ALGO_SEND_RESULT_STACK_SIZE
#define ALGO_SEND_RESULT_STACK_SIZE     (4096)
#endif /* SAMPLE_HELPER_STACK_SIZE  */

#ifndef CIS_CAPTURE_IMAGE_STACK_SIZE
#define CIS_CAPTURE_IMAGE_STACK_SIZE     (4096)
#endif /* CIS_CAPTURE_IMAGE_STACK_SIZE  */

#ifndef SAMPLE_HELPER_THREAD_PRIORITY
#define SAMPLE_HELPER_THREAD_PRIORITY  		(4)
#endif /* SAMPLE_HELPER_THREAD_PRIORITY  */

#ifndef NBIOT_SERVICE_THREAD_PRIORITY
#define NBIOT_SERVICE_THREAD_PRIORITY   	(5)
#endif /* NBIOT_SERVICE_THREAD_PRIORITY  */

#ifndef ALGO_SEND_RESULT_THREAD_PRIORITY
#define ALGO_SEND_RESULT_THREAD_PRIORITY    (6)
#endif /* ALGO_SEND_RESULT_THREAD_PRIORITY  */

#ifndef CIS_CAPTURE_IMAGE_THREAD_PRIORITY
#define CIS_CAPTURE_IMAGE_THREAD_PRIORITY   (7)
#endif /* CIS_CAPTURE_IMAGE_THREAD_PRIORITY  */

/* Define user configurable symbols. */
#ifndef SAMPLE_IP_STACK_SIZE
#define SAMPLE_IP_STACK_SIZE            (2048)
#endif /* SAMPLE_IP_STACK_SIZE  */

#ifndef SAMPLE_PACKET_COUNT
#define SAMPLE_PACKET_COUNT             (32)
#endif /* SAMPLE_PACKET_COUNT  */

#ifndef SAMPLE_PACKET_SIZE
#define SAMPLE_PACKET_SIZE              (1536)
#endif /* SAMPLE_PACKET_SIZE  */

#define SAMPLE_POOL_SIZE                ((SAMPLE_PACKET_SIZE + sizeof(NX_PACKET)) * SAMPLE_PACKET_COUNT)

#ifndef SAMPLE_ARP_CACHE_SIZE
#define SAMPLE_ARP_CACHE_SIZE           (512)
#endif /* SAMPLE_ARP_CACHE_SIZE  */

#ifndef SAMPLE_IP_THREAD_PRIORITY
#define SAMPLE_IP_THREAD_PRIORITY       (1)
#endif /* SAMPLE_IP_THREAD_PRIORITY */

#ifdef SAMPLE_DHCP_DISABLE
#ifndef SAMPLE_IPV4_ADDRESS
#define SAMPLE_IPV4_ADDRESS           IP_ADDRESS(192, 168, 100, 33)
//#error "SYMBOL SAMPLE_IPV4_ADDRESS must be defined. This symbol specifies the IP address of device. "
#endif /* SAMPLE_IPV4_ADDRESS */

#ifndef SAMPLE_IPV4_MASK
#define SAMPLE_IPV4_MASK              0xFFFFFF00UL
//#error "SYMBOL SAMPLE_IPV4_MASK must be defined. This symbol specifies the IP address mask of device. "
#endif /* SAMPLE_IPV4_MASK */

#ifndef SAMPLE_GATEWAY_ADDRESS
#define SAMPLE_GATEWAY_ADDRESS        IP_ADDRESS(192, 168, 100, 1)
//#error "SYMBOL SAMPLE_GATEWAY_ADDRESS must be defined. This symbol specifies the gateway address for routing. "
#endif /* SAMPLE_GATEWAY_ADDRESS */

#ifndef SAMPLE_DNS_SERVER_ADDRESS
#define SAMPLE_DNS_SERVER_ADDRESS     IP_ADDRESS(192, 168, 100, 1)
//#error "SYMBOL SAMPLE_DNS_SERVER_ADDRESS must be defined. This symbol specifies the dns server address for routing. "
#endif /* SAMPLE_DNS_SERVER_ADDRESS */
#else
#define SAMPLE_IPV4_ADDRESS             IP_ADDRESS(192, 168, 52, 10)
#define SAMPLE_IPV4_MASK                IP_ADDRESS(255, 255, 255, 0)
#define SAMPLE_IPV4_GATEWAY             IP_ADDRESS(192, 168, 52, 254)
#endif /* SAMPLE_DHCP_DISABLE */


//0302static TX_THREAD        sample_helper_thread;
static TX_THREAD        nbiot_service_thread;
static TX_THREAD        algo_send_result_thread;
static TX_THREAD        cis_capture_image_thread;
static NX_PACKET_POOL   pool_0;
static NX_IP            ip_0;
static NX_DNS           dns_0;
#ifndef SAMPLE_DHCP_DISABLE
static NX_DHCP          dhcp_0;
#endif /* SAMPLE_DHCP_DISABLE  */


/* Define the stack/cache for ThreadX.  */
static ULONG sample_ip_stack[SAMPLE_IP_STACK_SIZE / sizeof(ULONG)];
#ifndef SAMPLE_POOL_STACK_USER
static ULONG sample_pool_stack[SAMPLE_POOL_SIZE / sizeof(ULONG)];
static ULONG sample_pool_stack_size = sizeof(sample_pool_stack);
#else
extern ULONG sample_pool_stack[];
extern ULONG sample_pool_stack_size;
#endif
//0302static ULONG sample_arp_cache_area[SAMPLE_ARP_CACHE_SIZE / sizeof(ULONG)];
//0302static ULONG sample_helper_thread_stack[SAMPLE_HELPER_STACK_SIZE / sizeof(ULONG)];
static ULONG nbiot_service_thread_stack[NBIOT_SERVICE_STACK_SIZE / sizeof(ULONG)];
static ULONG algo_send_result_thread_stack[ALGO_SEND_RESULT_STACK_SIZE / sizeof(ULONG)];
static ULONG cis_capture_image_thread_stack[CIS_CAPTURE_IMAGE_STACK_SIZE / sizeof(ULONG)];

/* Define the prototypes for sample thread.  */
static void sample_helper_thread_entry(ULONG parameter);

/* Define the prototypes for nbiot service thread.  */
static void nbiot_service_thread_entry(ULONG parameter);

/* Define the prototypes for algo send result thread.  */
static void algo_send_result_thread_entry(ULONG parameter);

/* Define the prototypes for cis capture image thread.  */
static void cis_capture_image_thread_entry(ULONG parameter);

/* Azure IoT DPS. */
AT_STRING azure_iotdps_connect_user_name;
//AT_STRING azure_iotdps_connect_password                  = "\"SharedAccessSignature sr=0ne00214C2E%2Fregistrations%2Fweiplus01&sig=pKrcx4RAvJA8TUL2GeVKElCggVfCq0jVwwy4emOAeZU%3D&se=12965819185&skn=registration\"";
/* azure centrol iotdps pw
char azure_iotdps_connect_password[]= "SharedAccessSignature sr=0ne00214C2E%2Fregistrations%2Fweiplus01&sig=pKrcx4RAvJA8TUL2GeVKElCggVfCq0jVwwy4emOAeZU%3D&se=12965819185&skn=registration";
*/
/* azure portal iotdps pw */
//char azure_iotdps_connect_password[]="SharedAccessSignature sr=0ne001D36F3%2Fregistrations%2Fweiplus&sig=Q5utiUqBAUdqFZt8zuF16a25pSVTDABEDwJjPAcJXIU%3D&se=12963587115&skn=enrolltest";
char azure_iotdps_connect_password[512];

AT_STRING azure_iotdps_get_registrations_publish_topic   = AZURE_IOTDPS_GET_CLIENT_REGISTER_STATUS_PUBLISH_TOPIC;//"$dps/registrations/GET/iotdps-get-operationstatus/?$rid=1&operationId=";

char azure_iotdps_registrations_msg[256];
char azure_iotdps_get_registrations_msg[256];
char azure_registrations_msg_len[2];


/* Azure Central IoT HUB. */
char azure_iothub_connect_host_name[128];

/* Azure Portal IoT HUB.
char azure_iothub_connect_host_name[]= "WeiPlusIoTHub.azure-devices.net";
*/
//0312- AT_STRING azure_iothub_connect_user_name[384];
char azure_iothub_connect_user_name[384];
char  azure_iothub_connect_password[512];
//char  azure_iothub_connect_password[]= "SharedAccessSignature sr=WeiPlusIoTHub.azure-devices.net%2Fdevices%2Fweiplus01&sig=DPJmQsNG%2BmAWFLyAaCCjp4W15VqNh8JZvn5Qlywa09U%3D&se=1615344534";
AT_STRING azure_iothub_publish_topic;
char azure_iothub_publish_msg[512];
char azure_iothub_publish_msg_len[2];
char azure_iothub_publish_human_num[2];

/* azure iotdps registration info. */
char azure_iotdps_reg_time_hex[832];
char azure_iotdps_reg_time_ascii[512];
char azure_iotdps_reg_assignhub_ascii[512];

/* Azure PNP DPS¡@Event initial. */
uint8_t azure_pnp_iotdps_event;

/* Azure PNP IoTHub¡@Event initial. */
uint8_t azure_pnp_iothub_event;

/* Azure Algorithm¡@Event initial. */
uint8_t azure_algo_event;

char recv_buf[AT_MAX_LEN];
char azure_iotdps_reg_opid[256];
uint32_t recv_len = AT_MAX_LEN;

/*#################Algorithm Info#################*/
/* define image size*/
#define IMAGE_SIZE 			30*1024 //unit:BYTE
#define SEND_PKG_MAX_SIZE 	256		//unit:BYTE
#define SEND_PKG_TIMES		IMAGE_SIZE/SEND_PKG_MAX_SIZE

/* detected human number. */
uint8_t algo_human_cnt = 3;

/* image buffer. */
char g_wdma3_img_buf[SEND_PKG_MAX_SIZE];
/*#################Algorithm Info#################*/

char azure_iothub_publish_msg_json[64];
char azure_iothub_publish_msg_json_len[2];

static char azure_iothub_publish_msg_ascii[SEND_PKG_MAX_SIZE];
static int  azure_iothub_img_send_cnt = 0;
static int  azure_iothub_img_send_idx = 0;

#if 0
uint32_t azure_iotdps_reg_year,azure_iotdps_reg_month,azure_iotdps_reg_day;
uint32_t azure_iotdps_reg_hour,azure_iotdps_reg_minute,azure_iotdps_reg_sec;
#else
struct tm azure_iotdps_network_tm;
time_t azure_iotdps_network_epoch_time;

struct tm azure_iotdps_reg_tm;
time_t azure_iotdps_epoch_time;
#endif
int hex_to_ascii(char c, char d);

#ifndef SAMPLE_DHCP_DISABLE
static void dhcp_wait();
#endif /* SAMPLE_DHCP_DISABLE */

static UINT dns_create();

static UINT unix_time_get(ULONG *unix_time);

/* Include the platform IP driver. */
void _nx_ram_network_driver(struct NX_IP_DRIVER_STRUCT *driver_req);

/* Parsing String_Hex to String_ASCII. */
int hex_to_int(char c){
     int first = c/16 - 3;
     int second = c % 16;
     int result = first*10 + second;
     if(result > 9) result--;
     return result;
}

int hex_to_ascii(char c, char d){
     int high = hex_to_int(c) * 16;
     int low = hex_to_int(d);
     return high+low;
}


/* Get network time for generate DPS SAS¡@KEY*/
static uint8_t azure_iotdps_get_network_time()
{
	char azure_iotdps_netwoerk_time[48];
	char *azure_iotdps_get_network_time_loc = NULL ;

	const UINT azure_iotdps_parsing_network_time_len = 24;
	uint32_t azure_iotdps_network_year,azure_iotdps_network_month,azure_iotdps_network_day;
	uint32_t azure_iotdps_network_hour,azure_iotdps_network_minute,azure_iotdps_network_sec;
	uint32_t azure_iotdps_network_time_zone;

	ULONG current_time;
	UCHAR *resource_dps_sas_token = NULL;
	get_time_flag = 0;

	/*parsing registered time*/
	/* 2021/02/22,11:17:44GMT+8 */
	azure_iotdps_get_network_time_loc = strstr(recv_buf, ":");
	strncpy(azure_iotdps_netwoerk_time, (recv_buf+(azure_iotdps_get_network_time_loc - recv_buf+1)),(strlen(recv_buf) - (azure_iotdps_get_network_time_loc - recv_buf)));
	memset(recv_buf, 0,AT_MAX_LEN);//clear buffer
//	xprintf("### azure_iotdps_netwoerk_time:%s ###\n",azure_iotdps_netwoerk_time);

//		/* eX: 2021/02/22,11:17:44GMT+8 */
		/* parsing network time Year*/
		azure_iotdps_network_year = atoi(azure_iotdps_netwoerk_time);
//		xprintf("### azure_iotdps_netwoerk_year:%d ###\n", azure_iotdps_network_year);
		azure_iotdps_network_tm.tm_year = (atoi(azure_iotdps_netwoerk_time) - 1900);
		//xprintf("*** azure_iotdps_reg_tm.tm_year:%d\n", (azure_iotdps_reg_tm.tm_year + 1900));

		/* parsing network time Month*/
		azure_iotdps_get_network_time_loc = strstr(azure_iotdps_netwoerk_time, "/");
		strncpy(azure_iotdps_netwoerk_time, (azure_iotdps_netwoerk_time+(azure_iotdps_get_network_time_loc - azure_iotdps_netwoerk_time+1)), azure_iotdps_parsing_network_time_len- (azure_iotdps_get_network_time_loc - azure_iotdps_netwoerk_time));
//		azure_iotdps_network_month = atoi(azure_iotdps_netwoerk_time);
//		xprintf("### azure_iotdps_netwoerk_month:%d ###\n", azure_iotdps_network_month);
		azure_iotdps_network_tm.tm_mon = (atoi(azure_iotdps_netwoerk_time) - 1);
		//xprintf("*** azure_iotdps_reg_tm.tm_mon:%d ***\n", (azure_iotdps_reg_tm.tm_mon + 1));

		/* parsing network time Day*/
		azure_iotdps_get_network_time_loc = strstr(azure_iotdps_netwoerk_time, "/");
		strncpy(azure_iotdps_netwoerk_time, (azure_iotdps_netwoerk_time+(azure_iotdps_get_network_time_loc - azure_iotdps_netwoerk_time+1)), azure_iotdps_parsing_network_time_len- (azure_iotdps_get_network_time_loc - azure_iotdps_netwoerk_time));
//		azure_iotdps_network_day = atoi(azure_iotdps_netwoerk_time);
//		xprintf("### azure_iotdps_netwoerk_day:%d ###\n", azure_iotdps_network_day);
		azure_iotdps_network_tm.tm_mday = atoi(azure_iotdps_netwoerk_time);
		//xprintf("*** azure_iotdps_reg_tm.tm_mday:%d ***\n", azure_iotdps_reg_tm.tm_mday);

		/* parsing network time Hour*/
		azure_iotdps_get_network_time_loc = strstr(azure_iotdps_netwoerk_time, ",");
		strncpy(azure_iotdps_netwoerk_time, (azure_iotdps_netwoerk_time+(azure_iotdps_get_network_time_loc - azure_iotdps_netwoerk_time+1)), azure_iotdps_parsing_network_time_len- (azure_iotdps_get_network_time_loc - azure_iotdps_netwoerk_time));
//		azure_iotdps_network_hour = atoi(azure_iotdps_netwoerk_time);
//		xprintf("### azure_iotdps_network_hour:%d ###\n", azure_iotdps_network_hour);
		azure_iotdps_network_tm.tm_hour = (atoi(azure_iotdps_netwoerk_time) - 1);
		//xprintf("*** azure_iotdps_reg_tm.tm_hour:%d ***\n",azure_iotdps_reg_tm.tm_hour + 1);

		/* parsing network time Minute*/
		azure_iotdps_get_network_time_loc = strstr(azure_iotdps_netwoerk_time, ":");
		strncpy(azure_iotdps_netwoerk_time, (azure_iotdps_netwoerk_time+(azure_iotdps_get_network_time_loc - azure_iotdps_netwoerk_time+1)), azure_iotdps_parsing_network_time_len- (azure_iotdps_get_network_time_loc - azure_iotdps_netwoerk_time));
//		azure_iotdps_network_minute = atoi(azure_iotdps_netwoerk_time);
//		xprintf("### azure_iotdps_network_minute:%d ###\n", azure_iotdps_network_minute);
		azure_iotdps_network_tm.tm_min = (atoi(azure_iotdps_netwoerk_time) - 1);
		//xprintf("*** azure_iotdps_reg_tm.tm_hour:%d ***\n",azure_iotdps_reg_tm.tm_hour + 1);

		/* parsing network time Second*/
		azure_iotdps_get_network_time_loc = strstr(azure_iotdps_netwoerk_time, ":");
		strncpy(azure_iotdps_netwoerk_time, (azure_iotdps_netwoerk_time+(azure_iotdps_get_network_time_loc - azure_iotdps_netwoerk_time+1)), azure_iotdps_parsing_network_time_len- (azure_iotdps_get_network_time_loc - azure_iotdps_netwoerk_time));
//		azure_iotdps_network_sec = atoi(azure_iotdps_netwoerk_time);
//		xprintf("### azure_iotdps_network_sec:%d ###\n", azure_iotdps_network_sec);
		azure_iotdps_network_tm.tm_sec = (atoi(azure_iotdps_netwoerk_time) - 1);
		//xprintf("*** azure_iotdps_reg_tm.tm_hour:%d ***\n",azure_iotdps_reg_tm.tm_hour + 1);

		azure_iotdps_network_epoch_time = mktime(&azure_iotdps_network_tm);
		get_time_flag = 1;
		unix_time_get(&current_time);
		//azure_iotdps_network_epoch_time = azure_iotdps_network_epoch_time-28800;//-3600*8
		//xprintf(" azure_iotdps_network_epoch_time:%ld \n", azure_iotdps_network_epoch_time);
		//xprintf("*** current_time:%ld ***\n", current_time);
		nbiot_service_get_dps_key(azure_iotdps_network_epoch_time, resource_dps_sas_token);

		return 1;
}

/* Send Algorithm result to Cloud*/
uint8_t send_algo_result_to_cloud(char *algo_res)
{
#if 1
	/* follow json format.*/
	/* json format eX: {"human":3} */
	char azure_iothub_publish_msg_json_ascii[256];
	sprintf(azure_iothub_publish_human_num, "%d", *algo_res);
	//printf("**** azure_iothub_publish_human_num:%s ****\n",azure_iothub_publish_human_num);
	/*  {"human": */
	strcpy(azure_iothub_publish_msg_json,AZURE_IOTHUB_PUBLISH_MESSAGE_JSON_PREFIX);

	/* {"human":3 */
	strcat(azure_iothub_publish_msg_json, azure_iothub_publish_human_num);

	/* {"human":3 }*/
	strcpy(azure_iothub_publish_msg_json_ascii,azure_iothub_publish_msg_json);
	strcat(azure_iothub_publish_msg_json_ascii, AZURE_IOTHUB_PUBLISH_MESSAGE_JSON_SUFFIX);

	//xprintf("**** azure_iothub_publish_msg_json_ascii:%s ****\n",azure_iothub_publish_msg_json_ascii);

	/* ASCII¡@convert to Hex */
	for (int j = 0; j < strlen(azure_iothub_publish_msg_json_ascii); j++){
		sprintf((azure_iothub_publish_msg_json + (j * 2)), "%02X", *(azure_iothub_publish_msg_json_ascii + j));
	}
	sprintf(azure_iothub_publish_msg_json_len, "%d", strlen(azure_iothub_publish_msg_json));
	//xprintf("**** azure_iothub_publish_msg_json_hex:%s, %s ****\n",azure_iothub_publish_msg_json_len,azure_iothub_publish_msg_json);


	/* send human number to azure cloud. */
	tx_thread_sleep(1);
	if(0 >wnb303r_MQTT_send_publish_packet(dev_uart_comm, "0", azure_iothub_publish_topic, "1", "0", "0", \
			azure_iothub_publish_msg_json_len,azure_iothub_publish_msg_json))
	{
		xprintf("aglo result at send fail.\n");
	}
#else
	/* for Test. note:only send human number, no follow json format*/
	xprintf("**** Detected human number:%d ****\n",*algo_res);
	/* ascii¡@convert to hex string. */
	sprintf(azure_iothub_publish_msg, "%02X", *algo_res);
	sprintf(azure_iothub_publish_msg_len, "%d", strlen(azure_iothub_publish_msg));

	/* send human number to azure cloud. */
	if(0 >wnb303r_MQTT_send_publish_packet(dev_uart_comm, "0", azure_iothub_publish_topic, "1", "0", "0", \
			azure_iothub_publish_msg_len,azure_iothub_publish_msg))
	{
		xprintf("aglo result at send fail.\n");
	}
#endif
	return 1;
}
/* Send image to cloud. */
uint8_t send_algo_img_to_cloud(char *imgbuf)
//uint8_t send_img_to_cloud()
{
	//img_send_idx = send_cnt * SEND_PKG_MAX_SIZE;
	//memcpy(azure_iothub_publish_msg_ascii,imgbuf,sizeof(azure_iothub_publish_msg_ascii)); //(g_wdma3_img_buf+img_send_idx)
	memcpy(azure_iothub_publish_msg_ascii,imgbuf,sizeof(azure_iothub_publish_msg_ascii));
	/* ascii¡@convert to hex string. */
	for (int j = 0; j < strlen(azure_iothub_publish_msg_ascii); j++){
		sprintf((azure_iothub_publish_msg + (j * 2)), "%02X", *(azure_iothub_publish_msg_ascii + j));
	}

	sprintf(azure_iothub_publish_msg_len, "%d", strlen(azure_iothub_publish_msg));

	/* send pkg to azure cloud. */
	if(0 >wnb303r_MQTT_send_publish_packet(dev_uart_comm, "0", azure_iothub_publish_topic, "1", "0", "0", \
			azure_iothub_publish_msg_len,azure_iothub_publish_msg))
	{
		xprintf("image data at send fail.\n");
	}
	return 1;
}

/* Parsing Azure IoT DPS Registration Operation ID. */
static uint8_t azure_pnp_iotdps_get_registration_operation_id()
{
	char azure_iotdps_reg_opid_hex[512];
	char azure_iotdps_opid_ascii[256];
	char azure_iotdps_reg_opid[256];
	char hex_msb = 0 ;
	char *azure_iotdps_reg_opid_loc;

	UINT azure_iotdps_parsing_reg_opid = 0;
	UINT azure_iotdps_reg_opid_hex_len;
	const UINT azure_iotdps_reg_opid_len = 55;

	sprintf(azure_registrations_msg_len, "%d", strlen(azure_iotdps_registrations_msg));

	//xprintf("1****azure_pnp_iotdps_get_registration_operation_id  recv_buf=%d %s ****1\r\n",strlen(recv_buf),recv_buf);

	if (strstr(recv_buf, "+EMQPUB:")!= NULL){
		azure_iotdps_reg_opid_loc = strstr(recv_buf, "+EMQPUB:");
		strncpy(azure_iotdps_reg_opid_hex, (recv_buf+(azure_iotdps_reg_opid_loc - recv_buf+1)),(strlen(recv_buf) - (azure_iotdps_reg_opid_loc - recv_buf)));
		//xprintf("2**** azure_iotdps_reg_opid_hex:%d,%s*****2\n",strlen(azure_iotdps_reg_opid_hex),azure_iotdps_reg_opid_hex);
		memset(recv_buf, 0,AT_MAX_LEN);//clear buffer

		while(1)
		{
			if(strstr(azure_iotdps_reg_opid_hex, ",") !=NULL )
			{
				azure_iotdps_parsing_reg_opid++;
				azure_iotdps_reg_opid_loc = strstr(azure_iotdps_reg_opid_hex, ",");
				strncpy(azure_iotdps_reg_opid_hex, (azure_iotdps_reg_opid_hex+(azure_iotdps_reg_opid_loc - azure_iotdps_reg_opid_hex+1)),(strlen(azure_iotdps_reg_opid_hex) - (azure_iotdps_reg_opid_loc - azure_iotdps_reg_opid_hex)));
			}

			if(azure_iotdps_parsing_reg_opid == 6)
				break;
		}//while

		azure_iotdps_reg_opid_hex_len = strlen(azure_iotdps_reg_opid_hex);
		//xprintf("3**** azure_iotdps_reg_opid_hex=%d, %s ****3\r\n",azure_iotdps_reg_opid_hex_len,azure_iotdps_reg_opid_hex);
		UINT cnt = 0;
		for(int i = 0; i < azure_iotdps_reg_opid_hex_len; i++)
		{
	       if(i % 2 != 0){
	    	   azure_iotdps_opid_ascii[cnt] = hex_to_ascii(hex_msb, azure_iotdps_reg_opid_hex[i]);
	    	   //xprintf("%c",azure_iotdps_opid_ascii[cnt]);
	    	   cnt++;
	       }else{
	    	   hex_msb = azure_iotdps_reg_opid_hex[i];
	       }
	    }//for
		//xprintf("\n");


		/* str_operationId = {"operationId":"4.17252aac68733575.931568c7-69ab-4d50-a1a7-ee65b024f3f9","status":"assigning"} */
		//0204 - azure_iotdps_reg_opid_loc = strstr(azure_iotdps_opid_ascii, ":"); //14
		azure_iotdps_reg_opid_loc = strstr(azure_iotdps_opid_ascii, "operationId");
		memset(azure_iotdps_reg_opid, 0, sizeof(azure_iotdps_reg_opid));
		//0204- strncpy(azure_iotdps_reg_opid, (azure_iotdps_opid_ascii+(azure_iotdps_reg_opid_loc - azure_iotdps_opid_ascii+2)),55);
		strncpy(azure_iotdps_reg_opid, (azure_iotdps_opid_ascii+(azure_iotdps_reg_opid_loc - azure_iotdps_opid_ascii+14)),azure_iotdps_reg_opid_len);
		azure_iotdps_reg_opid[55]='\0';
		//xprintf("4*****azure_iotdps_reg_opid:%d, %s *****4\n",strlen(azure_iotdps_reg_opid),azure_iotdps_reg_opid);

		/* get registration publish topic. */
		strncat(azure_iotdps_get_registrations_publish_topic, azure_iotdps_reg_opid, strlen(azure_iotdps_reg_opid));
		//xprintf("\n1.#### azure_iotdps_get_registrations_publish_topic: %s ####1.\n",azure_iotdps_get_registrations_publish_topic);

		/* $dps/registrations/GET/iotdps-get-operationstatus/?$rid=1&operationId=4.17252aac68733575.931568c7-69ab-4d50-a1a7-ee65b024f3f9 */
		/* azure_iotdps_get_registrations_publish_topic to hex for get registration publish message. */
		//for (int j = 0; j < strlen(azure_iotdps_reg_opid); j++){
		for (int j = 0; j < strlen(azure_iotdps_get_registrations_publish_topic); j++){
			sprintf((azure_iotdps_get_registrations_msg + (j * 2)), "%02X", *(azure_iotdps_get_registrations_publish_topic + j));
		 }

		 /* assign azure_iotdps_get_registrations_msg length. */
		 sprintf(azure_registrations_msg_len, "%d", strlen(azure_iotdps_get_registrations_msg));
		 //xprintf("\n2.#### azure_iotdps_get_registrations_msg: %s ####2.\n",azure_iotdps_get_registrations_msg);

		 return 1;
	}else{
		//waiting read nbiot reply data...
		return 0;
	}
}

/* Parsing Azure IoT DPS Registration Status. */
static uint8_t azure_pnp_iotdps_get_registration_status()
{
	UINT azure_iotdps_parsing_reg_time = 0;
	char hex_msb = 0 ;
	const UINT azure_iotdps_parsing_reg_time_len = 19;

//	char azure_iotdps_reg_time_hex[832];
//	char azure_iotdps_reg_time_ascii[512];
//	char azure_iotdps_reg_assignhub_ascii[512];
//	char *azure_iotdps_reg_time_hex;
//	char *azure_iotdps_reg_time_ascii;
//	char *azure_iotdps_reg_assignhub_ascii;
	char azure_iotdps_reg_time_tmp[19];
	char azure_iotdps_reg_assignedhub_tmp[128];

	char *azure_iotdps_reg_create_time_loc = NULL ;
	char *azure_iotdps_reg_create_assignedhub_loc = NULL;
	char *azure_iotdps_reg_create_deviceid_loc = NULL;

//	azure_iotdps_reg_time_hex = malloc(832);
//	azure_iotdps_reg_time_ascii = malloc(512);
//	azure_iotdps_reg_assignhub_ascii = malloc(512);
	//xprintf("\n3.###azure_iotdps_get_registrations_publish_topic:%s ###3\n",azure_iotdps_get_registrations_publish_topic);
	//xprintf("\n4.###azure_iotdps_get_registrations_msg:%s ###4\n",azure_iotdps_get_registrations_msg);
	UINT azure_iotdps_reg_time_hex_len;
  	//for sas token
	ULONG current_time;
	UCHAR *resource_dps_sas_token = NULL;
	get_time_flag = 0;

	tx_thread_sleep(1);
	//board_delay_ms(100);//100
	if(0 > wnb303r_MQTT_send_publish_packet(dev_uart_comm, "0", azure_iotdps_get_registrations_publish_topic, "1", "0", "0", \
				azure_registrations_msg_len,azure_iotdps_get_registrations_msg))
	{
		xprintf(" Azure IoT DPS publish get registration status packet Send Fail.\n");
	}

	/*parsing registered time*/
	if (strstr(recv_buf, "+EMQPUB:")!= NULL){
		azure_iotdps_reg_create_time_loc = strstr(recv_buf, "+EMQPUB:");
		strncpy(azure_iotdps_reg_time_hex, (recv_buf+(azure_iotdps_reg_create_time_loc - recv_buf+1)),(strlen(recv_buf) - (azure_iotdps_reg_create_time_loc - recv_buf)));
		//xprintf("\n2**** azure_iotdps_reg_time_hex:%d,%s ****2\n",strlen(azure_iotdps_reg_time_hex),azure_iotdps_reg_time_hex);
		memset(recv_buf, 0,AT_MAX_LEN);//clear buffer

		while(1)
		{
			if(strstr(azure_iotdps_reg_time_hex, ",") !=NULL )
			{
				azure_iotdps_parsing_reg_time++;
				azure_iotdps_reg_create_time_loc = strstr(azure_iotdps_reg_time_hex, ",");
				strncpy(azure_iotdps_reg_time_hex, (azure_iotdps_reg_time_hex+(azure_iotdps_reg_create_time_loc - azure_iotdps_reg_time_hex+1)),(strlen(azure_iotdps_reg_time_hex) - (azure_iotdps_reg_create_time_loc - azure_iotdps_reg_time_hex)));
			}
			if(azure_iotdps_parsing_reg_time == 6)
				break;
		}//while

		azure_iotdps_reg_time_hex_len = strlen(azure_iotdps_reg_time_hex);
		//xprintf("\n3**** azure_iotdps_reg_time_hex=%d, %s ****3\n",azure_iotdps_reg_time_hex_len,azure_iotdps_reg_time_hex);
		int cnt = 0;
		for(int i = 0; i < azure_iotdps_reg_time_hex_len; i++)
		{
	       if(i % 2 != 0){
	    	   azure_iotdps_reg_time_ascii[cnt] = hex_to_ascii(hex_msb, azure_iotdps_reg_time_hex[i]);
	    	   //azure_iotdps_reg_assignhub_ascii[cnt] = hex_to_ascii(hex_msb, azure_iotdps_reg_time_hex[i]);
	    	   //xprintf("%c",azure_iotdps_reg_time_ascii[cnt]);
	    	   cnt++;
	       }else{
	    	   hex_msb = azure_iotdps_reg_time_hex[i];
	       }
	    }//for
		//xprintf("\n");
		strcpy(azure_iotdps_reg_assignhub_ascii,azure_iotdps_reg_time_ascii);

		azure_iotdps_reg_create_time_loc = strstr(azure_iotdps_reg_time_ascii, "createdDateTimeUtc");
		memset(azure_iotdps_reg_time_tmp, 0, sizeof(azure_iotdps_reg_time_tmp));
		strncpy(azure_iotdps_reg_time_tmp, (azure_iotdps_reg_time_ascii+(azure_iotdps_reg_create_time_loc - azure_iotdps_reg_time_ascii+21)),azure_iotdps_parsing_reg_time_len);
		azure_iotdps_reg_time_tmp[19]='\0';
		//xprintf("\n4**** azure_iotdps_reg_time_tmp=%d, %s ****4\n",strlen(azure_iotdps_reg_time_tmp),azure_iotdps_reg_time_tmp);

		/* eX: 2021-01-30T07:55:20 */
		/* parsing registered time Year*/
		//azure_iotdps_reg_year = atoi(azure_iotdps_reg_time_tmp);
		azure_iotdps_reg_tm.tm_year = (atoi(azure_iotdps_reg_time_tmp) - 1900);
		//xprintf("*** azure_iotdps_reg_tm.tm_year:%d\n", (azure_iotdps_reg_tm.tm_year + 1900));

		/* parsing registered time Month*/
		azure_iotdps_reg_create_time_loc = strstr(azure_iotdps_reg_time_tmp, "-");
		strncpy(azure_iotdps_reg_time_tmp, (azure_iotdps_reg_time_tmp+(azure_iotdps_reg_create_time_loc - azure_iotdps_reg_time_tmp+1)), azure_iotdps_parsing_reg_time_len- (azure_iotdps_reg_create_time_loc - azure_iotdps_reg_time_tmp));
		//azure_iotdps_reg_month = atoi(azure_iotdps_reg_time_tmp);
		azure_iotdps_reg_tm.tm_mon = (atoi(azure_iotdps_reg_time_tmp) - 1);
		//xprintf("*** azure_iotdps_reg_tm.tm_mon:%d ***\n", (azure_iotdps_reg_tm.tm_mon + 1));

		/* parsing registered time Day*/
		azure_iotdps_reg_create_time_loc = strstr(azure_iotdps_reg_time_tmp, "-");
		strncpy(azure_iotdps_reg_time_tmp, (azure_iotdps_reg_time_tmp+(azure_iotdps_reg_create_time_loc - azure_iotdps_reg_time_tmp+1)), azure_iotdps_parsing_reg_time_len- (azure_iotdps_reg_create_time_loc - azure_iotdps_reg_time_tmp));
		//azure_iotdps_reg_day = atoi(azure_iotdps_reg_time_tmp);
		azure_iotdps_reg_tm.tm_mday = atoi(azure_iotdps_reg_time_tmp);
		//xprintf("*** azure_iotdps_reg_tm.tm_mday:%d ***\n", azure_iotdps_reg_tm.tm_mday);

		/* parsing registered time Hour*/
		azure_iotdps_reg_create_time_loc = strstr(azure_iotdps_reg_time_tmp, "T");
		strncpy(azure_iotdps_reg_time_tmp, (azure_iotdps_reg_time_tmp+(azure_iotdps_reg_create_time_loc - azure_iotdps_reg_time_tmp+1)), azure_iotdps_parsing_reg_time_len- (azure_iotdps_reg_create_time_loc - azure_iotdps_reg_time_tmp));
		//azure_iotdps_reg_hour = atoi(azure_iotdps_reg_time_tmp);
		azure_iotdps_reg_tm.tm_hour = (atoi(azure_iotdps_reg_time_tmp) - 1);
		//xprintf("*** azure_iotdps_reg_tm.tm_hour:%d ***\n",azure_iotdps_reg_tm.tm_hour + 1);

		/* parsing registered time Minute*/
		azure_iotdps_reg_create_time_loc = strstr(azure_iotdps_reg_time_tmp, ":");
		strncpy(azure_iotdps_reg_time_tmp, (azure_iotdps_reg_time_tmp+(azure_iotdps_reg_create_time_loc - azure_iotdps_reg_time_tmp+1)), azure_iotdps_parsing_reg_time_len- (azure_iotdps_reg_create_time_loc - azure_iotdps_reg_time_tmp));
		//azure_iotdps_reg_minute = atoi(azure_iotdps_reg_time_tmp);
		azure_iotdps_reg_tm.tm_min = (atoi(azure_iotdps_reg_time_tmp) - 1);
		//xprintf("*** azure_iotdps_reg_tm.tm_min:%d ***\n", (azure_iotdps_reg_tm.tm_min + 1));

		/* parsing registered time Second*/
		azure_iotdps_reg_create_time_loc = strstr(azure_iotdps_reg_time_tmp, ":");
		strncpy(azure_iotdps_reg_time_tmp, (azure_iotdps_reg_time_tmp+(azure_iotdps_reg_create_time_loc - azure_iotdps_reg_time_tmp+1)), azure_iotdps_parsing_reg_time_len- (azure_iotdps_reg_create_time_loc - azure_iotdps_reg_time_tmp));
		//azure_iotdps_reg_sec = atoi(azure_iotdps_reg_time_tmp);
		azure_iotdps_reg_tm.tm_sec = (atoi(azure_iotdps_reg_time_tmp) - 1);
		//xprintf("*** azure_iotdps_reg_tm.tm_sec:%d ***\n", (azure_iotdps_reg_tm.tm_sec + 1));

		azure_iotdps_epoch_time = mktime(&azure_iotdps_reg_tm);
		get_time_flag = 1;
		unix_time_get(&current_time);
		//xprintf("*** azure_iotdps_epoch_time:%ld ***\n", azure_iotdps_epoch_time);
		//xprintf("*** current_time:%ld ***\n", current_time);

		/* Get assignedHub name. */
		//(azure_iodps_reg_create_assignedhub_loc - azure_iotdps_reg_time_ascii)+14
		//

		azure_iotdps_reg_create_assignedhub_loc = strstr(azure_iotdps_reg_assignhub_ascii, "assignedHub");
		azure_iotdps_reg_create_deviceid_loc = strstr(azure_iotdps_reg_assignhub_ascii, "deviceId");
		memset(azure_iotdps_reg_assignedhub_tmp, 0, sizeof(azure_iotdps_reg_assignedhub_tmp));
		//len = (azure_iodps_reg_create_deviceid_loc - azure_iodps_reg_create_assignedhub_loc) -16
		strncpy(azure_iotdps_reg_assignedhub_tmp, (azure_iotdps_reg_assignhub_ascii+(azure_iotdps_reg_create_assignedhub_loc - azure_iotdps_reg_assignhub_ascii+14)),((azure_iotdps_reg_assignhub_ascii+(azure_iotdps_reg_create_deviceid_loc - azure_iotdps_reg_assignhub_ascii)) - (azure_iotdps_reg_assignhub_ascii+(azure_iotdps_reg_create_assignedhub_loc - azure_iotdps_reg_assignhub_ascii)) )-17);
		azure_iotdps_reg_assignedhub_tmp[128]='\0';
		//xprintf("*** azure_iotdps_reg_assignedhub_tmp:%s ***\n", azure_iotdps_reg_assignedhub_tmp);

		strcpy(azure_iothub_connect_host_name,azure_iotdps_reg_assignedhub_tmp);
		xprintf("*** azure_iothub_connect_host_name:%s ***\n", azure_iothub_connect_host_name);

		nbiot_service_get_iothub_key(azure_iotdps_epoch_time, resource_dps_sas_token);

//		free(azure_iotdps_reg_time_hex);
//		free(azure_iotdps_reg_time_ascii);
//		free(azure_iotdps_reg_assignhub_ascii);
		return 1;
	}else{
		xprintf("\n waiting azure_pnp_iotdps_get_registration_status...\n\n");
		return 0;
	}
}

/* Define Azure RTOS TLS info.  */
static NX_SECURE_X509_CERT root_ca_cert;
static UCHAR nx_azure_iot_tls_metadata_buffer[NX_AZURE_IOT_TLS_METADATA_BUFFER_SIZE];
static ULONG nx_azure_iot_thread_stack[NX_AZURE_IOT_STACK_SIZE / sizeof(ULONG)];
/* Define what the initial system looks like.  */
void    tx_application_define(void *first_unused_memory)
{

UINT  status;


    NX_PARAMETER_NOT_USED(first_unused_memory);


    /* Driver Initial. */
    xprintf("wnb303r_initial!!!\n");
    dev_uart_comm = wnb303r_drv_init(DFSS_UART_0_ID, UART_BAUDRATE_115200);
    if(dev_uart_comm == NULL)
    {
    	xprintf("wnb303r_init fail\n");
    	return;
    }

//	//for test
//	for(int k = 0;k<SEND_PKG_MAX_SIZE;k++){
//		g_wdma3_img_buf[k]='A';
//	}

#ifdef ALGO_EN
     /* TBD. */
#endif

#ifdef NETWORK_EN
    /* Initialize the NetX system.  */
    nx_system_initialize();
#endif
    /* Create a packet pool.  */
    status = nx_packet_pool_create(&pool_0, "NetX Main Packet Pool", SAMPLE_PACKET_SIZE,
                                   (UCHAR *)sample_pool_stack , sample_pool_stack_size);
    xprintf("nx_packet_pool_create status: %u\r\n", status);
    /* Check for pool creation error.  */
    if (status)
    {
        xprintf("nx_packet_pool_create fail: %u\r\n", status);
        return;
    }

#ifdef NETWORK_EN
    /* Create an IP instance.  */
#if 1
    status = nx_ip_create(&ip_0, "NetX IP Instance 0",
                          SAMPLE_IPV4_ADDRESS, SAMPLE_IPV4_MASK,
                          &pool_0, NULL,
                          (UCHAR*)sample_ip_stack, sizeof(sample_ip_stack),
                          SAMPLE_IP_THREAD_PRIORITY);
#else
    status = nx_ip_create(&ip_0, "NetX IP Instance 0", 0, 0, &pool_0, NULL, NULL, 0, 0);
#endif
    //0303 xprintf("nx_ip_create: %u\r\n", status);
    //! owen add
    //nx_ip_gateway_address_set(&ip_0, SAMPLE_IPV4_GATEWAY);
    /* Check for IP create errors.  */
    if (status)
    {
        xprintf("nx_ip_create fail: %u\r\n", status);
        return;
    }
#endif

#if 0
    /* Enable ARP and supply ARP cache memory for IP Instance 0.  */
    status = nx_arp_enable(&ip_0, (VOID *)sample_arp_cache_area, sizeof(sample_arp_cache_area));
    xprintf("nx_arp_enable : %u\r\n", status);
    /* Check for ARP enable errors.  */
    if (status)
    {
        xprintf("nx_arp_enable fail: %u\r\n", status);
        return;
    }

    /* Enable ICMP traffic.  */
    status = nx_icmp_enable(&ip_0);
    xprintf("nx_icmp_enable: %u\r\n", status);
    /* Check for ICMP enable errors.  */
    if (status)
    {
        xprintf("nx_icmp_enable fail: %u\r\n", status);
        return;
    }

    /* Enable TCP traffic.  */
    status = nx_tcp_enable(&ip_0);
    xprintf("nx_tcp_enable: %u\r\n", status);
    /* Check for TCP enable errors.  */
    if (status)
    {
        xprintf("nx_tcp_enable fail: %u\r\n", status);
        return;
    }

    /* Enable UDP traffic.  */
    status = nx_udp_enable(&ip_0);
    xprintf("nx_udp_enable: %u\r\n", status);
    /* Check for UDP enable errors.  */
    if (status)
    {
        xprintf("nx_udp_enable fail: %u\r\n", status);
        return;
    }
#endif

#ifdef NETWORK_EN
    /* Initialize TLS.  */
    nx_secure_tls_initialize();
#endif

#if 1
    /* Create nbiot service thread. */
    status = tx_thread_create(&nbiot_service_thread, "nbiot service Thread",
    							nbiot_service_thread_entry, 0,
								nbiot_service_thread_stack, NBIOT_SERVICE_STACK_SIZE,
								NBIOT_SERVICE_THREAD_PRIORITY, NBIOT_SERVICE_THREAD_PRIORITY,
								TX_NO_TIME_SLICE, TX_AUTO_START);

    /* Check status.  */
    if (status)
    {
        xprintf("nbiot service thread creation fail: %u\r\n", status);
        return;
    }


    /* Create algo send result thread. */
    status = tx_thread_create(&algo_send_result_thread, "algo send result Thread",
    							algo_send_result_thread_entry, 0,
								algo_send_result_thread_stack, ALGO_SEND_RESULT_STACK_SIZE,
								ALGO_SEND_RESULT_THREAD_PRIORITY, ALGO_SEND_RESULT_THREAD_PRIORITY,
								TX_NO_TIME_SLICE, TX_AUTO_START);

    /* Check status.  */
    if (status)
    {
        xprintf("algo send result thread creation fail: %u\r\n", status);
        return;
    }

    /* Create cis capture image thread. */
    status = tx_thread_create(&cis_capture_image_thread, "cis capture image Thread",
    							cis_capture_image_thread_entry, 0,
								cis_capture_image_thread_stack, CIS_CAPTURE_IMAGE_STACK_SIZE,
								CIS_CAPTURE_IMAGE_THREAD_PRIORITY, CIS_CAPTURE_IMAGE_THREAD_PRIORITY,
								TX_NO_TIME_SLICE, TX_AUTO_START);

    /* Check status.  */
    if (status)
    {
        xprintf("CIS¡@capture image thread creation fail: %u\r\n", status);
        return;
    }

#if 0//def NETWORK_EN
    /* Create sample helper thread. */
    status = tx_thread_create(&sample_helper_thread, "Demo Thread",
                              sample_helper_thread_entry, 0,
                              sample_helper_thread_stack, SAMPLE_HELPER_STACK_SIZE,
                              SAMPLE_HELPER_THREAD_PRIORITY, SAMPLE_HELPER_THREAD_PRIORITY,
                              TX_NO_TIME_SLICE, TX_AUTO_START);
    xprintf("Demo helper thread creation : %u\r\n", status);
    /* Check status.  */
    if (status)
    {
        xprintf("Demo helper thread creation fail: %u\r\n", status);
        return;
    }
#endif
#else

#endif
}

/* Define sample helper thread entry.  */
void sample_helper_thread_entry(ULONG parameter)
{
#if 0
UINT    status;
ULONG   ip_address = 0;
ULONG   network_mask = 0;
ULONG   gateway_address = 0;


    NX_PARAMETER_NOT_USED(parameter);

#ifndef SAMPLE_DHCP_DISABLE
    dhcp_wait();
#else
    nx_ip_gateway_address_set(&ip_0, SAMPLE_GATEWAY_ADDRESS);
#endif /* SAMPLE_DHCP_DISABLE  */

    /* Get IP address and gateway address. */
    nx_ip_address_get(&ip_0, &ip_address, &network_mask);
    nx_ip_gateway_address_get(&ip_0, &gateway_address);

    /* Output IP address and gateway address. */
    xprintf("IP address: %lu.%lu.%lu.%lu\r\n",
           (ip_address >> 24),
           (ip_address >> 16 & 0xFF),
           (ip_address >> 8 & 0xFF),
           (ip_address & 0xFF));
    xprintf("Mask: %lu.%lu.%lu.%lu\r\n",
           (network_mask >> 24),
           (network_mask >> 16 & 0xFF),
           (network_mask >> 8 & 0xFF),
           (network_mask & 0xFF));
    xprintf("Gateway: %lu.%lu.%lu.%lu\r\n",
           (gateway_address >> 24),
           (gateway_address >> 16 & 0xFF),
           (gateway_address >> 8 & 0xFF),
           (gateway_address & 0xFF));

    /* Create DNS.  */
    status = dns_create();

    /* Check for DNS create errors.  */
    if (status)
    {
        xprintf("dns_create fail: %u\r\n", status);
        return;
    }
#endif
    /* Use time to init the seed. FIXME: use real rand on device.  */
    srand((unsigned int)time(NULL));

    /* Start sample.  */
    sample_entry(&ip_0, &pool_0, &dns_0, unix_time_get, &sample_context);
}

static VOID nbiot_service_provisioning_client_mqtt_receive_callback(NXD_MQTT_CLIENT *client_ptr,
                                                                   UINT number_of_messages)
{
NX_AZURE_IOT_RESOURCE *resource = nx_azure_iot_resource_search(client_ptr);
NX_AZURE_IOT_PROVISIONING_CLIENT *prov_client_ptr = NX_NULL;
NX_PACKET *packet_ptr;
NX_PACKET *packet_next_ptr;
UINT status;

    /* This function is protected by MQTT mutex.  */

    NX_PARAMETER_NOT_USED(number_of_messages);

    if (resource && (resource -> resource_type == NX_AZURE_IOT_RESOURCE_IOT_PROVISIONING))
    {
        prov_client_ptr = (NX_AZURE_IOT_PROVISIONING_CLIENT *)resource -> resource_data_ptr;
    }

    if (prov_client_ptr)
    {
        for (packet_ptr = client_ptr -> message_receive_queue_head;
            packet_ptr;
            packet_ptr = packet_next_ptr)
        {

            /* Store next packet in case current packet is consumed.  */
            packet_next_ptr = packet_ptr -> nx_packet_queue_next;

            /* Adjust packet to simply process logic.  */
            nx_azure_iot_mqtt_packet_adjust(packet_ptr);

            /* Last response was not yet consumed, probably duplicate from service.  */
            if (prov_client_ptr -> nx_azure_iot_provisioning_client_last_response)
            {
                nx_packet_release(packet_ptr);
                continue;
            }

            prov_client_ptr -> nx_azure_iot_provisioning_client_last_response = packet_ptr;
            status = nx_cloud_module_event_set(&(prov_client_ptr -> nx_azure_iot_ptr -> nx_azure_iot_cloud_module),
                                               NX_AZURE_IOT_PROVISIONING_CLIENT_RESPONSE_EVENT);
            if (status)
            {
                //nx_azure_iot_provisioning_client_update_state(prov_client_ptr, status);
            	prov_client_ptr -> nx_azure_iot_provisioning_client_state = 1;
            	prov_client_ptr -> nx_azure_iot_provisioning_client_result = status;

            }
        }

        /* Clear all message from MQTT receive queue.  */
        client_ptr -> message_receive_queue_head = NX_NULL;
        client_ptr -> message_receive_queue_tail = NX_NULL;
        client_ptr -> message_receive_queue_depth = 0;
    }
}

/* Define the prototypes for Azure RTOS IoT.  */
UINT nbiot_service_iot_provisioning_client_initialize(ULONG expiry_time_secs)
{
	UINT status;
	UINT mqtt_user_name_length;
	NXD_MQTT_CLIENT *mqtt_client_ptr;
	NX_AZURE_IOT_RESOURCE *resource_ptr;
	UCHAR *buffer_ptr;
	UINT buffer_size;
	VOID *buffer_context;
	az_span endpoint_span = az_span_create((UCHAR *)ENDPOINT, (INT)sizeof(ENDPOINT) - 1);
	az_span id_scope_span = az_span_create((UCHAR *)ID_SCOPE, (INT)sizeof(ID_SCOPE) - 1);
	az_span registration_id_span = az_span_create((UCHAR *)REGISTRATION_ID, (INT)sizeof(REGISTRATION_ID) - 1);

    memset(&dps_client, 0, sizeof(NX_AZURE_IOT_PROVISIONING_CLIENT));
    /* Set resource pointer.  */
    resource_ptr = &(dps_client.nx_azure_iot_provisioning_client_resource);
    mqtt_client_ptr = &(dps_client.nx_azure_iot_provisioning_client_resource.resource_mqtt);

    dps_client.nx_azure_iot_ptr = &nx_azure_iot;
    dps_client.nx_azure_iot_provisioning_client_endpoint = (UCHAR *)ENDPOINT;
    dps_client.nx_azure_iot_provisioning_client_endpoint_length = sizeof(ENDPOINT) - 1;
    dps_client.nx_azure_iot_provisioning_client_id_scope = (UCHAR *)ID_SCOPE;
    dps_client.nx_azure_iot_provisioning_client_id_scope_length = sizeof(ID_SCOPE) - 1;
    dps_client.nx_azure_iot_provisioning_client_registration_id = (UCHAR *)REGISTRATION_ID;
    dps_client.nx_azure_iot_provisioning_client_registration_id_length = sizeof(REGISTRATION_ID) - 1;
    dps_client.nx_azure_iot_provisioning_client_resource.resource_crypto_array = _nx_azure_iot_tls_supported_crypto;
    dps_client.nx_azure_iot_provisioning_client_resource.resource_crypto_array_size = _nx_azure_iot_tls_supported_crypto_size;
    dps_client.nx_azure_iot_provisioning_client_resource.resource_cipher_map = _nx_azure_iot_tls_ciphersuite_map;
    dps_client.nx_azure_iot_provisioning_client_resource.resource_cipher_map_size = _nx_azure_iot_tls_ciphersuite_map_size;
    dps_client.nx_azure_iot_provisioning_client_resource.resource_metadata_ptr = nx_azure_iot_tls_metadata_buffer;
    dps_client.nx_azure_iot_provisioning_client_resource.resource_metadata_size = sizeof(nx_azure_iot_tls_metadata_buffer);
#ifdef NX_SECURE_ENABLE /*YUN*/
    dps_client.nx_azure_iot_provisioning_client_resource.resource_trusted_certificate = &root_ca_cert;
#endif
    dps_client.nx_azure_iot_provisioning_client_resource.resource_hostname = (UCHAR *)ENDPOINT;
    dps_client.nx_azure_iot_provisioning_client_resource.resource_hostname_length = sizeof(ENDPOINT) - 1;
    resource_ptr->resource_mqtt_client_id_length = dps_client.nx_azure_iot_provisioning_client_registration_id_length;
    resource_ptr->resource_mqtt_client_id = (UCHAR *)dps_client.nx_azure_iot_provisioning_client_registration_id;

    //expiry_time_secs = (ULONG)time(NULL);//(ULONG)azure_iotdps_epoch_time;
    dps_client.nx_azure_iot_provisioning_client_symmetric_key = (UCHAR *)DEVICE_SYMMETRIC_KEY;
    dps_client.nx_azure_iot_provisioning_client_symmetric_key_length = sizeof(DEVICE_SYMMETRIC_KEY) - 1;
    expiry_time_secs += NX_AZURE_IOT_PROVISIONING_CLIENT_TOKEN_EXPIRY;
    xprintf("expiry_time_secs %ld\r\n", expiry_time_secs);

    if (az_result_failed(az_iot_provisioning_client_init(&(dps_client.nx_azure_iot_provisioning_client_core),
                                                         endpoint_span, id_scope_span,
                                                         registration_id_span, NULL)))
    {
    	xprintf("IoTProvisioning client initialize fail: failed to initialize core client\n");
        return(NX_AZURE_IOT_SDK_CORE_ERROR);
    }

    status = _nxd_mqtt_client_cloud_create(mqtt_client_ptr, (CHAR *)nx_azure_iot.nx_azure_iot_name,
                                           (CHAR *)resource_ptr -> resource_mqtt_client_id,
                                           resource_ptr -> resource_mqtt_client_id_length,
										   nx_azure_iot.nx_azure_iot_ip_ptr,
										   nx_azure_iot.nx_azure_iot_pool_ptr,
                                           &nx_azure_iot.nx_azure_iot_cloud);
    if (status)
    {
    	xprintf("IoTProvisioning initialize create fail: MQTT CLIENT CREATE FAIL status: %d\n", status);
        return(status);
    }

    status = nxd_mqtt_client_receive_notify_set(mqtt_client_ptr,
    											nbiot_service_provisioning_client_mqtt_receive_callback);
    if (status)
    {
    	xprintf("IoTProvisioning client set message callback status: %d\n", status);
        nxd_mqtt_client_delete(mqtt_client_ptr);
        return(status);
    }
    //buffer_size = 2048;
    status = nx_azure_iot_buffer_allocate(dps_client.nx_azure_iot_ptr,
                                          &buffer_ptr, &buffer_size, &buffer_context);
    xprintf("buffer_size %d\n", buffer_size);
    if (status)
    {
    	xprintf("IoTProvisioning client failed initialization: BUFFER ALLOCATE FAIL\n");
        return(status);
    }

    /* Build user name.  */
    if (az_result_failed(az_iot_provisioning_client_get_user_name(&(dps_client.nx_azure_iot_provisioning_client_core),
                                                                  (CHAR *)buffer_ptr, buffer_size, &mqtt_user_name_length)))
    {
    	xprintf("IoTProvisioning client connect fail: NX_AZURE_IOT_Provisioning_CLIENT_USERNAME_SIZE is too small.\n");
        return(NX_AZURE_IOT_INSUFFICIENT_BUFFER_SPACE);
    }
    xprintf("mqtt_user_name_length %d\n",mqtt_user_name_length);
    xprintf("buffer_ptr %s\n",buffer_ptr);
    /* Save the resource buffer.  */
    resource_ptr -> resource_mqtt_buffer_context = buffer_context;
    resource_ptr -> resource_mqtt_buffer_size = buffer_size;
    resource_ptr -> resource_mqtt_user_name_length = mqtt_user_name_length;
    resource_ptr -> resource_mqtt_user_name = buffer_ptr;
    resource_ptr -> resource_mqtt_sas_token = buffer_ptr + mqtt_user_name_length;
    dps_client.nx_azure_iot_provisioning_client_sas_token_buff_size = buffer_size - mqtt_user_name_length;

    /* Link the resource.  */
    dps_client.nx_azure_iot_provisioning_client_resource.resource_data_ptr = &dps_client;
    dps_client.nx_azure_iot_provisioning_client_resource.resource_type = NX_AZURE_IOT_RESOURCE_IOT_PROVISIONING;
    nx_azure_iot_resource_add(&nx_azure_iot, &(dps_client.nx_azure_iot_provisioning_client_resource));
    return(NX_AZURE_IOT_SUCCESS);
}

static UINT nbiot_service_iot_hub_client_process_publish_packet(UCHAR *start_ptr,
                                                           ULONG *topic_offset_ptr,
                                                           USHORT *topic_length_ptr)
{
UCHAR *byte = start_ptr;
UINT byte_count = 0;
UINT multiplier = 1;
UINT remaining_length = 0;
UINT topic_length;

    /* Validate packet start contains fixed header.  */
    do
    {
        if (byte_count >= 4)
        {
            LogError(LogLiteralArgs("Invalid mqtt packet start position"));
            return(NX_AZURE_IOT_INVALID_PACKET);
        }

        byte++;
        remaining_length += (((*byte) & 0x7F) * multiplier);
        multiplier = multiplier << 7;
        byte_count++;
    } while ((*byte) & 0x80);

    if (remaining_length < 2)
    {
        return(NX_AZURE_IOT_INVALID_PACKET);
    }

    /* Retrieve topic length.  */
    byte++;
    topic_length = (UINT)(*(byte) << 8) | (*(byte + 1));

    if (topic_length > remaining_length - 2u)
    {
        return(NX_AZURE_IOT_INVALID_PACKET);
    }

    *topic_offset_ptr = (ULONG)((byte + 2) - start_ptr);
    *topic_length_ptr = (USHORT)topic_length;

    /* Return.  */
    return(NX_AZURE_IOT_SUCCESS);
}

static VOID nbiot_service_iot_hub_client_mqtt_receive_callback(NXD_MQTT_CLIENT* client_ptr,
                                                          UINT number_of_messages)
{
NX_AZURE_IOT_RESOURCE *resource = nx_azure_iot_resource_search(client_ptr);
NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr = NX_NULL;
NX_PACKET *packet_ptr;
NX_PACKET *packet_next_ptr;
ULONG topic_offset;
USHORT topic_length;

    /* This function is protected by MQTT mutex.  */

    NX_PARAMETER_NOT_USED(number_of_messages);

    if (resource && (resource -> resource_type == NX_AZURE_IOT_RESOURCE_IOT_HUB))
    {
        hub_client_ptr = (NX_AZURE_IOT_HUB_CLIENT *)resource -> resource_data_ptr;
    }

    if (hub_client_ptr)
    {
        for (packet_ptr = client_ptr -> message_receive_queue_head;
             packet_ptr;
             packet_ptr = packet_next_ptr)
        {

            /* Store next packet in case current packet is consumed.  */
            packet_next_ptr = packet_ptr -> nx_packet_queue_next;

            /* Adjust packet to simply process logic.  */
            nx_azure_iot_mqtt_packet_adjust(packet_ptr);

            if (nbiot_service_iot_hub_client_process_publish_packet(packet_ptr -> nx_packet_prepend_ptr, &topic_offset,
                                                               &topic_length))
            {

                /* Message not supported. It will be released.  */
                nx_packet_release(packet_ptr);
                continue;
            }

            if ((topic_offset + topic_length) >
                (ULONG)(packet_ptr -> nx_packet_append_ptr - packet_ptr -> nx_packet_prepend_ptr))
            {

                /* Only process topic in the first packet since the fixed topic is short enough to fit into one packet.  */
                topic_length = (USHORT)(((ULONG)(packet_ptr -> nx_packet_append_ptr - packet_ptr -> nx_packet_prepend_ptr) -
                                         topic_offset) & 0xFFFF);
            }

            if (hub_client_ptr -> nx_azure_iot_hub_client_direct_method_message.message_process &&
                (hub_client_ptr -> nx_azure_iot_hub_client_direct_method_message.message_process(hub_client_ptr, packet_ptr,
                                                                                                 topic_offset,
                                                                                                 topic_length) == NX_AZURE_IOT_SUCCESS))
            {

                /* Direct method message is processed.  */
                continue;
            }

            if (hub_client_ptr -> nx_azure_iot_hub_client_c2d_message.message_process &&
                (hub_client_ptr -> nx_azure_iot_hub_client_c2d_message.message_process(hub_client_ptr, packet_ptr,
                                                                                       topic_offset,
                                                                                       topic_length) == NX_AZURE_IOT_SUCCESS))
            {

                /* Could to Device message is processed.  */
                continue;
            }

            if ((hub_client_ptr -> nx_azure_iot_hub_client_device_twin_message.message_process) &&
                (hub_client_ptr -> nx_azure_iot_hub_client_device_twin_message.message_process(hub_client_ptr,
                                                                                               packet_ptr, topic_offset,
                                                                                               topic_length) == NX_AZURE_IOT_SUCCESS))
            {

                /* Device Twin message is processed.  */
                continue;
            }

            /* Message not supported. It will be released.  */
            nx_packet_release(packet_ptr);
        }

        /* Clear all message from MQTT receive queue.  */
        client_ptr -> message_receive_queue_head = NX_NULL;
        client_ptr -> message_receive_queue_tail = NX_NULL;
        client_ptr -> message_receive_queue_depth = 0;
    }
}

UINT nbiot_service_iot_hub_client_initialize(NX_AZURE_IOT_HUB_CLIENT* hub_client_ptr,
                                        NX_AZURE_IOT *nx_azure_iot_ptr,
                                        const UCHAR *host_name, UINT host_name_length,
                                        const UCHAR *device_id, UINT device_id_length,
                                        const UCHAR *module_id, UINT module_id_length,
                                        const NX_CRYPTO_METHOD **crypto_array, UINT crypto_array_size,
                                        const NX_CRYPTO_CIPHERSUITE **cipher_map, UINT cipher_map_size,
                                        UCHAR * metadata_memory, UINT memory_size
#ifdef NX_SECURE_ENABLE /*YUN*/
										,NX_SECURE_X509_CERT *trusted_certificate
#endif
										)
{
UINT status;
NX_AZURE_IOT_RESOURCE *resource_ptr;
az_span hostname_span = az_span_create((UCHAR *)host_name, (INT)host_name_length);
az_span device_id_span = az_span_create((UCHAR *)device_id, (INT)device_id_length);
az_iot_hub_client_options options = az_iot_hub_client_options_default();
az_result core_result;

    if ((nx_azure_iot_ptr == NX_NULL) || (hub_client_ptr == NX_NULL) || (host_name == NX_NULL) ||
        (device_id == NX_NULL) || (host_name_length == 0) || (device_id_length == 0))
    {
        LogError(LogLiteralArgs("IoTHub client initialization fail: INVALID POINTER"));
        return(NX_AZURE_IOT_INVALID_PARAMETER);
    }

    memset(hub_client_ptr, 0, sizeof(NX_AZURE_IOT_HUB_CLIENT));

    hub_client_ptr -> nx_azure_iot_ptr = nx_azure_iot_ptr;
    hub_client_ptr -> nx_azure_iot_hub_client_resource.resource_crypto_array = crypto_array;
    hub_client_ptr -> nx_azure_iot_hub_client_resource.resource_crypto_array_size = crypto_array_size;
    hub_client_ptr -> nx_azure_iot_hub_client_resource.resource_cipher_map = cipher_map;
    hub_client_ptr -> nx_azure_iot_hub_client_resource.resource_cipher_map_size = cipher_map_size;
    hub_client_ptr -> nx_azure_iot_hub_client_resource.resource_metadata_ptr = metadata_memory;
    hub_client_ptr -> nx_azure_iot_hub_client_resource.resource_metadata_size = memory_size;
#ifdef NX_SECURE_ENABLE /*YUN*/
    hub_client_ptr -> nx_azure_iot_hub_client_resource.resource_trusted_certificate = trusted_certificate;
#endif
    hub_client_ptr -> nx_azure_iot_hub_client_resource.resource_hostname = host_name;
    hub_client_ptr -> nx_azure_iot_hub_client_resource.resource_hostname_length = host_name_length;
    options.module_id = az_span_create((UCHAR *)module_id, (INT)module_id_length);
    options.user_agent = AZ_SPAN_FROM_STR(NX_AZURE_IOT_HUB_CLIENT_USER_AGENT);

    core_result = az_iot_hub_client_init(&hub_client_ptr -> iot_hub_client_core,
                                         hostname_span, device_id_span, &options);
    if (az_result_failed(core_result))
    {
        LogError(LogLiteralArgs("IoTHub client failed initialization with error status: %d"), core_result);
        return(NX_AZURE_IOT_SDK_CORE_ERROR);
    }

    /* Set resource pointer.  */
    resource_ptr = &(hub_client_ptr -> nx_azure_iot_hub_client_resource);

    /* Create MQTT client.  */
    status = _nxd_mqtt_client_cloud_create(&(resource_ptr -> resource_mqtt),
                                           (CHAR *)nx_azure_iot_ptr -> nx_azure_iot_name,
                                           "", 0,
                                           nx_azure_iot_ptr -> nx_azure_iot_ip_ptr,
                                           nx_azure_iot_ptr -> nx_azure_iot_pool_ptr,
                                           &nx_azure_iot_ptr -> nx_azure_iot_cloud);
    if (status)
    {
        LogError(LogLiteralArgs("IoTHub client initialization fail: MQTT CLIENT CREATE FAIL status: %d"), status);
        return(status);
    }

    /* Set mqtt receive notify.  */
    status = nxd_mqtt_client_receive_notify_set(&(resource_ptr -> resource_mqtt),
    											nbiot_service_iot_hub_client_mqtt_receive_callback);
    if (status)
    {
        LogError(LogLiteralArgs("IoTHub client set message callback status: %d"), status);
        nxd_mqtt_client_delete(&(resource_ptr -> resource_mqtt));
        return(status);
    }

    /* Link the resource.  */
    resource_ptr -> resource_data_ptr = (VOID *)hub_client_ptr;
    resource_ptr -> resource_type = NX_AZURE_IOT_RESOURCE_IOT_HUB;
    nx_azure_iot_resource_add(nx_azure_iot_ptr, resource_ptr);


    return(NX_AZURE_IOT_SUCCESS);
}

static UINT nbiot_service_dps_entry(void)
{
	UINT status;
	ULONG expiry_time_secs;
	/*//20210310 jason-
	UCHAR *iothub_hostname = (UCHAR *)HOST_NAME;
	UCHAR *iothub_device_id = (UCHAR *)DEVICE_ID;
	UINT iothub_hostname_length = sizeof(HOST_NAME) - 1;
	UINT iothub_device_id_length = sizeof(DEVICE_ID) - 1;
	*/
/* 20210310 jason+*/
#ifdef ENABLE_AZURE_PORTAL_DPS_HUB_DEVICE
	UCHAR *iothub_hostname = (UCHAR *)HOST_NAME;
	UINT iothub_hostname_length = sizeof(HOST_NAME) - 1;
#else
	UCHAR *iothub_hostname = (UCHAR *)ENDPOINT;
	UINT iothub_hostname_length = sizeof(ENDPOINT) - 1;
#endif

	UCHAR *iothub_device_id = (UCHAR *)AZURE_IOTHUB_DEVICE_ID;
	UINT iothub_device_id_length = sizeof(AZURE_IOTHUB_DEVICE_ID) - 1;

	expiry_time_secs = (ULONG)time(NULL);

	//xprintf("Create Azure IoT handler...\r\n");
	/* Create Azure IoT handler.  */
	if ((status = nx_azure_iot_create(&nx_azure_iot, (UCHAR *)"Azure IoT", &ip_0, &pool_0, &dns_0,
									  nx_azure_iot_thread_stack, sizeof(nx_azure_iot_thread_stack),
									  NX_AZURE_IOT_THREAD_PRIORITY, unix_time_get)))
	{
		xprintf("Failed on nx_azure_iot_create!: error code = 0x%08x\r\n", status);
		return(NX_AZURE_IOT_SDK_CORE_ERROR);
	}

    //xprintf("Start Provisioning Client...\r\n");

    /* Initialize IoT provisioning client.  */
    if (status = nbiot_service_iot_provisioning_client_initialize(expiry_time_secs))
    {
        xprintf("Failed on nx_azure_iot_provisioning_client_initialize!: error code = 0x%08x\r\n", status);
        return(status);
    }

    /* Initialize IoTHub client.  */
    if ((status = nbiot_service_iot_hub_client_initialize(&hub_client, &nx_azure_iot,
                                                     iothub_hostname, iothub_hostname_length,
                                                     iothub_device_id, iothub_device_id_length,
                                                     (UCHAR *)MODEL_ID, sizeof(MODEL_ID) - 1,
                                                     _nx_azure_iot_tls_supported_crypto,
                                                     _nx_azure_iot_tls_supported_crypto_size,
                                                     _nx_azure_iot_tls_ciphersuite_map,
                                                     _nx_azure_iot_tls_ciphersuite_map_size,
                                                     nx_azure_iot_tls_metadata_buffer,
                                                     sizeof(nx_azure_iot_tls_metadata_buffer),
                                                     &root_ca_cert)))
    {
        xprintf("Failed on nx_azure_iot_hub_client_initialize!: error code = 0x%08x\r\n", status);
        return(status);
    }

    return(status);
}

INT nbiot_service_get_dps_key(ULONG expiry_time_secs, UCHAR *resource_dps_sas_token)
{
		UINT status;
		NX_AZURE_IOT_RESOURCE *resource_ptr;
		UCHAR *output_ptr;
		UINT output_len;
		az_span span;
		az_result core_result;
		az_span buffer_span;
		az_span policy_name = AZ_SPAN_LITERAL_FROM_STR(NX_AZURE_IOT_PROVISIONING_CLIENT_POLICY_NAME);

	    /* Set resource pointer.  */
	    resource_ptr = &(dps_client.nx_azure_iot_provisioning_client_resource);

	    span = az_span_create(resource_ptr->resource_mqtt_sas_token,
	                          (INT)dps_client.nx_azure_iot_provisioning_client_sas_token_buff_size);

	    status = nx_azure_iot_buffer_allocate(dps_client.nx_azure_iot_ptr,
	                                          &buffer_ptr, &buffer_size,
	                                          &buffer_context);
	    //xprintf("nx_azure_iot_buffer_allocate: BUFFER size %d\r\n", buffer_size);
	    if (status)
	    {
	        xprintf("IoTProvisioning client sas token fail: BUFFER ALLOCATE FAI\r\n");
	        return(status);
	    }

	    //xprintf("expiry_time_secs %ld\r\n", expiry_time_secs);

	    core_result = az_iot_provisioning_client_sas_get_signature(&(dps_client.nx_azure_iot_provisioning_client_core),
	                                                               expiry_time_secs, span, &span);
	    //xprintf("az_iot_provisioning_client_sas_get_signature\r\n");
	    if (az_result_failed(core_result))
	    {
	        xprintf("IoTProvisioning failed failed to get signature with error status: %d\r\n", core_result);
	        return(NX_AZURE_IOT_SDK_CORE_ERROR);
	    }
	    //xprintf("prov_client.nx_azure_iot_provisioning_client_symmetric_key %s\r\n", dps_client.nx_azure_iot_provisioning_client_symmetric_key);
	    //xprintf("prov_client.nx_azure_iot_provisioning_client_symmetric_key_length %d\r\n", dps_client.nx_azure_iot_provisioning_client_symmetric_key_length);

	    status = nx_azure_iot_base64_hmac_sha256_calculate(resource_ptr,
	    												   dps_client.nx_azure_iot_provisioning_client_symmetric_key,
														   dps_client.nx_azure_iot_provisioning_client_symmetric_key_length,
	                                                       az_span_ptr(span), (UINT)az_span_size(span), buffer_ptr, buffer_size,
	                                                       &output_ptr, &output_len);
	    //xprintf("output_ptr %s\r\n", output_ptr);
	    if (status)
	    {
	        xprintf("IoTProvisioning failed to encoded hash\r\n");
	        return(status);
	    }

	    buffer_span = az_span_create(output_ptr, (INT)output_len);

	    //xprintf("11expiry_time_secs %ld\r\n", expiry_time_secs);

	    core_result = az_iot_provisioning_client_sas_get_password(&(dps_client.nx_azure_iot_provisioning_client_core),
	                                                              buffer_span, expiry_time_secs, policy_name,
	                                                              (CHAR *)resource_ptr -> resource_mqtt_sas_token,
																  dps_client.nx_azure_iot_provisioning_client_sas_token_buff_size,
	                                                              &(resource_ptr -> resource_mqtt_sas_token_length));
	    if (az_result_failed(core_result))
	    {
	        xprintf("IoTProvisioning failed to generate token with error : %d\r\n", core_result);
	        return(NX_AZURE_IOT_SDK_CORE_ERROR);
	    }
	    //xprintf("resource_mqtt_sas_token %s\n",resource_ptr -> resource_mqtt_sas_token);
	    strcpy(azure_iotdps_connect_password, resource_ptr -> resource_mqtt_sas_token);
	   	//xprintf("\n*** azure_iotdps_connect_password:%s ***\n", azure_iotdps_connect_password);

	   	resource_dps_sas_token = resource_ptr -> resource_mqtt_sas_token;
	    return(NX_AZURE_IOT_SUCCESS);
}

static UINT nbiot_service_iot_hub_client_sas_token_get(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr,
                                                  ULONG expiry_time_secs, const UCHAR *key, UINT key_len,
                                                  UCHAR *sas_buffer, UINT sas_buffer_len, UINT *sas_length)
{
UCHAR *buffer_ptr;
UINT buffer_size;
VOID *buffer_context;
az_span span = az_span_create(sas_buffer, (INT)sas_buffer_len);
az_span buffer_span;
UINT status;
UCHAR *output_ptr;
UINT output_len;
az_result core_result;

    status = nx_azure_iot_buffer_allocate(hub_client_ptr -> nx_azure_iot_ptr, &buffer_ptr, &buffer_size, &buffer_context);
    if (status)
    {
        LogError(LogLiteralArgs("IoTHub client sas token fail: BUFFER ALLOCATE FAIL"));
        return(status);
    }

    core_result = az_iot_hub_client_sas_get_signature(&(hub_client_ptr -> iot_hub_client_core),
                                                      expiry_time_secs, span, &span);
    if (az_result_failed(core_result))
    {
        LogError(LogLiteralArgs("IoTHub failed failed to get signature with error status: %d"), core_result);
        nx_azure_iot_buffer_free(buffer_context);
        return(NX_AZURE_IOT_SDK_CORE_ERROR);
    }

    status = nx_azure_iot_base64_hmac_sha256_calculate(&(hub_client_ptr -> nx_azure_iot_hub_client_resource),
                                                       key, key_len, az_span_ptr(span), (UINT)az_span_size(span),
                                                       buffer_ptr, buffer_size, &output_ptr, &output_len);
    if (status)
    {
        LogError(LogLiteralArgs("IoTHub failed to encoded hash"));
        nx_azure_iot_buffer_free(buffer_context);
        return(status);
    }

    buffer_span = az_span_create(output_ptr, (INT)output_len);
    core_result= az_iot_hub_client_sas_get_password(&(hub_client_ptr -> iot_hub_client_core),
                                                    expiry_time_secs, buffer_span, AZ_SPAN_EMPTY,
                                                    (CHAR *)sas_buffer, sas_buffer_len, &sas_buffer_len);
    if (az_result_failed(core_result))
    {
        LogError(LogLiteralArgs("IoTHub failed to generate token with error status: %d"), core_result);
        nx_azure_iot_buffer_free(buffer_context);
        return(NX_AZURE_IOT_SDK_CORE_ERROR);
    }

    *sas_length = sas_buffer_len;
    nx_azure_iot_buffer_free(buffer_context);

    return(NX_AZURE_IOT_SUCCESS);
}

INT nbiot_service_get_iothub_key(ULONG expiry_time_secs, UCHAR *resource_dps_sas_token)
//INT nbiot_service_get_iothub_key(ULONG expiry_time_secs, UCHAR *resource_dps_sas_token,UCHAR *host_name )
{
	UINT            status;
	NXD_ADDRESS     server_address;
	NX_AZURE_IOT_RESOURCE *resource_ptr;
	NXD_MQTT_CLIENT *mqtt_client_ptr;
	UCHAR           *buffer_ptr;
	UINT            buffer_size;
	VOID            *buffer_context;
	UINT            buffer_length;
	az_result       core_result;

    /* Resolve the host name.  */
#if 0
    /* Note: always using default dns timeout.  */
    status = nxd_dns_host_by_name_get(hub_client.nx_azure_iot_ptr -> nx_azure_iot_dns_ptr,
                                      (UCHAR *)hub_client.nx_azure_iot_hub_client_resource.resource_hostname,
                                      &server_address, NX_AZURE_IOT_HUB_CLIENT_DNS_TIMEOUT, NX_IP_VERSION_V4);
    if (status)
    {
    	xprintf("IoTHub client connect fail: DNS RESOLVE FAIL status: %d\n", status);
        return(status);
    }
#endif

    /* Allocate buffer for client id, username and sas token.  */
    status = nx_azure_iot_buffer_allocate(hub_client.nx_azure_iot_ptr,
                                          &buffer_ptr, &buffer_size, &buffer_context);
    if (status)
    {
    	xprintf("IoTHub client failed initialization: BUFFER ALLOCATE FAIL\n");
        return(status);
    }

    /* Set resource pointer and buffer context.  */
    resource_ptr = &(hub_client.nx_azure_iot_hub_client_resource);

    /* Build client id.  */
    buffer_length = buffer_size;
    core_result = az_iot_hub_client_get_client_id(&(hub_client.iot_hub_client_core),
                                                  (CHAR *)buffer_ptr, buffer_length, &buffer_length);
    if (az_result_failed(core_result))
    {
        nx_azure_iot_buffer_free(buffer_context);
        xprintf("IoTHub client failed to get clientId with error status: \n", core_result);
        return(NX_AZURE_IOT_SDK_CORE_ERROR);
    }
    resource_ptr -> resource_mqtt_client_id = buffer_ptr;
    resource_ptr -> resource_mqtt_client_id_length = buffer_length;

    /* Update buffer for user name.  */
    buffer_ptr += resource_ptr -> resource_mqtt_client_id_length;
    buffer_size -= resource_ptr -> resource_mqtt_client_id_length;

    /* Build user name.  */
    buffer_length = buffer_size;
    core_result = az_iot_hub_client_get_user_name(&hub_client.iot_hub_client_core,
                                                  (CHAR *)buffer_ptr, buffer_length, &buffer_length);
    if (az_result_failed(core_result))
    {
        nx_azure_iot_buffer_free(buffer_context);
        xprintf("IoTHub client connect fail, with error status: %d\n", core_result);
        return(NX_AZURE_IOT_SDK_CORE_ERROR);
    }
    resource_ptr -> resource_mqtt_user_name = buffer_ptr;
    resource_ptr -> resource_mqtt_user_name_length = buffer_length;

    /* Build sas token.  */
    resource_ptr -> resource_mqtt_sas_token = buffer_ptr + buffer_length;
    resource_ptr -> resource_mqtt_sas_token_length = buffer_size - buffer_length;

    hub_client.nx_azure_iot_hub_client_symmetric_key = (UCHAR *)DEVICE_SYMMETRIC_KEY;
    hub_client.nx_azure_iot_hub_client_symmetric_key_length = sizeof(DEVICE_SYMMETRIC_KEY) - 1;

    /* Host Name. 20210309 jason+*/
    hub_client.iot_hub_client_core._internal.iot_hub_hostname._internal.ptr = (UCHAR *)azure_iothub_connect_host_name;
    hub_client.iot_hub_client_core._internal.iot_hub_hostname._internal.size = strlen(azure_iothub_connect_host_name) - 1;

    //xprintf("\n*** hub_client_iothub_host_name:%s ***\n", hub_client.iot_hub_client_core._internal.iot_hub_hostname._internal.ptr);

    expiry_time_secs += NX_AZURE_IOT_HUB_CLIENT_TOKEN_EXPIRY;
    status = nbiot_service_iot_hub_client_sas_token_get(&hub_client,
                                                       expiry_time_secs,
                                                       hub_client.nx_azure_iot_hub_client_symmetric_key,
                                                       hub_client.nx_azure_iot_hub_client_symmetric_key_length,
                                                       resource_ptr -> resource_mqtt_sas_token,
                                                       resource_ptr -> resource_mqtt_sas_token_length,
                                                       &(resource_ptr -> resource_mqtt_sas_token_length));
    if (status)
    {
        nx_azure_iot_buffer_free(buffer_context);
        xprintf("IoTHub client connect fail: Token generation failed status: %d", status);
        return(status);
    }
    resource_dps_sas_token = resource_ptr -> resource_mqtt_sas_token;
    //xprintf("resource_mqtt_sas_token %s\n",resource_ptr -> resource_mqtt_sas_token);

    strcpy(azure_iothub_connect_password, resource_ptr -> resource_mqtt_sas_token);
    //xprintf("\n*** azure_iothub_connect_password:%s ***\n", azure_iothub_connect_password);

    return(NX_AZURE_IOT_SUCCESS);
}

/* Define nbiot service thread entry.  */
void nbiot_service_thread_entry(ULONG parameter)
{
	ULONG expiry_time_secs = 0;
	UCHAR *resource_dps_sas_token = NULL;
	//TX_INTERRUPT_SAVE_AREA
	unsigned int azure_iotdps_timeout_cnt = 0 ,azure_iothub_timeout_cnt = 0, azure_atcmd_retry_cnt = 0;
	xprintf("****Enter nbiot_service_thread_entry ****\n");
	nbiot_service_dps_entry();
	expiry_time_secs = (ULONG)time(NULL);
	nbiot_service_get_dps_key(expiry_time_secs, resource_dps_sas_token);
	//nbiot_service_get_iothub_key(expiry_time_secs, resource_dps_sas_token);

	//TX_INTERRUPT_SAVE_AREA

#ifdef NETWORK_EN
	/*#############################################################################*/
	xprintf("\n#############################################################################\n");
	xprintf("**** Enter Azure DPS Connect...!! ****\n");
	xprintf("#############################################################################\n");
	/*#############################################################################*/
	/* Azure PNP DPS¡@Event initial. */
	azure_pnp_iotdps_event = PNP_IOTDPS_INITIAL;
	/* Azure PNP IoTHub¡@Event initial. */
	azure_pnp_iothub_event = PNP_IOTHUB_NBIOT_CERTIFICATION;


	/*"ID_SCOPE/registrations/REGISTRATION_ID/api-version=2019-03-31"*/
	azure_iotdps_connect_user_name ="\"" ID_SCOPE "/registrations/" REGISTRATION_ID
		                                           "/api-version=" AZURE_IOTDPS_SERVICE_VERSION "\"";
	//xprintf("**** azure_iotdps_connect_user_name: %s ****\n",azure_iotdps_connect_user_name);

	/* {"registrationId":REGISTRATION_ID,"payload":{"modelId":MODEL_ID}}*/
	char azure_iotdps_registrations_msg_ascii[] = "{\"registrationId\":\"" REGISTRATION_ID "\""
			",\"payload\":{\"modelId\":\"" MODEL_ID "\"}}";
	//xprintf("**** azure_iotdps_registrations_msg_ascii: %s ****\n",azure_iotdps_registrations_msg_ascii);


	for (int i = 0; i < strlen(azure_iotdps_registrations_msg_ascii); i++){
		sprintf((azure_iotdps_registrations_msg + (i* 2)), "%02X", *(azure_iotdps_registrations_msg_ascii + i));
	 }
	//xprintf("**** azure_iotdps_registrations_msg: %s ****\n",azure_iotdps_registrations_msg);
	sprintf(azure_registrations_msg_len, "%d", strlen(azure_iotdps_registrations_msg));

	while(1)
  	{
		io_uart_read(DFSS_UART_0_ID, (uint8_t *) recv_buf, &recv_len, UART_TRANSFER_INT);
		//xprintf("[IoT DPS]\nrecv_len:%d %s\n",strlen(recv_buf),recv_buf);
		tx_thread_sleep(1);

		switch (azure_pnp_iotdps_event)
		{
		case PNP_IOTDPS_INITIAL: /* 0:NBIOT Initial .... */
			if ((strstr(recv_buf, "+IP")!= NULL)){
				memset(recv_buf,0,AT_MAX_LEN);//clear buffer
				//azure_pnp_iotdps_event = PNP_IOTDPS_NBIOT_CERTIFICATION;
				azure_pnp_iotdps_event = PNP_IOTDPS_GET_NETWORK_TIME;
//0309-
//				if(0 > wnb303r_query_time(dev_uart_comm))
//				{
//					xprintf(" Azure IoT DPS query network time Send Fail.\n");
//				}

				tx_thread_sleep(1);
				break;
			}
		break;
		case PNP_IOTDPS_GET_NETWORK_TIME: /* 1:Get Network time*/
			//if(azure_iotdps_get_network_time()){
			if(strstr(recv_buf,"+CCLK:")){
				/*get network time. */
				azure_iotdps_get_network_time();
				azure_pnp_iotdps_event = PNP_IOTDPS_NBIOT_CERTIFICATION;
				azure_iotdps_timeout_cnt = 0;
				tx_thread_sleep(1);
				break;
			}else{
				if(0 > wnb303r_query_time(dev_uart_comm))
				{
					xprintf(" Azure IoT DPS query network time Send Fail.\n");
				}
				azure_iotdps_timeout_cnt++;
			}
		tx_thread_sleep(1);
		break;
		case PNP_IOTDPS_NBIOT_CERTIFICATION: /* 2: NBIOT Certification. */
			if(strstr(recv_buf, AT_OK_STR)!= NULL)
			{
				memset(recv_buf,0,AT_MAX_LEN);//clear buffer
				//xprintf("PNP_IOTDPS_NBIOT_CERTIFICATION OK!!\n");
				azure_pnp_iotdps_event = PNP_IOTDPS_CREATE_CONNECTION;
				azure_iotdps_timeout_cnt = 0;
				if(0 >  wnb303r_MQTT_create(dev_uart_comm, "\"" ENDPOINT "\"", NBIOT_MQTT_TLS_PORT, "1200", "1000"))
				{
					xprintf("Azure IoT DPS connection with server over TCP Send Fail.\n");
				}
				tx_thread_sleep(1);
				break;
			}else{
				if(0 > wnb303r_MQTT_certification(dev_uart_comm, "3", "0", "0", "0,"))
				{
						xprintf("MQTTS Certification Send Fail.\n");
				}
				azure_iotdps_timeout_cnt++;
			}
		tx_thread_sleep(1);
		break;
		case PNP_IOTDPS_CREATE_CONNECTION: /* 3:Create MQTT¡@Connection */
			//0208- if(strstr(recv_buf, "+EMQNEW")!= NULL)
			if(strstr(recv_buf, "+EMQNEW:")!= NULL)
			{
				memset(recv_buf,0,AT_MAX_LEN);//clear buffer
				azure_pnp_iotdps_event = PNP_IOTDPS_CONNECT_TO_DPS;
				azure_iotdps_timeout_cnt = 0;
				if(0 >  wnb303r_MQTT_send_connect_packet(dev_uart_comm, "0", "4", "\"" REGISTRATION_ID"\"" ,"240","1", "0", azure_iotdps_connect_user_name, azure_iotdps_connect_password))
				{
					xprintf("Azure IoT DPS Connect to DPS Send Fail.\n");
				}
				tx_thread_sleep(1);
				break;
			}
			azure_iotdps_timeout_cnt++;
			tx_thread_sleep(1);
		break;
		case PNP_IOTDPS_CONNECT_TO_DPS: /* 4:MQTT connection to Cloud. */
			if(strstr(recv_buf, AT_OK_STR)!= NULL)
			{
				memset(recv_buf,0,AT_MAX_LEN);//clear buffer
				azure_pnp_iotdps_event = PNP_IOTDPS_REGISTRATION_SUBSCRIBE;
				azure_iotdps_timeout_cnt = 0;
				tx_thread_sleep(1);
				break;
			}else if (strstr(recv_buf, AT_ERROR_STR)!= NULL)
			{
				xprintf("PNP_IOTDPS_CONNECT_TO_DPS Fail.\n");
			}
			azure_iotdps_timeout_cnt++;
			tx_thread_sleep(1);
		 break;
		 case PNP_IOTDPS_REGISTRATION_SUBSCRIBE: /* 5:MQTT subscribe packet. */
			if(strstr(recv_buf, AT_OK_STR)!= NULL){
				memset(recv_buf,0,AT_MAX_LEN);//clear buffer
				azure_pnp_iotdps_event = PNP_IOTDPS_REGISTRATION_PUBLISH;
				azure_iotdps_timeout_cnt = 0;
				if(0 >wnb303r_MQTT_send_publish_packet(dev_uart_comm, "0", AZURE_IOTDPS_CLIENT_REGISTER_PUBLISH_TOPIC, "1", "0", "0", \
				azure_registrations_msg_len,azure_iotdps_registrations_msg))
				{
					xprintf("Azure IoT DPS publish packet Send Fail.\n");
				}
				tx_thread_sleep(1);
				break;
			}else{
				if(0 > wnb303r_MQTT_send_subscribe_packet(dev_uart_comm, "0", "\"" AZURE_IOTDPS_CLIENT_REGISTER_SUBSCRIBE_TOPIC "\"", "1"))
				{
					xprintf("Azure IoTDPS subscribe packet Send Fail.\n");
				}

				azure_iotdps_timeout_cnt++;
			}
		tx_thread_sleep(1);
		 break;
		case PNP_IOTDPS_REGISTRATION_PUBLISH: /* 6:MQTT publish packet.*/
			if (strstr(recv_buf, "+EMQPUB:")!= NULL )
			{
				azure_pnp_iotdps_event = PNP_IOTDPS_GET_REGISTRATION_OPERATION_ID;
				azure_iotdps_timeout_cnt = 0;
				tx_thread_sleep(1);//1
				break;
			}
			azure_iotdps_timeout_cnt++;
		tx_thread_sleep(1);
		break;
		case PNP_IOTDPS_GET_REGISTRATION_OPERATION_ID: /* 7:parsing registration operation ID.*/
			if(azure_pnp_iotdps_get_registration_operation_id())
			{
				azure_pnp_iotdps_event = PNP_IOTDPS_GET_REGISTRATION_STATUS;
				azure_iotdps_timeout_cnt = 0;
				tx_thread_sleep(1);
				break;
			}
			azure_iotdps_timeout_cnt++;
		tx_thread_sleep(1);
		break;
		case PNP_IOTDPS_GET_REGISTRATION_STATUS: /* 8:parsing registration status.*/
			if(azure_pnp_iotdps_get_registration_status())
			{
				azure_pnp_iotdps_event = PNP_IOTDPS_REGISTRATION_DONE;
				azure_iotdps_timeout_cnt = 0;
				tx_thread_sleep(1);
				break;
			}
			azure_iotdps_timeout_cnt ++;
		//tx_thread_sleep(1);
		break;
		case PNP_IOTDPS_RECONNECT:
		{
			azure_iotdps_timeout_cnt = 0;

			if(strstr(recv_buf,AT_OK_STR)){
				memset(recv_buf,0,AT_MAX_LEN);//clear buffer
				azure_pnp_iotdps_event = PNP_IOTDPS_GET_NETWORK_TIME;
			}else{
				/*MQTT Disconnect DPS*/
				if(0 > wnb303r_MQTT_disconnect(dev_uart_comm,"0"))
				{
					xprintf("Azure IoT DPS Disconnect Send Fail.\n");
				}
			}

#if 0
			int cur_tick0, cur_tick1;
			xprintf("**** wnb303r reset ...****\n");
		    /* wnb303r reset. */
		    hx_drv_iomux_set_pmux(IOMUX_PGPIO14, 3);
		    hx_drv_iomux_set_outvalue(IOMUX_PGPIO14, 1);
		    cur_tick0 = _arc_aux_read(AUX_TIMER0_CNT);
		    tx_thread_sleep(1);
		    cur_tick1 = _arc_aux_read(AUX_TIMER0_CNT);
		    hx_drv_iomux_set_outvalue(IOMUX_PGPIO14, 0);
		    xprintf("##### diff_tick = %d\n", cur_tick1 - cur_tick0);
#endif
		    tx_thread_sleep(1);
		}
		break;
		default:
		break;
		} //switch
		//xprintf("*** azure_pnp_iotdps_event = %d , azure_iotdps_timeout_cnt = %d ***\n",azure_pnp_iotdps_event, azure_iotdps_timeout_cnt);
		xprintf("*** azure_pnp_iotdps_event = %d ***\n",azure_pnp_iotdps_event);

		if(azure_pnp_iotdps_event == PNP_IOTDPS_REGISTRATION_DONE) /* MQTT Connect DPS Success. */
		{
#if 0
			azure_iotdps_timeout_cnt = 0;
			if(strstr(recv_buf,AT_OK_STR)){
				xprintf("\n**** PNP_IOTDPS_REGISTRATION_DONE ****\n");
				memset(recv_buf,0,AT_MAX_LEN);//clear buffer
				break;
			}else{
				/*MQTT Disconnect DPS*/
				if(0 > wnb303r_MQTT_disconnect(dev_uart_comm,"0"))
				{
					xprintf("Azure IoT DPS Disconnect Send Fail.\n");
				}
				tx_thread_sleep(1);
			}
#else
			xprintf("\n**** PNP_IOTDPS_REGISTRATION_DONE ****\n");
			azure_iotdps_timeout_cnt = 0;
			/*MQTT Disconnect DPS*/
			if(0 > wnb303r_MQTT_disconnect(dev_uart_comm,"0"))
			{
				xprintf("Azure IoT DPS Disconnect Send Fail.\n");
			}
			break;
#endif
		}

		if(azure_iotdps_timeout_cnt == AZURE_CONNECT_RETRY_MAX_TIMES)
		//if(azure_pnp_iotdps_event == PNP_IOTDPS_RE_CONNECT)
		{
			xprintf("\n**** AZURE_PNP_IOTDPS_RECONNECT.... ****\n");
			memset(recv_buf,0,AT_MAX_LEN);//clear buffer
			azure_pnp_iotdps_event = PNP_IOTDPS_RECONNECT;
		}

		tx_thread_sleep(1);
  	}//end while

	/*#############################################################################*/
//		//for test
//		azure_pnp_iothub_event = PNP_IOTHUB_INITIAL;
//		while(1)
//		{
//			io_uart_read(DFSS_UART_0_ID, (uint8_t *) recv_buf, &recv_len, UART_TRANSFER_INT);
//			xprintf("[RESET TEST]\nrecv_buf:%d %s\n",strlen(recv_buf),recv_buf);
//			tx_thread_sleep(1);
//
//			switch(azure_pnp_iothub_event)
//			{
//			case PNP_IOTHUB_INITIAL: /* 0:NBIOT Initial .... */
//				if ((strstr(recv_buf, "+IP")!= NULL)){
//					memset(recv_buf,0,AT_MAX_LEN);//clear buffer
//					azure_pnp_iothub_event = PNP_IOTHUB_NBIOT_CERTIFICATION;
//					tx_thread_sleep(1);
//					break;
//				}else{
////					/* send "AT" command. */
//					wnb303r_drv_write_at_cmd(dev_uart_comm,AT_EXECUTE, NULL);
//				}
//			break;
//			case PNP_IOTHUB_NBIOT_CERTIFICATION:
//			{
//				int cur_tick0, cur_tick1;
//				memset(recv_buf,0,AT_MAX_LEN);//clear buffer
//
//				/* Test reset pin. */
//				azure_pnp_iothub_event = PNP_IOTHUB_INITIAL;
//				/* Test wakeup-in pin. */
//				//azure_pnp_iothub_event = PNP_IOTHUB_CONNECT_TO_DEVICE_DONE;
//
//				//azure_iothub_timeout_cnt=0;
//
//				/* waiting wnb303r enter sleep mode. */
//				//xprintf("\n**** waiting wnb303r enter sleep mode ****\n");
//				//wnb303r_drv_control_sleep_mode(dev_uart_comm,"UNLOCK");
//				/* GG*/
//				xprintf("### wnb303r reset ...###\n");
//				hx_drv_iomux_set_pmux(IOMUX_PGPIO14, 3);
//				hx_drv_iomux_set_outvalue(IOMUX_PGPIO14, 1);
//				//board_delay_ms(150);
//				tx_thread_sleep(1);
//				hx_drv_iomux_set_outvalue(IOMUX_PGPIO14, 0);
//				/* */
//
////				/* wnb303r reset. */
////				xprintf("### wnb303r reset ...###\n");
////				hx_drv_iomux_set_pmux(IOMUX_PGPIO14, 3);
////				hx_drv_iomux_set_outvalue(IOMUX_PGPIO14, 1);
////				//cur_tick0 = _arc_aux_read(AUX_TIMER0_CNT);
////				tx_thread_sleep(2);
////				//cur_tick1 = _arc_aux_read(AUX_TIMER0_CNT);
////				hx_drv_iomux_set_outvalue(IOMUX_PGPIO14, 0);
////				//xprintf("##### diff_tick = %d\n", cur_tick0);
////				//xprintf("##### diff_tick = %d\n", cur_tick1);
//			tx_thread_sleep(1);
//			break;
//			}
//			case PNP_IOTHUB_CONNECT_TO_DEVICE_DONE:
//			xprintf("\n**** azure_iothub_timeout_cnt:%d ****\n",azure_iothub_timeout_cnt);
//			azure_iothub_timeout_cnt++;
//			tx_thread_sleep(1);
//			break;
//			default:
//			break;
//			}//switch
//
//			if(azure_iothub_timeout_cnt >= 50){
//
//				azure_pnp_iothub_event = PNP_IOTHUB_INITIAL;
//
//				azure_iothub_timeout_cnt = 0;
//
//				/* wnb303r wakeup-in. */
//				xprintf("### wnb303r wakeup-in ...###\n");
//				hx_drv_iomux_set_pmux(IOMUX_PGPIO13, 3);
//				hx_drv_iomux_set_outvalue(IOMUX_PGPIO13, 1);
//				//board_delay_ms(150);
//				tx_thread_sleep(1);
//				hx_drv_iomux_set_outvalue(IOMUX_PGPIO13, 0);
//
//				board_delay_ms(150);
//
//				xprintf("### query IP ...###\n");
//				wnb303r_query_ip(dev_uart_comm);
//				/* send "AT" command. */
//				//wnb303r_drv_write_at_cmd(dev_uart_comm,AT_EXECUTE, NULL);
//
////			    /* wnb303r power on. */
////			    hx_drv_iomux_set_pmux(IOMUX_PGPIO12, 3);
////			    hx_drv_iomux_set_outvalue(IOMUX_PGPIO12, 1);
////			    //board_delay_ms(600);
////				tx_thread_sleep(1);
////			    hx_drv_iomux_set_outvalue(IOMUX_PGPIO12, 0);
//
//
//
////				xprintf("wnb303r_drv_deinit\n");
////				if(0>wnb303r_drv_deinit(DFSS_UART_0_ID)){
////					xprintf("wnb303r_drv_deinit fail\n");
////				}
////
////				tx_thread_sleep(1);
////
////				xprintf("wnb303r_init\n");
////			    dev_uart_comm = wnb303r_drv_init(DFSS_UART_0_ID, UART_BAUDRATE_115200);
////			    if(dev_uart_comm == NULL)
////			    {
////			    	xprintf("wnb303r_init fail\n");
////			    }
//
//			}
//			tx_thread_sleep(1);
//		}//while

	/*#############################################################################*/
	xprintf("wnb303r_drv_deinit\n");
	if(0>wnb303r_drv_deinit(DFSS_UART_0_ID)){
		xprintf("wnb303r_drv_deinit fail\n");
	}

	tx_thread_sleep(1);

	xprintf("wnb303r_init\n");
    dev_uart_comm = wnb303r_drv_init(DFSS_UART_0_ID, UART_BAUDRATE_115200);
    if(dev_uart_comm == NULL)
    {
    	xprintf("wnb303r_init fail\n");
    }

    /*#############################################################################*/
	/*#############################################################################*/
	xprintf("\n#############################################################################\n");
	xprintf("**** Enter Azure IoTHUB Connect...!! ****\n");
	xprintf("#############################################################################\n");
	/*#############################################################################*/
	/* azure_iothub_connect_host_name/AZURE_IOTHUB_DEVICE_ID/?api-version=2020-09-30&model-id= .*/
	//azure_iothub_connect_user_name ="\"" AZURE_IOTHUB_HOST_NAME "/" AZURE_IOTHUB_DEVICE_ID "/?api-version=" AZURE_IOTHUB_SERVICE_VERSION "&model-id=" MODEL_ID "\"";

	//azure_iothub_connect_user_name ="\"" "iotc-204261de-05de-420c-ad3c-9e030e247346.azure-devices.net" "/" AZURE_IOTHUB_DEVICE_ID "/?api-version=" AZURE_IOTHUB_SERVICE_VERSION "&model-id=" MODEL_ID "\"";
	strcat(azure_iothub_connect_user_name,"\"");
	strcat(azure_iothub_connect_user_name,azure_iothub_connect_host_name);
	strcat(azure_iothub_connect_user_name,"/");
	char azure_iothub_connect_user_name_tmp[] = AZURE_IOTHUB_DEVICE_ID "/?api-version=" AZURE_IOTHUB_SERVICE_VERSION "&model-id=" MODEL_ID "\"";
	/* host_name/device_id/?api-version=2020-09-30&model-id=dtmi:himax:weiplus;1*/
	strcat(azure_iothub_connect_user_name,azure_iothub_connect_user_name_tmp);
	//xprintf("**** azure_iothub_connect_user_name: %s ****\n",azure_iothub_connect_user_name);

	/* azure iothub publish topic. */
	azure_iothub_publish_topic ="devices/" AZURE_IOTHUB_DEVICE_ID "/messages/events/";
	/* devices/device_id/messages/events/ */
	//xprintf("**** azure_iothub_publish_topic: %s ****\n",azure_iothub_publish_topic);

	memset(recv_buf,0,AT_MAX_LEN);//clear buffer
	//azure_pnp_iothub_event = PNP_IOTHUB_INITIAL;

	tx_thread_sleep(1);
	while(1)
	{
		io_uart_read(DFSS_UART_0_ID, (uint8_t *) recv_buf, &recv_len, UART_TRANSFER_INT);
		//xprintf("[IoT HUB]\nrecv_buf:%d %s\n",strlen(recv_buf),recv_buf);
		tx_thread_sleep(1);
		switch(azure_pnp_iothub_event)
		{
		case PNP_IOTHUB_INITIAL: /* 0:NBIOT Initial .... */
			if ((strstr(recv_buf, "+IP")!= NULL)){
				memset(recv_buf,0,AT_MAX_LEN);//clear buffer
				azure_pnp_iothub_event = PNP_IOTHUB_NBIOT_CERTIFICATION;
				tx_thread_sleep(1);
				break;
			}
		break;
		case PNP_IOTHUB_NBIOT_CERTIFICATION: /* 1: NBIOT Certification. */
			if(strstr(recv_buf, AT_OK_STR)!= NULL)
			{
				memset(recv_buf,0,AT_MAX_LEN);//clear buffer
				//xprintf("PNP_IOTDPS_NBIOT_CERTIFICATION OK!!\n");
				azure_pnp_iothub_event = PNP_IOTHUB_CREATE_CONNECTION;
				azure_iothub_timeout_cnt = 0;
				if(0 >  wnb303r_MQTT_create(dev_uart_comm, azure_iothub_connect_host_name, NBIOT_MQTT_TLS_PORT, "1200", "1000"))
				{
					xprintf("Azure IoT Hub connection with server over TCP Send Fail.\n");
				}
				tx_thread_sleep(1);
				break;
			}else{
				if(0 > wnb303r_MQTT_certification(dev_uart_comm, "3", "0", "0", "0,"))
				{
						xprintf("MQTTS Certification Send Fail.\n");
				}
				azure_iothub_timeout_cnt++;
			}
		tx_thread_sleep(1);
		break;
		case PNP_IOTHUB_CREATE_CONNECTION: /* 2:Create Connection */
			if(strstr(recv_buf, "+EMQNEW:")!= NULL)
			{
				memset(recv_buf,0,AT_MAX_LEN);//clear buffer
				azure_pnp_iothub_event = PNP_IOTHUB_CONNECT_TO_DEVICE;
				azure_iothub_timeout_cnt = 0;
				if(0 >  wnb303r_MQTT_send_connect_packet(dev_uart_comm, "0", "4", "\""AZURE_IOTHUB_DEVICE_ID"\"","240","1", "0", azure_iothub_connect_user_name, azure_iothub_connect_password))
				{
					xprintf("Azure IoT Hub Connect to Device Send Fail.\n");
				}
				tx_thread_sleep(1);
				break;
			}
			azure_iothub_timeout_cnt++;
		tx_thread_sleep(1);
		break;
		case PNP_IOTHUB_CONNECT_TO_DEVICE:/* 3:Connect to device*/
			if(strstr(recv_buf, AT_OK_STR)!= NULL)
			{
				memset(recv_buf,0,AT_MAX_LEN);//clear buffer
				azure_pnp_iothub_event = PNP_IOTHUB_MESSAGE_PUBLISH;//PNP_IOTHUB_CONNECT_TO_DEVICE_DONE;
				azure_iothub_timeout_cnt = 0;
				tx_thread_sleep(1);
				break;
			}else if(strstr(recv_buf, AT_ERROR_STR)!= NULL){
				xprintf("PNP_IOTHUB_CONNECT_TO_DEVICE Fail.\n");
			}else{
				xprintf("***ERROR\nPNP_IOTHUB_CONNECT_TO_DEVICE***\nrecv_buf:%d %s\n",strlen(recv_buf),recv_buf);
			}
			azure_iothub_timeout_cnt++;
		tx_thread_sleep(1);
		break;
		case PNP_IOTHUB_MESSAGE_PUBLISH:/* 4:publish message to cloud*/
			azure_pnp_iothub_event = PNP_IOTHUB_CONNECT_TO_DEVICE_DONE;//PNP_IOTHUB_IMAGE_PUBLISH;
		tx_thread_sleep(1);
		break;
		case PNP_IOTHUB_CONNECT_TO_DEVICE_DONE :
			azure_pnp_iothub_event = PNP_IOTHUB_CONNECTIING; // not do thing.
			//azure_algo_event = ALGO_EVENT_SEND_RESULT_TO_CLOUD;
			azure_algo_event = ALGO_EVENT_IDLE;
			azure_iothub_timeout_cnt = 0;
		tx_thread_sleep(1);
		break;
		case PNP_IOTHUB_RECONNECT:
		{
			azure_iothub_timeout_cnt = 0;
			if(strstr(recv_buf,AT_OK_STR)){
				memset(recv_buf,0,AT_MAX_LEN);//clear buffer
				azure_pnp_iothub_event = PNP_IOTHUB_NBIOT_CERTIFICATION;
				/* regenerate azure_iothub sas key. */
				//nbiot_service_get_iothub_key(azure_iotdps_epoch_time, resource_dps_sas_token);
			}else{
				/*MQTT Disconnect DPS*/
				if(0 > wnb303r_MQTT_disconnect(dev_uart_comm,"0"))
				{
					xprintf("Azure IoT DPS Disconnect Send Fail.\n");
				}
			}

#if 0
			int cur_tick0, cur_tick1;
		    /* wnb303r reset. */
			xprintf("**** wnb303r reset ...****\n");
			hx_drv_iomux_set_pmux(IOMUX_PGPIO14, 3);
			hx_drv_iomux_set_outvalue(IOMUX_PGPIO14, 1);
			cur_tick0 = _arc_aux_read(AUX_TIMER0_CNT);
			tx_thread_sleep(1);
			cur_tick1 = _arc_aux_read(AUX_TIMER0_CNT);
			hx_drv_iomux_set_outvalue(IOMUX_PGPIO14, 0);
			xprintf("##### diff_tick = %d\n", cur_tick0);
			xprintf("##### diff_tick = %d\n", cur_tick1);
#endif
			tx_thread_sleep(1);
		}
		break;
		default:
		break;
		}//switch

		//xprintf("*** azure_pnp_iothub_event = %d , azure_iothub_timeout_cnt = %d***\n",azure_pnp_iothub_event,azure_iothub_timeout_cnt);
		if(azure_pnp_iothub_event !=PNP_IOTHUB_CONNECTIING)
			xprintf("*** azure_pnp_iothub_event = %d ***\n",azure_pnp_iothub_event);

//		if(azure_pnp_iothub_event == PNP_IOTHUB_CONNECT_TO_DEVICE_DONE){
//			azure_iothub_timeout_cnt = 0;
//			//xprintf("\n**** PNP_IOTHUB_CONNECT_TO_DEVICE_DONE ****\n");
//			//break;
//		}

		if(azure_iothub_timeout_cnt == AZURE_CONNECT_RETRY_MAX_TIMES)
		{
			xprintf("\n**** AZURE PNP_IOTHUB RECONNECT.... ****\n");
			memset(recv_buf,0,AT_MAX_LEN);//clear buffer
			azure_pnp_iothub_event = PNP_IOTHUB_RECONNECT;
		}
		tx_thread_sleep(1);
	}//while

#ifdef TEST_OK
	/*#############################################################################*/
	xprintf("\n#############################################################################\n");
	xprintf("**** Enter Azure Algorithm Event ****\n");
	xprintf("#############################################################################\n");

	memset(recv_buf,0,AT_MAX_LEN);//clear buffer
	//azure_algo_event = ALGO_EVENT_SEND_IMAGE_TO_CLOUD;
	azure_algo_event = ALGO_EVENT_SEND_RESULT_TO_CLOUD;
	tx_thread_sleep(1);
	while(1)
	{
		io_uart_read(DFSS_UART_0_ID, (uint8_t *) recv_buf, &recv_len, UART_TRANSFER_INT);
		xprintf("[ALGO EVENT] recv_buf:%d %s\n",strlen(recv_buf),recv_buf);
		tx_thread_sleep(1);

		switch (azure_algo_event)
		{
		case ALGO_EVENT_IDLE:
		/* TBD */
		tx_thread_sleep(1);
		break;
		case ALGO_EVENT_SEND_RESULT_TO_CLOUD:
		if(strstr(recv_buf,AT_OK_STR)){
			xprintf("****  Algorithm result to cloud end!! ****\n");
			azure_algo_event = ALGO_EVENT_SEND_IMAGE_TO_CLOUD;//ALGO_EVENT_IDLE;
			/* clear buffer. */
			memset(recv_buf,0,AT_MAX_LEN);
			memset(azure_iothub_publish_msg_ascii,0,SEND_PKG_MAX_SIZE);
			memset(azure_iothub_publish_msg,0,(SEND_PKG_MAX_SIZE*2));

		}else{
			send_algo_result_to_cloud(&algo_human_cnt);
		}
		tx_thread_sleep(1);
		break;
		case ALGO_EVENT_SEND_IMAGE_TO_CLOUD:
			if(azure_iothub_img_send_cnt <SEND_PKG_TIMES){
				if(strstr(recv_buf, AT_OK_STR)!= NULL){
					xprintf("**** Send %d/%d Bytes... ****\n",(azure_iothub_img_send_cnt*SEND_PKG_MAX_SIZE)+SEND_PKG_MAX_SIZE,IMAGE_SIZE);
					++azure_iothub_img_send_cnt;
					azure_iothub_img_send_idx = azure_iothub_img_send_cnt * SEND_PKG_MAX_SIZE;
					/* clear buffer. */
					memset(recv_buf,0,AT_MAX_LEN);
					memset(azure_iothub_publish_msg_ascii,0,SEND_PKG_MAX_SIZE);
					memset(azure_iothub_publish_msg,0,(SEND_PKG_MAX_SIZE*2));
				}else{
					send_algo_img_to_cloud(g_wdma3_img_buf);
				}
			}else{
				/* clear buffer. */
				memset(recv_buf,0,AT_MAX_LEN);
				memset(azure_iothub_publish_msg_ascii,0,SEND_PKG_MAX_SIZE);
				memset(azure_iothub_publish_msg,0,(SEND_PKG_MAX_SIZE*2));

				azure_iothub_img_send_cnt = 0;
				azure_iothub_img_send_idx = 0;

				azure_algo_event = ALGO_EVENT_IDLE;
				xprintf("**** Image send to cloud end!! ****\n");
				tx_thread_sleep(1);
				break;
			}
		tx_thread_sleep(1);
		break;
		case ALGO_EVENT_IOTHUB_RECONNECT:
		/*TBD*/
		tx_thread_sleep(1);
		break;
		default:
		break;
		}

		xprintf("*** azure_algo_event = %d ***\n",azure_algo_event);

		if(azure_atcmd_retry_cnt == AZURE_ATCMD_RETRY_MAX_TIMES)
		{
			azure_algo_event = ALGO_EVENT_IOTHUB_RECONNECT;

		}

		tx_thread_sleep(1);
	}//while
#endif //TEST_OK

#if 0
	//for test
	for(int k = 0;k<SEND_PKG_MAX_SIZE;k++){
		g_wdma3_img_buf[k]='6';
	}

	//while(1)
	{
//		tx_thread_sleep(1);
//		if(0 >wnb303r_MQTT_send_publish_packet(dev_uart_comm, "0", azure_iothub_publish_topic, "1", "0", "0", \
//			azure_iothub_publish_msg_len,azure_iothub_publish_msg))
//		{
//			xprintf(" Azure IoT Hub publish packet Send Fail.\n");
//		}
		if(send_img_to_cloud())
		{
			xprintf("\n**** send image to Azure Cloud ok!!****\n");
		}else{
			xprintf("\n**** send image to Azure Cloud fail.. ****\n");
		}
		tx_thread_sleep(1);
	}//while
#elif 0

	/* azure iothub publish topic. */
	azure_iothub_publish_topic ="devices/" AZURE_IOTHUB_DEVICE_ID "/messages/events/";
	/* devices/weiplus01/messages/events/ */
	xprintf("**** azure_iothub_publish_topic: %s ****\n",azure_iothub_publish_topic);

	char azure_iothub_publish_msg_ascii[SEND_PKG_MAX_SIZE];

	for(int k = 0;k<SEND_PKG_MAX_SIZE;k++){
		g_wdma3_img_buf[k]='9';
	}

	/* send json prefix. */
	/* JSON_RPEFIX
	 * {"human": */
	strcpy(azure_iothub_publish_msg_ascii,AZURE_IOTHUB_PUBLISH_MESSAGE_JSON_PREFIX);
	/* ascii¡@convert to hex string. */
	for (int j = 0; j < strlen(azure_iothub_publish_msg_ascii); j++){
		sprintf((azure_iothub_publish_msg + (j * 2)), "%02X", *(azure_iothub_publish_msg_ascii + j));
	}
	sprintf(azure_iothub_publish_msg_len, "%d", strlen(azure_iothub_publish_msg));
	/* send pkg to azure iothub. */
	if(0 >wnb303r_MQTT_send_publish_packet(dev_uart_comm, "0", azure_iothub_publish_topic, "1", "0", "0", \
				azure_iothub_publish_msg_len,azure_iothub_publish_msg))
	{
		xprintf("JSON prefix send fail.\n");
	}
	/* clear buffer. */
	memset(recv_buf,0,AT_MAX_LEN);//clear buffer
	memset(azure_iothub_publish_msg_ascii,0,SEND_PKG_MAX_SIZE);
	memset(azure_iothub_publish_msg,0,(SEND_PKG_MAX_SIZE*2));

	/* send image data. */
	//for(int i = 0; i<(IMAGE_SIZE/SEND_PKG_MAX_SIZE );i++)
	int send_cnt = 0;
	int img_send_idx = send_cnt * SEND_PKG_MAX_SIZE;

	while(1)//(send_cnt > (IMAGE_SIZE/SEND_PKG_MAX_SIZE))
	{
		io_uart_read(DFSS_UART_0_ID, (uint8_t *) recv_buf, &recv_len, UART_TRANSFER_INT);
		//0228 xprintf("[SEND IMG] recv_buf:%d %s\n",strlen(recv_buf),recv_buf);
		tx_thread_sleep(1);
		if(strstr(recv_buf,AT_OK_STR)!= NULL)
		{
			//xprintf(" **** send_cnt:%d ****\n",send_cnt);//0228+
			++send_cnt;
			img_send_idx = send_cnt * SEND_PKG_MAX_SIZE;
			xprintf(" **** send %d/%d bytes.. ****\n",(send_cnt*256),IMAGE_SIZE);//0228+
			/* clear buffer. */
			memset(recv_buf,0,AT_MAX_LEN);
			memset(azure_iothub_publish_msg_ascii,0,SEND_PKG_MAX_SIZE);
			memset(azure_iothub_publish_msg,0,(SEND_PKG_MAX_SIZE*2));

		}else{
			//0228 board_delay_ms(50);
			//idx = i * SEND_PKG_MAX_SIZE;
			memcpy(azure_iothub_publish_msg_ascii,g_wdma3_img_buf,sizeof(azure_iothub_publish_msg_ascii)); //(g_wdma3_img_buf+idx)

			/* ascii¡@convert to hex string. */
			for (int j = 0; j < strlen(azure_iothub_publish_msg_ascii); j++){
				sprintf((azure_iothub_publish_msg + (j * 2)), "%02X", *(azure_iothub_publish_msg_ascii + j));
			}

			sprintf(azure_iothub_publish_msg_len, "%d", strlen(azure_iothub_publish_msg));

			/* send pkg to azure iothub. */
			if(0 >wnb303r_MQTT_send_publish_packet(dev_uart_comm, "0", azure_iothub_publish_topic, "1", "0", "0", \
					azure_iothub_publish_msg_len,azure_iothub_publish_msg))
			{
				xprintf("image data at send fail.\n",send_cnt);
			}
		}
		//0228 xprintf("**** img_send_idx:%d ****\n",img_send_idx);

		//if(send_cnt >(IMAGE_SIZE/SEND_PKG_MAX_SIZE))
		if(send_cnt < SEND_PKG_TIMES)
		{
			xprintf("Image send to cloud end!!\n");
			break;
		}
		tx_thread_sleep(1);
	}//while

	/* send json suffix*/
	strcpy(azure_iothub_publish_msg_ascii,AZURE_IOTHUB_PUBLISH_MESSAGE_JSON_SUFFIX);
	/* ascii¡@convert to hex string. */
	for (int j = 0; j < strlen(azure_iothub_publish_msg_ascii); j++){
		sprintf((azure_iothub_publish_msg + (j * 2)), "%02X", *(azure_iothub_publish_msg_ascii + j));
	}
	sprintf(azure_iothub_publish_msg_len, "%d", strlen(azure_iothub_publish_msg));
	/* send pkg to azure iothub. */
	if(0 >wnb303r_MQTT_send_publish_packet(dev_uart_comm, "0", azure_iothub_publish_topic, "1", "0", "0", \
			azure_iothub_publish_msg_len,azure_iothub_publish_msg))
	{
		xprintf("JSON suffix send fail.\n");
	}
	/* clear buffer. */
	memset(azure_iothub_publish_msg_ascii,0,SEND_PKG_MAX_SIZE);
	memset(azure_iothub_publish_msg,0,(SEND_PKG_MAX_SIZE*2));
#endif

//	while(1){
//		xprintf("nbiot_service_thread_entry end!!\n");
//		tx_thread_sleep(1);
//	}

#endif //NETWORK_EN
}

/* Define algo send result thread entry.  */
void algo_send_result_thread_entry(ULONG parameter)
{
	unsigned int azure_atcmd_retry_cnt = 0;
	xprintf("**** Enter algo_send_result_thread_entry ****\n");
	/*check DPS and IoTHub connect done.*/
	while(1)
	{
		/* DPS->HUB->Cloud */
		//if(azure_pnp_iotdps_event == PNP_IOTDPS_REGISTRATION_DONE && azure_pnp_iothub_event == PNP_IOTHUB_CONNECTIING ){
		//if(azure_pnp_iothub_event == PNP_IOTHUB_CONNECTIING ){ // HUB->Cloud
		if(azure_pnp_iotdps_event == PNP_IOTDPS_REGISTRATION_DONE && azure_pnp_iothub_event == PNP_IOTHUB_CONNECTIING && azure_algo_event !=ALGO_EVENT_IDLE ){
			break;
		}

		tx_thread_sleep(1);
	}

	/*#############################################################################*/
	xprintf("\n#############################################################################\n");
	xprintf("**** Enter Azure Algorithm Event ****\n");
	xprintf("#############################################################################\n");

	memset(recv_buf,0,AT_MAX_LEN);//clear buffer
	//azure_algo_event = ALGO_EVENT_SEND_IMAGE_TO_CLOUD;
	//azure_algo_event = ALGO_EVENT_IDLE;//ALGO_EVENT_SEND_RESULT_TO_CLOUD;
	tx_thread_sleep(1);
	while(1)
	{
		io_uart_read(DFSS_UART_0_ID, (uint8_t *) recv_buf, &recv_len, UART_TRANSFER_INT);
		//xprintf("[ALGO EVENT] recv_buf:%d %s\n",strlen(recv_buf),recv_buf);
		tx_thread_sleep(1);

		switch (azure_algo_event)
		{
		case ALGO_EVENT_IDLE:
		/* Not do thing. */
		tx_thread_sleep(1);
		break;
		case ALGO_EVENT_SEND_RESULT_TO_CLOUD:
			if(strstr(recv_buf,AT_OK_STR)){
				xprintf("### Algo result to cloud ok!! ###\n");
				//azure_algo_event = ALGO_EVENT_SEND_IMAGE_TO_CLOUD;
				azure_algo_event = ALGO_EVENT_IDLE;
				/* clear buffer. */
				memset(recv_buf,0,AT_MAX_LEN);
				memset(azure_iothub_publish_msg_ascii,0,SEND_PKG_MAX_SIZE);
				memset(azure_iothub_publish_msg,0,(SEND_PKG_MAX_SIZE*2));
			}else{
				send_algo_result_to_cloud(&algo_human_cnt);
			}
		tx_thread_sleep(1);
		break;
		case ALGO_EVENT_SEND_IMAGE_TO_CLOUD:
			if(azure_iothub_img_send_cnt <SEND_PKG_TIMES){
				if(strstr(recv_buf, AT_OK_STR)!= NULL){
					xprintf("### Send %d/%d Bytes... ###\n",(azure_iothub_img_send_cnt*SEND_PKG_MAX_SIZE)+SEND_PKG_MAX_SIZE,IMAGE_SIZE);
					++azure_iothub_img_send_cnt;
					azure_iothub_img_send_idx = azure_iothub_img_send_cnt * SEND_PKG_MAX_SIZE;
					/* clear buffer. */
					memset(recv_buf,0,AT_MAX_LEN);
					memset(azure_iothub_publish_msg_ascii,0,SEND_PKG_MAX_SIZE);
					memset(azure_iothub_publish_msg,0,(SEND_PKG_MAX_SIZE*2));
				}else if(strstr(recv_buf, AT_ERROR_STR)!= NULL){
					/* when data send error, re-send. */
					send_algo_img_to_cloud(g_wdma3_img_buf);
					++azure_atcmd_retry_cnt;
				}
				else{
					send_algo_img_to_cloud(g_wdma3_img_buf);
				}
			}else{
				/* clear buffer. */
				memset(recv_buf,0,AT_MAX_LEN);
				memset(azure_iothub_publish_msg_ascii,0,SEND_PKG_MAX_SIZE);
				memset(azure_iothub_publish_msg,0,(SEND_PKG_MAX_SIZE*2));
				azure_iothub_img_send_cnt = 0;
				azure_iothub_img_send_idx = 0;

				azure_algo_event = ALGO_EVENT_IDLE;
				xprintf("### Image send to cloud ok!! ###\n");
				tx_thread_sleep(1);
				break;
			}
		tx_thread_sleep(1);
		break;
		case ALGO_EVENT_IOTHUB_RECONNECT:
			azure_algo_event 		= ALGO_EVENT_IDLE;
			azure_pnp_iothub_event  = PNP_IOTHUB_RECONNECT;
		tx_thread_sleep(1);
		break;
		default:
		break;
		}//switch

		xprintf("*** azure_algo_event = %d ***\n",azure_algo_event);

		if(azure_atcmd_retry_cnt == AZURE_ATCMD_RETRY_MAX_TIMES)
		{
			azure_atcmd_retry_cnt = 0;
			azure_algo_event = ALGO_EVENT_IOTHUB_RECONNECT;
		}

		tx_thread_sleep(1);
	}//while

}


/* Define cis capture image thread entry.  */
void cis_capture_image_thread_entry(ULONG parameter)
{
	xprintf("**** cis_capture_image_thread_entry ****\n");

	while(1){
		if(azure_pnp_iothub_event == PNP_IOTHUB_CONNECTIING && azure_algo_event == ALGO_EVENT_IDLE)
			break;

		tx_thread_sleep(1);
	}

	xprintf("\n#############################################################################\n");
	xprintf("**** Enter CIS Capture Image ****\n");
	xprintf("#############################################################################\n");

	int img_cnt= 0;
	while(1){
		GetImage(28,28);
		img_cnt++;
		xprintf("### capture image:%d ###\n",img_cnt);
		if(img_cnt == 10)
		{
			azure_algo_event == ALGO_EVENT_SEND_RESULT_TO_CLOUD;
			img_cnt= 0;
		}
		tx_thread_sleep(1);
	}

}

#ifndef SAMPLE_DHCP_DISABLE
static void dhcp_wait()
{
ULONG   actual_status;

    xprintf("DHCP In Progress...\r\n");

    /* Create the DHCP instance.  */
    nx_dhcp_create(&dhcp_0, &ip_0, "DHCP Client");

    /* Start the DHCP Client.  */
    nx_dhcp_start(&dhcp_0);

    /* Wait util address is solved. */
    nx_ip_status_check(&ip_0, NX_IP_ADDRESS_RESOLVED, &actual_status, NX_WAIT_FOREVER);
}
#endif /* SAMPLE_DHCP_DISABLE  */

static UINT dns_create()
{

UINT    status;
ULONG   dns_server_address[3];
//0302UINT    dns_server_address_size = 12;

    /* Create a DNS instance for the Client.  Note this function will create
       the DNS Client packet pool for creating DNS message packets intended
       for querying its DNS server. */
    status = nx_dns_create(&dns_0, &ip_0, (UCHAR *)"DNS Client");
    if (status)
    {
        return(status);
    }

    /* Is the DNS client configured for the host application to create the packet pool? */
#ifdef NX_DNS_CLIENT_USER_CREATE_PACKET_POOL

    /* Yes, use the packet pool created above which has appropriate payload size
       for DNS messages. */
    status = nx_dns_packet_pool_set(&dns_0, ip_0.nx_ip_default_packet_pool);
    if (status)
    {
        nx_dns_delete(&dns_0);
        return(status);
    }
#endif /* NX_DNS_CLIENT_USER_CREATE_PACKET_POOL */

#ifndef SAMPLE_DHCP_DISABLE
    /* Retrieve DNS server address.  */
    nx_dhcp_interface_user_option_retrieve(&dhcp_0, 0, NX_DHCP_OPTION_DNS_SVR, (UCHAR *)(dns_server_address),
                                           &dns_server_address_size);
#else
    dns_server_address[0] = SAMPLE_DNS_SERVER_ADDRESS;
#endif /* SAMPLE_DHCP_DISABLE */

    /* Add an IPv4 server address to the Client list. */
    status = nx_dns_server_add(&dns_0, dns_server_address[0]);
    if (status)
    {
        nx_dns_delete(&dns_0);
        return(status);
    }

    /* Output DNS Server address.  */
    xprintf("DNS Server address: %lu.%lu.%lu.%lu\r\n",
           (dns_server_address[0] >> 24),
           (dns_server_address[0] >> 16 & 0xFF),
           (dns_server_address[0] >> 8 & 0xFF),
           (dns_server_address[0] & 0xFF));

    return(NX_SUCCESS);
}

static UINT unix_time_get(ULONG *unix_time)
{

    /* Using time() to get unix time on x86.
       Note: User needs to implement own time function to get the real time on device, such as: SNTP.  */
	*unix_time = (ULONG)time(NULL);

    return(NX_SUCCESS);
}
#endif
