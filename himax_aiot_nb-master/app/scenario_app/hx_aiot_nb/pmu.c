#include <hx_aiot_nb/hii.h>
#include <hx_aiot_nb/pmu.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "hx_drv_pmu.h"
#include "hx_drv_tflm.h"





PMU_WAKEUPEVENT_E wakeup_event;

static char wakeup_event_str[][MAX_STRING]={
	{"[0] : PS_SD : wakeup by (i2c_fsm_sd_all_trig || ext_force_pmu_allon)"},
	{"[1]: PS_SD wakeup by ext_int_nxt_all(GPIO)"},
	{"[2]: PS_SD wakeup by REG_CPU_SCENARIO=4'b0001&S_EXT_INT"},
	{"[3]: PS_SLP2 wakeup by i2c_fsm_slp2_all_trig || ext_force_pmu_allon"},
	{"[4]: PS_SLP2 wakeup by ext_int_nxt_all(GPIO)"},
	{"[5]: PS_SLP2 wakeup by REG_CPU_SCENARIO=4'b0010&S_EXT_INT"},
	{"[6]: PS_SLP2 wakeup by REG_CPU_SCENARIO=4'b0010& RTC_timer"},
	{"[7]: PS_SLP2 wakeup by REG_CPU_SCENARIO=4'b0010& RTC_ADC_timer_int"},
	{"[8]: PS_SLP1 wakeup by i2c_fsm_slp1_all_trig || ext_force_pmu_allon"},
	{"[9]: PS_SLP1 wakeup by ext_int_nxt_all(GPIO)"},
	{"[10]: PS_SLP1 wakeup by (Sensor_EXT_INT && CPU3 && SC4)||(RTC_timer_int && CPU3 && SC5)"},
	{"[11]: PS_SLP1 wakeup by (RTC_ADC_timer_int && CPU3 && (SC4 ||  SC5)"},
	{"[12]: PS_EXP wakeup by i2c_fsm_exp_all_trig || ext_force_pmu_allon || pmu_wdg_timeout_int)"},
	{"[13]: PS_EXP wakeup by ((CPU8 && ADCC_int) ||(CPU12 && ~pmu_sen_active && ADCC_int))"},
	{"[14]: PS_CAP wakeup by i2c_fsm_cap_all_trig || ext_force_pmu_allon || pmu_wdg_timeout_int"},
	{"[15]: PS_CAP wakeup by WRDMAx_abnormal_int"},
	{"[16]: PS_CAP wakeup by DP_ABNORMAL_INT"},
	{"[17]: PS_CAP wakeup by (CPU4 && SC_nframe_end && all_cfg_xDMAx_int && DP_CDM_MOTION_INT)"},
	{"[18]: PS_CAP wakeup by(CPU12 && SC_nframe_end && all_cfg_xDMAx_int && DP_CDM_MOTION_INT && pmu_adc_active && ACC_int)"},
	{"[19]: PS_CAP wakeup by (CPU12 && SC_nframe_end && all_cfg_xDMAx_int && DP_CDM_MOTION_INT && pmu_adc_active && ADC_timeout && ~ADCC_int)"},
	{"[20]: PS_CAP wakeup by (CPU12 && SC_nframe_end && all_cfg_xDMAx_int && DP_CDM_MOTION_INT && ~pmu_adc_active)"},
	{"[21]: PS_CAP wakeup by (CPU12 && SC_nframe_end && all_cfg_xDMAx_int && ~DP_CDM_MOTION_INT && ADCC_int)"},
};


