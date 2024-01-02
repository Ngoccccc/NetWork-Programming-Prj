#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "util.h"

int meltMsg(char* msg, char* result[]){
    int msg_len = strlen(msg);
    int ele_count = 0;
    char buff[100];
    int buff_idx = 0;
    for(int i = 0; i <= msg_len; i++){
        if(msg[i] == '-' || msg[i] == '\0'){
            buff[buff_idx] = '\0';
            result[ele_count] = (char*) malloc(100);
            strcpy(result[ele_count], buff);
            ele_count++;
            buff_idx = 0;
        } else {
            buff[buff_idx++] = msg[i];
        }
    }
    return ele_count;
}

