#include <stdio.h>
#include "stdlib.h"
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include "hdf_base.h"
#include "hdf_io_service_if.h"
#include "osal_time.h"
#include "gpio/gpio_service.h"   
#include "gpio_if.h"


uint16_t gpio3_clk = 96; 
uint16_t gpio3_dio = 98;  
uint16_t out = 1;
uint8_t write_high = {1};
unsigned char SigNum[6]={0xbf, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f};


void start() {
    GpioSetDir(gpio3_clk, out);
    GpioSetDir(gpio3_dio, out);
    GpioWrite(gpio3_dio, 1);
    GpioWrite(gpio3_clk, 1);
    OsalUDelay(2);
    GpioWrite(gpio3_dio, 0);
    OsalUDelay(2);
}

void writebyte(uint8_t byte) {
    uint8_t i;
    GpioSetDir(gpio3_clk, out);
    GpioSetDir(gpio3_dio, out);
    for(i = 0; i < 8; i++) {
        GpioWrite(gpio3_clk, 0);
        if(byte & 0x01) {
            GpioWrite(gpio3_dio, 1);  
        } else {
            GpioWrite(gpio3_dio, 0);  
        }
        OsalUDelay(3);
        byte = byte >> 1;
        GpioWrite(gpio3_clk, 1);
        OsalUDelay(3);
    }
}

void stop() {
    GpioSetDir(gpio3_clk, out);
    GpioSetDir(gpio3_dio, out);
    GpioWrite(gpio3_clk, 0);
    OsalUDelay(2);
    GpioWrite(gpio3_dio, 0);  
    OsalUDelay(2);
    GpioWrite(gpio3_clk, 1);
    OsalUDelay(2);
    GpioWrite(gpio3_dio, 1);  
}

void wait_ask() {
    GpioSetDir(gpio3_clk, out);
    GpioWrite(gpio3_clk, 0);
    OsalUDelay(5);
    GpioWrite(gpio3_clk, 1);
    OsalUDelay(2);
    GpioWrite(gpio3_clk, 0);
}

int main(int argc, char* argv[]) {
    uint8_t i = 0;
    printf("enter TM1637 test\n");
    GpioSetDir(gpio3_clk, out);
    GpioSetDir(gpio3_dio, out);
    // 自动地址增加
    start();
    writebyte(0x40);
    wait_ask();
    stop();
    // 设置起始地址
    start();
    writebyte(0xc0);
    wait_ask();
    // 写数据
    for(i = 0; i < 6; i ++)
	{
		writebyte(SigNum[i]);
		wait_ask();
	}
    // printf("write data\n");
    // 开显示
    stop();
    start();
    writebyte(0x8f);
    wait_ask();
    stop();

    return 1;
}