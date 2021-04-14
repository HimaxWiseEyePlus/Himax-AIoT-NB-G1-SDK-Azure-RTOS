#include <hx_aiot_nb/hii.h>
#include <hx_aiot_nb/pmu.h>
#include <hx_aiot_nb/tflitemicro_algo.h>
#include "spi_master_protocol.h"
#include "hx_drv_tflm.h"
#include "hx_drv_pmu.h"
#include "powermode.h"
#include "azure_sphere.h"
#include "tx_api.h"

//datapath boot up reason flag
volatile uint8_t g_bootup_md_detect = 0;
//#define SPI_SEND
//#define ENABLE_PMU

#define IMAGE_HEIGHT 480
#define IMAGE_WIDTH 640
#define IMAGE_POOL_SIZE 640*480
struct_algoResult algo_result;
uint32_t g_imgsize;
unsigned char *g_img_cur_addr_pos;

#ifdef SPI_SEND
static int open_spi()
{
	int ret ;
#ifndef SPI_MASTER_SEND
	ret = hx_drv_spi_slv_open();
	hx_drv_uart_print("SPI slave ");
#else
	ret = hx_drv_spi_mst_open();
	hx_drv_uart_print("SPI master ");
#endif
    return ret;
}

static int spi_write(uint32_t addr, uint32_t size, SPI_CMD_DATA_TYPE data_type)
{
#ifndef SPI_MASTER_SEND
	return hx_drv_spi_slv_protocol_write_simple_ex(addr, size, data_type);
#else
	return hx_drv_spi_mst_protocol_write_sp(addr, size, data_type);
#endif
}
#endif

hx_drv_sensor_image_config_t g_pimg_config;
static bool is_initialized = false;
GetImage(int image_width,int image_height, int channels) {
	int ret = 0;
	//static bool is_initialized = false;
	xprintf("is_initialized : %d \n",is_initialized);
	  if (!is_initialized) {
	    if (hx_drv_sensor_initial(&g_pimg_config) != HX_DRV_LIB_PASS) {
	    	xprintf("hx_drv_sensor_initial error\n");
	      return ERROR;
	    }
#ifdef SPI_SEND
	    if (hx_drv_spim_init() != HX_DRV_LIB_PASS) {
	      return ERROR;
	    }
	    ret = open_spi();
#endif
	    is_initialized = true;
	  }

	  //capture image by sensor
	  hx_drv_sensor_capture(&g_pimg_config);

	  g_img_cur_addr_pos = (unsigned char *)g_pimg_config.jpeg_address;
	  g_imgsize = g_pimg_config.jpeg_size;
	  xprintf("g_pimg_config.jpeg_address:0x%x size : %d \n",g_pimg_config.jpeg_address,g_pimg_config.jpeg_size);
#ifdef SPI_SEND
	  //send jpeg image data out through SPI
	  ret = spi_write(g_pimg_config.jpeg_address, g_pimg_config.jpeg_size, DATA_TYPE_JPG);
	  //ret = spi_write(g_pimg_config.raw_address, g_pimg_config.raw_size, DATA_TYPE_RAW_IMG);
#endif
	  return OK;
}


#ifdef ENABLE_PMU
int GetWakeUpEvent() {
	PMU_WAKEUPEVENT_E wakeup_event;
	hx_drv_pmu_get_ctrl(PMU_WAKEUP_CPU_EVT_PMU, &wakeup_event);
	print_wakeup_event(wakeup_event);
	if(((wakeup_event & PMU_WAKEUP_SD_EXTGPIO) != 0)
				|| ((wakeup_event & PMU_WAKEUP_SD_S_EXT_INT) != 0))
	{
		g_bootup_md_detect = 1;
	}else{
		g_bootup_md_detect = 0;
	}
}

