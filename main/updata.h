#ifndef _UP_
#define _UP_
#include <stdio.h>
#include <stdlib.h>
#include <string.h>



void save_web_data(char *data);
int save_nvs(const char * str,char * data);
int read_nvs(const char * str,char * data);
#endif