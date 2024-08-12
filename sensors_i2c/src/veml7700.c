#include <stdio.h>
#include "stdlib.h"
#include "hdf_base.h"
#include "hdf_log.h"
#include "hdf_sbuf.h"

#include "hdf_io_service_if.h"
#include "sensor_if.h"
#include "sensor_type.h"
#include "osal_mem.h"
#include "osal_time.h"
#include "i2c_if.h"
#include "common.h"

#include <unistd.h> //使用到usleep()进程挂起函数
// VEML7700的I2C地址
#define VEML7700_I2C_ADDR 0x10 // 7位I2C地址加上写入操作位（0x10或0xAE）
// VEML7700寄存器地址
#define ALS_DATA_REGISTER 0x04 // 光照度高分辨率输出值寄存器地址

uint8_t wbuff[STRING_MAXSIZE] = { 0 };
uint8_t rbuff[STRING_MAXSIZE] = { 0 };
uint16_t als_value;

void printI2cMsg(struct I2cMsg* i2cMsg) {
    printf("I2cMsg Information:\n");
    printf("Length: %u\n", i2cMsg->len);
    printf("Address: %u\n", i2cMsg->addr);
    printf("Flags: %u\n", i2cMsg->flags);
    printf("Buffer: ");
    for (int i = 0; i < i2cMsg->len; i++) {
        printf("%02X ", i2cMsg->buf[i]);
    }
    printf("\n");
}

uint16_t read_lux(DevHandle handle) {
    struct I2cMsg msgs[2];
    // 读操作
    // 设置msgs数组有效数目
    uint16_t msgs_count = 2;
    // 初始化msgs[0]，该部分为主设备发送从设备的i2c内容
    msgs[0].addr = VEML7700_I2C_ADDR;
    msgs[0].flags = 0;
    msgs[0].len = 1;
    wbuff[0] = ALS_DATA_REGISTER;          
    msgs[0].buf = wbuff;
    // 初始化msgs[1]，该部分为主设备读取从设备发送的i2c内容
    msgs[1].addr = VEML7700_I2C_ADDR;
    msgs[1].flags = 1;
    msgs[1].len = 2;
    msgs[1].buf = rbuff;
    // i2c数据传输，传输次数为2次
    ret = I2cTransfer(handle, msgs, msgs_count);
    if (ret < 0) {
        printf("I2cTransfer(read) failed and ret = %d\n", ret);
        return -1;
    }
    printf("I2cTransfer success and read data length = %lu\n", strlen((char *)rbuff));
    for (uint32_t i = 0; i < strlen((char *)rbuff); i++) {
        printf("rbuff[%d] = 0x%x\n", i, rbuff[i]);
    }
    return 1;
}

int main(int argc, char* argv[]) {
    DevHandle i2cHandle = NULL;
    i2cHandle = I2cOpen(1);
    if(i2cHandle == NULL){
        printf("get handle failed\n");
        I2cClose(i2cHandle);
        return 0;
    }
    ret = read_lux(i2cHandle);
    if(ret < 0) {
        printf("read lux failed\n");
        I2cClose(i2cHandle);
        return 0;
    }
    // 组合两个字节为16位的光感值
    als_value = ((uint16_t)rbuff[1] << 8) | rbuff[0];
    printf("ALS Value: %u\n", als_value);
    memset(rbuff, 0 , sizeof(rbuff));
    I2cClose(i2cHandle);
    return 0;
}