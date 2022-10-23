
#ifndef __MPU6050_H_
#define __MPU6050_H_

#include <stdio.h>
#define TXD_PIN (26)
#define RXD_PIN (25)
#define BUF_SIZE  128

typedef struct SAcc
{
	short a[3];
	short T;
}SAcc;
typedef struct SGyro
{
	short w[3];
	short T;
}SGyro;
typedef struct SAngle
{
	short Angle[3];
	short T;
}SAngle;
/*
typedef struct Leg_Rehalibitation_Data{
   	SAcc Acc;
	SGyro Gyro;
	SAngle All_Angle;//float  0.1
}Leg_Rehalibitation_Data;
 */
typedef struct test_per_data
{
	uint8_t start_time;
	uint8_t end_time;
	uint8_t test_time;
	float test_angle; //%0.1f
	float prv_angle;
	float cur_angle;
	//float start_angle;
	float end_angle;
	struct test_data * next_data;
}test_per_data;//每一步训练所得数据

typedef struct leg_test_results
{
    uint8_t prv_count;
    uint8_t prv_per_angle;
    uint8_t lv_test_time[100];
	uint32_t prv_time_sum;
	uint32_t test_time_sum;
    float leg_freq;
	float prv_leg_freq;
    float lv_test_angle[100];
}leg_test_results;

extern uint8_t test_count;
extern leg_test_results test_results;
void mpu6050_uart_init();
#endif
