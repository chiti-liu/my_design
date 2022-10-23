#include "mpu6050.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "driver/uart.h"
#include "esp_system.h"
#include <string.h>
#include <stdio.h>
#include "mqtt.h"
#include "mpu6050.h"
#include "D:\ESP32\leg\components\lv_examples\lv_examples\src\lv_demo_leg_recover\lv_demo_leg_recover.h"
#include "dy_sv17f.h"
#include "esp_timer.h"
#define LEG_UP true
#define LEG_DOWN false

SAcc Acc;
SGyro Gyro;
SAngle All_Angle;
uint8_t test_count = 0;
test_per_data *every_test_data = NULL;
test_per_data *first_data = NULL;
leg_test_results test_results ;

static bool next_record = false;
static bool up_down_flag = LEG_UP;
static  esp_timer_handle_t periodic_timer;
static uint8_t timer_time = 0; 
static uint8_t YAWCMD[3] = {0XFF,0XAA,0X52};//0轴校准
static uint8_t ACCCMD[3] = {0XFF,0XAA,0X67};//加速度校准
static uint8_t SLEEPCMD[3] = {0XFF,0XAA,0X60};//休眠
static uint8_t VERTICALCMD[3] = {0xFF,0xAA,0x66};//垂直安装
static uint8_t UARTMODECMD[3] = {0XFF,0XAA,0X61};//uart通讯
static uint8_t IICMODECMD[3] = {0XFF,0XAA,0X62};///IIC通讯


static float get_x_angle = 0;//抬腿是逆时针，所以需要转换
static float get_y_angle = 0;
static float get_z_angle = 0;
static void sendcmd(uint8_t *cmd);
static void mpu6050_server_task(void *pvParameters);
static void CopeSerial2Data(uint8_t ucData);
 static void periodic_timer_callback(void* arg);

TaskHandle_t task_mpu6050_server;           //创建任务句柄

void mpu6050_uart_init()
{
       const uart_config_t uart_config = {
                                        .baud_rate = 115200,
                                        .data_bits = UART_DATA_8_BITS,
                                        .parity = UART_PARITY_DISABLE,
                                        .stop_bits = UART_STOP_BITS_1,
                                        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
                                        .source_clk = UART_SCLK_APB,
                                        };
    // We won't use a buffer for sending data.
    uart_driver_install(UART_NUM_1, BUF_SIZE * 2, 0, 0, NULL, 0);
    uart_param_config(UART_NUM_1, &uart_config);
    uart_set_pin(UART_NUM_1, TXD_PIN, RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

         /* 
      * 创建计时器：
      * 1.每0.5秒运行的一个周期性计时器，并且打印消息
      */
 
     //创建周期性计时器
     const esp_timer_create_args_t periodic_timer_args = {
             .callback = &periodic_timer_callback,
             //自定义名称
             .name = "periodic"
     };
 
     ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));
     ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, 1000000));//1秒

    xTaskCreatePinnedToCore(mpu6050_server_task,"mpu6050_server_task",4096*6,NULL,7,&task_mpu6050_server,1);
}