void EnterToPMU(uint32_t sleep_ms){
	uint16_t io_mask;
	PM_CFG_T aCfg;
	sensordplib_stop_capture();
	sensordplib_start_swreset();
	sensordplib_stop_swreset_WoSensorCtrl();
	dbg_printf(DBG_LESS_INFO,"PMU io_mask=0x%x\n",io_mask);
	app_convert_wkeuppin_for_pmu_mask(&io_mask);
	aCfg.mode = PM_MODE_RTC;
	aCfg.sensor_rtc = sleep_ms;
	aCfg.pmu_timeout = sleep_ms;
	aCfg.adc_rtc = 0;
	aCfg.adc_timeout = 0;
	aCfg.powerplan = PMU_WE1_POWERPLAN_INTERNAL_LDO;
	aCfg.io_mask = io_mask;
	aCfg.bootromspeed =PMU_BOOTROMSPEED_PLL_400M_50M;
	aCfg.s_ext_int_mask = 0;		//s_ext_int_mask
	aCfg.iccm_retention = 0;		/**< Only Support in PM_MODE_AOS_ADC_BOTH, PM_MODE_AOS_ONLY, PM_MODE_ADC_ONLY**/
	aCfg.dccm_retention = 0;		/**< Only Support in PM_MODE_AOS_ADC_BOTH, PM_MODE_AOS_ONLY, PM_MODE_ADC_ONLY**/
	aCfg.xccm_retention = 0;		/**< Only Support in PM_MODE_AOS_ADC_BOTH, PM_MODE_AOS_ONLY, PM_MODE_ADC_ONLY**/
	aCfg.yccm_retention = 0;		/**< Only Support in PM_MODE_AOS_ADC_BOTH, PM_MODE_AOS_ONLY, PM_MODE_ADC_ONLY**/
	aCfg.skip_bootflow = 0;
	aCfg.support_bootwithcap = 0;
	aCfg.mclk_alwayson = 0;
	aCfg.disable_xtal24M = 0;
	aCfg.peripheral_pad_as_input = 1;
	uint32_t version;
	PMU_ERROR_E pmu_err;
	pmu_err = hx_drv_pmu_get_ctrl(PMU_CHIP_VERSION, &version);
	if((version == PMU_CHIP_VERSION_A) || (version == PMU_CHIP_VERSION_B))
	{
		aCfg.flash_pad_high = 0;
	}else{
		aCfg.flash_pad_high = 1;
	}
	dbg_printf(DBG_LESS_INFO,"mclk_on=%d\n",aCfg.mclk_alwayson);
	aCfg.support_debugdump = 0;
	aCfg.ultra_lowpower = 1;
	dbg_printf(DBG_LESS_INFO,"Enter PMU Start\n");
	hx_lib_pm_mode_set(aCfg);
}
#endif

int img_cnt = 0;

void tflitemicro_start() {
	GetImage(IMAGE_WIDTH,IMAGE_HEIGHT,1);

#ifdef TFLITE_MICRO_GOOGLE_PERSON
	xprintf("tflitemicro_algo_run\n");
	img_cnt++;
	xprintf("### img_cnt:%d ###\n",img_cnt);
	tflitemicro_algo_run(g_pimg_config.raw_address, g_pimg_config.img_width, g_pimg_config.img_height, &algo_result);
	xprintf("humanPresence %d frame_count %d\n",algo_result.humanPresence,algo_result.frame_count);
	xprintf("det_box_x:%d\ndet_box_y:%d\ndet_box_width:%d\ndet_box_height:%d\n",algo_result.ht[0].upper_body_bbox.x,\
	algo_result.ht[0].upper_body_bbox.y, algo_result.ht[0].upper_body_bbox.width, \
	algo_result.ht[0].upper_body_bbox.height);
#endif

	if(img_cnt == 1){
		//azure_active_event = ALGO_EVENT_SEND_RESULT_TO_CLOUD;
		azure_active_event = ALGO_EVENT_SEND_IMAGE_TO_CLOUD;
		img_cnt= 0;
#ifdef ENABLE_PMU
		xprintf("EnterToPMU()\n");
		EnterToPMU(10000);
#endif
	}
}

void setup(){
	int ret = 0;

#ifdef TFLITE_MICRO_GOOGLE_PERSON
	xprintf("tflitemicro_algo_init\n");
	tflitemicro_algo_init();
#endif

#ifdef NB_IOT_BOARD
	/*Azure TX Task. */
	xprintf("#### nbiot_task_define ####\n");
	nbiot_task_define();
#endif

}

hx_drv_sensor_image_config_t g_pimg_config;
void hx_aiot_nb()
{
	//EnterToPMU(10000); //for measure power

	xprintf("setup\n");
	setup();
}