void print_wakeup_event(PMU_WAKEUPEVENT_E param_event)
{

	dbg_printf(DBG_LESS_INFO,"param_event=0x%x\r\n", param_event);
	if((param_event & 0x1)  != 0)
	{
		dbg_printf(DBG_LESS_INFO,"event=%s\r\n", wakeup_event_str[0]);
	}
	if((param_event & 0x2) != 0)
	{
		dbg_printf(DBG_LESS_INFO,"event=%s\r\n", wakeup_event_str[1]);
	}
	if((param_event & 0x4) != 0)
	{
		dbg_printf(DBG_LESS_INFO,"event=%s\r\n", wakeup_event_str[2]);
	}
	if((param_event & 0x8) != 0)
	{
		dbg_printf(DBG_LESS_INFO,"event=%s\r\n", wakeup_event_str[3]);
	}
	if((param_event & 0x10) != 0)
	{
		dbg_printf(DBG_LESS_INFO,"event=%s\r\n", wakeup_event_str[4]);
	}
	if((param_event & 0x20) != 0)
	{
		dbg_printf(DBG_LESS_INFO,"event=%s\r\n", wakeup_event_str[5]);
	}
	if((param_event & 0x40) != 0)
	{
		dbg_printf(DBG_LESS_INFO,"event=%s\r\n", wakeup_event_str[6]);
	}
	if((param_event & 0x80) != 0)
	{
		dbg_printf(DBG_LESS_INFO,"event=%s\r\n", wakeup_event_str[7]);
	}
	if((param_event & 0x100) != 0)
	{
		dbg_printf(DBG_LESS_INFO,"event=%s\r\n", wakeup_event_str[8]);
	}
	if((param_event & 0x200) != 0)
	{
		dbg_printf(DBG_LESS_INFO,"event=%s\r\n", wakeup_event_str[9]);
	}
	if((param_event & 0x400) != 0)
	{
		dbg_printf(DBG_LESS_INFO,"event=%s\r\n", wakeup_event_str[10]);
	}
	if((param_event & 0x800) != 0)
	{
		dbg_printf(DBG_LESS_INFO,"event=%s\r\n", wakeup_event_str[11]);
	}
	if((param_event & 0x1000) != 0)
	{
		dbg_printf(DBG_LESS_INFO,"event=%s\r\n", wakeup_event_str[12]);
	}
	if((param_event & 0x2000) != 0)
	{
		dbg_printf(DBG_LESS_INFO,"event=%s\r\n", wakeup_event_str[13]);
	}
	if((param_event & 0x4000) != 0)
	{
		dbg_printf(DBG_LESS_INFO,"event=%s\r\n", wakeup_event_str[14]);
	}
	if((param_event & 0x8000) != 0)
	{
		dbg_printf(DBG_LESS_INFO,"event=%s\r\n", wakeup_event_str[15]);
	}
	if((param_event & 0x10000) != 0)
	{
		dbg_printf(DBG_LESS_INFO,"event=%s\r\n", wakeup_event_str[16]);
	}
	if((param_event & 0x20000) != 0)
	{
		dbg_printf(DBG_LESS_INFO,"event=%s\r\n", wakeup_event_str[17]);
	}
	if((param_event & 0x40000) != 0)
	{
		dbg_printf(DBG_LESS_INFO,"event=%s\r\n", wakeup_event_str[18]);
	}
	if((param_event & 0x80000) != 0)
	{
		dbg_printf(DBG_LESS_INFO,"event=%s\r\n", wakeup_event_str[19]);
	}
	if((param_event & 0x100000) != 0)
	{
		dbg_printf(DBG_LESS_INFO,"event=%s\r\n", wakeup_event_str[20]);
	}
	if((param_event & 0x200000) != 0)
	{
		dbg_printf(DBG_LESS_INFO,"event=%s\r\n", wakeup_event_str[21]);
	}

}


void app_convert_wkeuppin_for_pmu_mask(uint16_t *io_mask , int pin_cnt)
{
	HX_DRV_GPIO_IOMUX  wakeupCPU_int_pin[MAX_SUPPORT_WAKEUP_CPU_INT_PIN];
	/*
	 * [0]: ext_pint0
	 * [1]: ext_pint1
	 * [2]: ext_pint2
	 * [3]: ext_pint3
	 * [4]: ext_pint4
	 * [5]: ext_pint5
	 * [6]: ext_pint6
	 * [7]: ext_sint0
	 * [8]: ext_sint1
	 *
	 * */
	*io_mask = 0x1FF;
	for(uint8_t idx = 0; idx < pin_cnt; idx++)
	{
		if(wakeupCPU_int_pin[idx] != WE1AppCfg_GPIO_IOMUX_NONE)
		{
			if(wakeupCPU_int_pin[idx] == WE1AppCfg_GPIO_IOMUX_PGPIO0)
			{
				xprintf("G 0\n");
				*io_mask = *io_mask & 0x1FE;
			}
			if(wakeupCPU_int_pin[idx] == WE1AppCfg_GPIO_IOMUX_PGPIO1)
			{
				xprintf("G 1\n");
				*io_mask = *io_mask & 0x1FD;
			}
			if(wakeupCPU_int_pin[idx] == WE1AppCfg_GPIO_IOMUX_PGPIO2)
			{
				xprintf("G 2\n");
				*io_mask = *io_mask & 0x1FB;
			}
			if(wakeupCPU_int_pin[idx] == WE1AppCfg_GPIO_IOMUX_PGPIO3)
			{
				xprintf("G 3\n");
				*io_mask = *io_mask & 0x1F7;
			}
			if(wakeupCPU_int_pin[idx] == WE1AppCfg_GPIO_IOMUX_PGPIO4)
			{
				xprintf("G 4\n");
				*io_mask = *io_mask & 0x1EF;
			}
			if(wakeupCPU_int_pin[idx] == WE1AppCfg_GPIO_IOMUX_PGPIO5)
			{
				xprintf("G 5\n");
				*io_mask = *io_mask & 0x1DF;
			}
			if(wakeupCPU_int_pin[idx] == WE1AppCfg_GPIO_IOMUX_PGPIO6)
			{
				xprintf("G 6\n");
				*io_mask = *io_mask & 0x1BF;
			}
			if(wakeupCPU_int_pin[idx] == WE1AppCfg_GPIO_IOMUX_SGPIO0)
			{
				xprintf("G 7\n");
				*io_mask = *io_mask & 0x17F;
			}
			if(wakeupCPU_int_pin[idx] == WE1AppCfg_GPIO_IOMUX_SGPIO1)
			{
				xprintf("G 8\n");
				*io_mask = *io_mask & 0xFF;
			}
		}
	}
}