//用串口2给JY模块发送指令
static void sendcmd(uint8_t *cmd)
{
		uart_write_bytes(UART_NUM_1,(const char *)cmd,3);
}
static void send_to_aliyuntask(void *pvParameters)
{
    while(1)
    {
     vTaskDelay(1000);
     if(send_to_aliyun_results(test_results)==false)
     continue;

     if(send_to_aliyun_chart(test_results)==false)
     continue;
     else  vTaskSuspend(NULL); 
    }
    vTaskDelete(NULL);//删除任务本身
}
static void mpu6050_server_task(void *pvParameters)
{
    /*
    ** 对数据进行处理
    */
    test_count = 0;
    static int rxBytes = 0;
    static bool get_new = true;
    static bool get_fix = true;
    static uint8_t data[BUF_SIZE] = {0};
    every_test_data = ((test_per_data *)malloc(sizeof(test_per_data)));
    first_data = every_test_data;
    every_test_data->prv_angle = 0;
    every_test_data->cur_angle = 0;
    static float angle_up_fix = 0;//消抖处理
    static float angle_down_fix = 0;    
            printf("正在进行加速度校准\r\n");
			sendcmd(ACCCMD);//等待模块内部自动校准好，模块内部会自动计算需要一定的时间
			printf("加速度校准完成\r\n");
			vTaskDelay(1000/ portTICK_PERIOD_MS);
			printf("进行Z轴角度清零\r\n");
			sendcmd(YAWCMD);
			printf("Z轴角度清零完成\r\n");
            vTaskDelay(1000/ portTICK_PERIOD_MS);
            printf("进行垂直放置\r\n");
            sendcmd(VERTICALCMD);
            vTaskDelay(1000/ portTICK_PERIOD_MS);
    while(1)
    {
        vTaskDelay(140/ portTICK_PERIOD_MS);
        rxBytes = 0;
        rxBytes = uart_read_bytes(UART_NUM_1, (uint8_t *)data, 50, 200 / portTICK_RATE_MS);
        if (rxBytes > 0) 
        {
            data[rxBytes] = '\0';
            //printf("recv:  ");
            printf("-----------------------------------\r\n");
            for(int m=0;m<rxBytes;m++)
            CopeSerial2Data(data[m]);
            //printf("%x ",data[m]);
           // printf("\r\n");
            //输出加速度
            //串口接受到的数据已经拷贝到对应的结构体的变量中了，根据说明书的协议，以加速度为例 stcAcc.a[0]/32768*16就是X轴的加速度，
           // printf("Acc:%.3f %.3f %.3f\r\n",(float)Acc.a[0]/32768*16,(float)Acc.a[1]/32768*16,(float)Acc.a[2]/32768*16);
            //输出角速度
            //printf("Gyro:%.3f %.3f %.3f\r\n",(float)Gyro.w[0]/32768*2000,(float)Gyro.w[1]/32768*2000,(float)Gyro.w[2]/32768*2000);
            if((get_x_angle == 0)&&(get_y_angle == 0)&&(get_z_angle == 0) )
            {
                printf("not get angle\n");
                continue;
            }
            //输出角度
            printf("Angle:%.3f %.3f %.3f\r\n",get_x_angle,get_y_angle,get_z_angle);
            //vTaskDelay(10/ portTICK_PERIOD_MS);
            send_to_aliyuniot(get_x_angle);//这里可以不加  最后再加  按次数显示，不要实时 后面测一下会不会拖累性能
            //vTaskDelay(10/ portTICK_PERIOD_MS);
             printf("free_heap_size = %d\n", esp_get_free_heap_size());
            if(get_fix == true)//说明没校准完   最好加个语音？
            {
                if((get_x_angle > 1)||(get_x_angle < -1)||(get_y_angle > 1)||(get_y_angle < -1)||(get_z_angle > 1)||(get_z_angle < -1))
                {
                    vTaskDelay(100/ portTICK_PERIOD_MS);
                    printf("fix again\n");
                    //sendcmd(ACCCMD);//等待模块内部自动校准好，模块内部会自动计算需要一定的时间
                    continue ;
                }
                else 
                {
                    speak_command(FIX_END,NULL);
                    get_fix = false;//校准完了
                }
            }
            //最好创建个新任务来获取,在这之前收取到新数据
            if(test_start_flag == true )//开始算出抬腿幅度  每次抬腿时间   抬腿次数  抬腿频率
            {
                angle_up_fix = (float)angle_per*4/5;//消抖处理
                angle_down_fix = (float)angle_per*2/5;    
                printf("angle_up_fix = %.1f, angle_down_fix = %.1f\n",angle_up_fix,angle_down_fix);
                 //定义链表存储数据
                 printf("get_x_angle = %.1f\n",get_x_angle);
                 every_test_data->cur_angle = get_x_angle;
                 if(get_new == true) 
                 {
                    printf("test_nextning\n");
                    //every_test_data->start_angle =every_test_data->cur_angle;
                    every_test_data->start_time = timer_time;
                    get_new = false;
                 }
                 if(up_down_flag == LEG_UP)//上升过程   处理过程中做次数计算
                 {
                     if(every_test_data->cur_angle >= every_test_data->prv_angle)
                    {
                        printf("test_up\n");
                        every_test_data->prv_angle = every_test_data->cur_angle;
                        continue ;
                    }
                    else if((every_test_data->cur_angle <= every_test_data->prv_angle) && (every_test_data->cur_angle < angle_up_fix))
                    {
                        printf("腿部在上升过程中抖动");
                        continue ;
                    }
                    else //抬到顶点，至于顶点的消抖处理交给下降过程，上升过程中的顶点就是最高点
                    {
                        printf("test_max\n");
                        every_test_data->end_angle = every_test_data->prv_angle;
                        up_down_flag = LEG_DOWN;
                        continue ;
                    }
                 }
                else //下降过程
                {
                    if(every_test_data->cur_angle <= every_test_data->prv_angle)
                    {
                        printf("test_down\n");
                        every_test_data->prv_angle = every_test_data->cur_angle;
                        continue ;
                    }
                    else if((every_test_data->cur_angle >= every_test_data->prv_angle)&&(every_test_data->cur_angle >angle_down_fix))
                    {
                        printf("腿部在下降过程中抖动");
                        continue ;
                    }
                    else
                    {
                        test_count ++;//实测次数加
                        speak_num(test_count);
                        printf("--------------------\n");
                        printf("get new ,test count is %d\n",test_count);
                        every_test_data->end_time = timer_time;
                        printf("time is %d\n",timer_time);
                        timer_time = 0;//计时器归零

                        every_test_data->test_time = every_test_data->end_time - every_test_data->start_time;
                        every_test_data->test_angle = every_test_data->end_angle;
                        
                        test_per_data * every_new_data = (test_per_data *)malloc(sizeof(test_per_data));
                        every_test_data->next_data = every_new_data;
                        every_test_data = every_test_data->next_data;
                        every_test_data->prv_angle = get_x_angle;
                        printf("free_heap_size = %d\n", esp_get_free_heap_size());
                        printf("--------------------\n");
                        get_new = true;
                        up_down_flag = LEG_UP;
                    }
                }

                if(test_count == count_sum)
                    test_end_flag = true;

                if(test_end_flag == true)//导出数据
                {
                    memset(test_results.lv_test_angle,0,99);
                    memset(test_results.lv_test_time,0,99);
                    //显示
                    printf("test_endding\n");
                    //send_to_appear(first_data);
                    get_new = false;
                    test_start_flag = false;
                    speak_command(END_CMD,NULL);
                         
                    for(int i = 0; i < count_sum; i ++)
                    {
                        test_results.lv_test_angle[i] = first_data->test_angle;
                        test_results.lv_test_time[i] = first_data->test_time;
                        test_results.test_time_sum += test_results.lv_test_time[i];
                        first_data = first_data->next_data;
                    }
                    test_results.leg_freq = (float)(test_results.test_time_sum)/(float)count_sum;//实测


                    test_results.prv_leg_freq = (float)time_sum/(float)count_sum;
                    test_results.prv_time_sum = time_sum;
                    test_results.prv_count = count_sum;
                    test_results.prv_per_angle = angle_per;
                    //发给阿里云
    
                    xTaskCreatePinnedToCore(send_to_aliyuntask,"send_to_aliyuntask",4096,NULL,8,NULL,1);
                }
            }

            //uart_write_bytes(UART_NUM_1, data, rxBytes);
            #if 0
            if(Deal_upperRx_Data(data,rxBytes) == true)  //命令正确
            {
                upper_cmd_t cmd = (upper_cmd_t) upper_rx.Function;
                switch(cmd)
                {
                    case Write_Device_Cmd:
                        break;
                    case Write_Tcp_Ip_Cmd:

                        break;
                    case Write_Tcp_Port_Cmd:

                        break;
                    case Test_Cmd:

                        break;
                    default:
                        break;
                }
            }
            else
            continue;
            #endif
        }
    }
    vTaskDelete(NULL);//删除任务本身
    //ESP_ERROR_CHECK(esp_timer_stop(periodic_timer));
    //ESP_ERROR_CHECK(esp_timer_delete(periodic_timer));
}
 //1秒周期性计时器回调函数
 static void periodic_timer_callback(void* arg)
 {
     timer_time ++;
     //int64_t time_since_boot = esp_timer_get_time();
     //ESP_LOGI(TAG, "Periodic timer called, time since boot: %lld us", time_since_boot);
 }

