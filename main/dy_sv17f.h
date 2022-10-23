#ifndef DY_SV17F_H
#define  DY_SV17F_H

typedef enum{
             SET_VOLUME = 0,//设置声音
             PLEASE_CMD,//请尽量按照规定的标准完成训练任务
             START_CMD,//训练开始语音
             LEG_ANGLE,//抬腿幅度语音
             LEG_COUNT,//抬腿次数语音
             LEG_TIME,//训练总时间语音
             END_CMD,//训练结束语音
             FIX_END//校准结束语音
            }dy_av17f_command;

void dy_sv17f_uart_init();
void speak_num(char num);
void speak_command(dy_av17f_command cmd,char num);
#endif
