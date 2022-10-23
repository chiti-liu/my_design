#include "dy_sv17f.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "driver/uart.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
void dy_sv17f_uart_init()
{
       const uart_config_t uart_config = {
                                        .baud_rate = 9600,
                                        .data_bits = UART_DATA_8_BITS,
                                        .parity = UART_PARITY_DISABLE,
                                        .stop_bits = UART_STOP_BITS_1,
                                        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
                                        .source_clk = UART_SCLK_APB,
                                        };
    // We won't use a buffer for sending data.
    uart_driver_install(UART_NUM_2, 128 * 2, 0, 0, NULL, 0);
    uart_param_config(UART_NUM_2, &uart_config);
    uart_set_pin(UART_NUM_2, 17, 16, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
}
//AA 07 02 曲目高 曲目低 SM  指定曲目
void speak_num(char num)
{
    char buf[7] = {0};
	buf[0] = 0xAA;
	buf[1] = 0x07;
	buf[2] = 0x02;
	buf[3] = 0x00;
	buf[4] = num;
	buf[5] = buf[0] + buf[1] + buf[2] + buf[4];
    buf[6] = 0x00;
	for(int i = 0; i < 7;i++)
            printf("  %x  ",buf[i]);
    uart_write_bytes(UART_NUM_2,buf,7);
}

static void speak_volume(char num)//AA 13 01 14 D2 设置音量为 20 级
{
    char buf[6] = {0};
	buf[0] = 0xAA;
	buf[1] = 0x13;
	buf[2] = 0x01;
	buf[3] = num;
	buf[4] = buf[0] + buf[1] + buf[2] + buf[3];
    buf[5] = 0x00;
	printf("%s\n",buf);
    uart_write_bytes(UART_NUM_2,buf,6);
}

void speak_command(dy_av17f_command cmd,char num)
{
    switch (cmd)
    {
        case SET_VOLUME:
        speak_volume(num);
        break;
        case PLEASE_CMD://请尽量按照规定的标准完成训练任务
        speak_num(105);
        printf("请尽量按照规定的标准完成训练任务\n");
        break;
        case START_CMD:
        speak_num(100);//下面开始训练
        printf("下面开始训练\n");
        //speak_num(num);
        break;
        case LEG_ANGLE://抬腿幅度为
        speak_num(101);
        vTaskDelay(180);
        //vTaskDelay(1000);
        printf("抬腿幅度为\n");
        speak_num(num);
        //vTaskDelay(1000);
        vTaskDelay(100);
        speak_num(106);//度
        break;
        case LEG_COUNT://抬腿次数为
        printf("抬腿次数为\n");
        speak_num(102);
        vTaskDelay(180);
        //        vTaskDelay(1000);
        speak_num(num);
        vTaskDelay(100);
         //       vTaskDelay(1000);
        speak_num(107);//次
        break;
        case LEG_TIME://训练拟定时间为
        printf("训练拟定时间为\n");
        speak_num(103);
        vTaskDelay(210);
        speak_num(num);
        vTaskDelay(100);
        //vTaskDelay(1000);
        speak_num(108);//分钟
        break;
        case END_CMD://辛苦了，训练结束
        printf("辛苦了，训练结束\n");
        speak_num(104);
        // vTaskDelay(1000);
        break;
        case FIX_END://初始腿部姿态校准结束  
        speak_num(109);
        break;
        default:
        break;
    }
    vTaskDelay(100);
}
