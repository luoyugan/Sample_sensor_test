#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

// 字符串最大长度
#define STRING_MAXSIZE              128
// 定义版本号
#define SOFTWARE_VERSION            "V1.0"

int16_t ret;

// 打印信息，用于打印普通信息
#define PRINT_INFO(fmt, args...)    printf("%s, %s, %d, info: "fmt, __FILE__, __func__, __LINE__, ##args)
// 打印信息，用于打印错误信息
#define PRINT_ERROR(fmt, args...)   printf("%s, %s, %d, error: "fmt, __FILE__, __func__, __LINE__, ##args)







#endif