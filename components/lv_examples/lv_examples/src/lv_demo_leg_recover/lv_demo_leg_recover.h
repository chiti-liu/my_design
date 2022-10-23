/**
 * @file lv_demo_leg_recover.h
 *
 */

#ifndef LV_DEMO_LEG_RECOVER_H
#define LV_DEMO_LEG_RECOVER_H
#include "driver/uart.h"
#include <stdio.h>
#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/
void lv_demo_leg_recover(void);

/**********************
 *      MACROS
 **********************/
//训练预设参数

extern  uint8_t count_sum;

extern uint8_t angle_per;
extern uint32_t time_sum;//预设时间总和
extern volatile bool test_start_flag;
extern volatile bool test_end_flag;
#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*LV_DEMO_KEYPAD_ENCODER_H*/