static void CopeSerial2Data(uint8_t ucData)
{
	static uint8_t ucRxBuffer[BUF_SIZE];
	static uint8_t ucRxCnt = 0;	
	
	ucRxBuffer[ucRxCnt++]=ucData;	//将收到的数据存入缓冲区中
	if (ucRxBuffer[0]!=0x55) //数据头不对，则重新开始寻找0x55数据头
	{
		ucRxCnt=0;
		return;
	}
	if (ucRxCnt<11) {return;}//数据不满11个，则返回
	else
	{
        for(int i =0 ;i< 11; i++)
        printf("%x\t",ucRxBuffer[i]);
		switch(ucRxBuffer[1])//判断数据是哪种数据，然后将其拷贝到对应的结构体中，有些数据包需要通过上位机打开对应的输出后，才能接收到这个数据包的数据
		{
			//memcpy为编译器自带的内存拷贝函数，需引用"string.h"，将接收缓冲区的字符拷贝到数据结构体里面，从而实现数据的解析。
			//case 0x51:	memcpy(&(Acc),&ucRxBuffer[2],8);break;
			//case 0x52:	memcpy(&(Gyro),&ucRxBuffer[2],8);break;
			case 0x53:	
            memcpy(&(All_Angle),&ucRxBuffer[2],8);
            get_x_angle = (float)All_Angle.Angle[0]/32768*180;
            get_x_angle = - get_x_angle;//抬腿是逆时针，所以需要转换
            get_y_angle = (float)All_Angle.Angle[1]/32768*180;
            get_z_angle = (float)All_Angle.Angle[2]/32768*180;
            break;
            default:
            break;
		}
		ucRxCnt=0;//清空缓存区
	}
}


