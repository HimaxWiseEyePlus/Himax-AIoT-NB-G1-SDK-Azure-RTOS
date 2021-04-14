/*
 * bodydetect_meta.h
 *
 *  Created on: 2019¦~12¤ë29¤é
 *      Author: 902447
 */

#ifndef SCENARIO_APP_AIOT_BODYDETECT_ALLON_BODYDETECT_META_H_
#define SCENARIO_APP_AIOT_BODYDETECT_ALLON_BODYDETECT_META_H_

//#include <app_macro_cfg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "embARC.h"
#include "embARC_debug.h"
#include "board_config.h"
#include "sensor_dp_lib.h"
#include "hx_drv_tflm.h"

#define MAX_TRACKED_ALGO_RES  10
#define COLOR_DEPTH	1 // 8bit per pixel FU
typedef enum
{
	MONO_FRAME=0,
	RAWBAYER_FRAME,
	YUV_FRAME
}enum_frameFormat;


typedef struct
{
    uint32_t x;
    uint32_t y;
    uint32_t width;
    uint32_t height;
}struct__box;
typedef struct
{
	struct__box bbox;
    uint32_t time_of_existence;
    uint32_t is_reliable;
}struct_MotionTarget;



typedef struct
{
	struct__box upper_body_bbox;
    uint32_t upper_body_scale;
    uint32_t upper_body_score;
    uint32_t upper_body_num_frames_since_last_redetection_attempt;
    struct__box head_bbox;
    uint32_t head_scale;
    uint32_t head_score;
    uint32_t head_num_frames_since_last_redetection_attempt;
    uint32_t octave;
    uint32_t time_of_existence;
    uint32_t isVerified;
}struct_Human;

typedef struct
{
    int num_hot_pixels ;
    struct_MotionTarget Emt[MAX_TRACKED_ALGO_RES]; ; //ecv::motion::Target* *tracked_moving_targets;
    int frame_count ;
    short num_tracked_moving_targets;
    short num_tracked_human_targets ;
    bool humanPresence ;
    struct_Human ht[MAX_TRACKED_ALGO_RES];  //TrackedHumanTarget* *tracked_human_targets;
    int num_reliable_moving_targets;
    int verifiedHumansExist;
}struct_algoResult;


extern struct_algoResult algo_result;
extern hx_drv_sensor_image_config_t g_pimg_config;
extern uint32_t g_imgsize;
extern unsigned char *g_img_cur_addr_pos;

#endif /* SCENARIO_APP_AIOT_BODYDETECT_ALLON_BODYDETECT_META_H_ */
