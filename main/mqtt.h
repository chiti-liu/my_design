#ifndef _MQTT_H_
#define _MQTT_H_

#include "mpu6050.h"
#include <stdbool.h>
void send_to_aliyuniot(float get_x_angle);
void mqtt_app_start(void);
bool send_to_aliyun_results(leg_test_results result_data);
bool send_to_aliyun_chart(leg_test_results result_data);

#endif
