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
#include <unistd.h> //使用到usleep()进程挂起函数
// 重新定义结构体方便使用

#define STRING_MAXSIZE 128
#define WRITE_FLAGS 0
#define READ_FLAGS  1
#define BUSID       1 //I2C总线
//定义命令发送函数
uint8_t regData[6L] = {0};
//定义发送命令
uint8_t cmdBuf[2] = {0x0B, 0x24};
uint8_t readbuffer[128] = {0};
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

//定义命令发送函数
int32_t SendCMD(DevHandle handle) {
    uint8_t ADDR = 0x44 << 1;
    int32_t ret;
    struct I2cMsg msgs[2];
    uint8_t msgLen = 2;
    // cmdBuf[0] = command >> 8L; //将命令拆分成高低位分别保存
    // cmdBuf[1] = command & 0xFF;
    uint8_t value[6];

    msgs[0].len = 2;
    msgs[0].addr = ADDR;
    msgs[0].flags = 0;
    msgs[0].buf = cmdBuf;

    msgs[1].len = sizeof(value);
    msgs[1].addr = ADDR;
    msgs[1].flags = 1;
    msgs[1].buf = value;
    ret = I2cTransfer(handle,msgs,msgLen);
    if(ret != 2){
        printf("%d: SendCommend faided",ret);
        printI2cMsg(msgs);
        // free(i2cMessage.i2cMsg);
        // return -1;
    }
    printf("%d: SendCommend successful-2",ret);
    printf("value = [%d]", value[1]);
    usleep(1000L * 1000L); //等待发送完成
    return 1;
}

// int32_t SendCMD1(DevHandle handle,uint16_t command)
// {
//     int32_t ret;
//     struct I2cMsg msgs[2]; 
//     uint8_t cmdBuf[2L] = {0};

//     cmdBuf[0] = command >> 8L; //将命令拆分成高低位分别保存
//     cmdBuf[1] = command & 0xFF;

//     msgs[0].len = 2L;
//     msgs[0].addr = ADDR;
//     msgs[0].flags = WRITE_FLAGS;
//     msgs[0].buf = cmdBuf;
    
//     ret = I2cTransfer(handle,msgs,msgs[0].len);
//     if(ret < 0){
//         printf(" SendCommend faided");
//         return -1;
//     }
//     usleep(50L * 1000L); //等待发送完成
//     return 1;
// }

int main(int argc, char* argv[]) {
    printf( "temp sensor enter\n\r");

    // int32_t ret = 0;
    DevHandle i2cHandle = NULL;
    // uint8_t ADDR = 0x44;
    uint16_t value = 0;
    // I2cMessage i2cMessage;
    i2cHandle = I2cOpen(BUSID);
    if(i2cHandle == NULL){
        printf("get handle failed");
        I2cClose(i2cHandle);
        return 0;
    }
    // SendCMD(i2cHandle,0x3093); //关闭reset命令
    // SendCMD(i2cHandle,0x2C06); //发送读取命令
    SendCMD(i2cHandle); //发送命令 repeatability=Low mps=0.5
    /**
     * 数据处理
    */
    // i2cMessage.i2cMsg = (struct I2cMsg *)malloc(sizeof(struct I2cMsg)*2); //申请内存
    // i2cMessage.i2cMsg[0].len = 2;
    // i2cMessage.i2cMsg[0].addr = ADDR;
    // i2cMessage.i2cMsg[0].flags = 1;
    // i2cMessage.i2cMsg[0].buf = readbuffer;
    // ret = I2cTransfer(i2cHandle,i2cMessage.i2cMsg,4);
    // if(ret < 0){
    //     printf("%d: SendCommend faided",ret);
    //     printI2cMsg(i2cMessage.i2cMsg);
    //     // free(i2cMessage.i2cMsg);
    //     // return -1;
    // }
    // printf("%d: SendCommend successful-2",ret);
    // free(i2cMessage.i2cMsg); //释放内存


    value = readbuffer[0] << 8;
    value = value | readbuffer[1];
    printf("Temperature: %.2f C\n",175.0f * (float)value / 65535.0f - 45.0f);
    value = 0;
    value = readbuffer[3] << 8;
    value = value | readbuffer[4];
    printf("Humidity: %.2f H\n",100.0f * (float)value / 65535.0f);
      /**
     * 关闭设备
    */    
    I2cClose(i2cHandle);
    return 0;
}