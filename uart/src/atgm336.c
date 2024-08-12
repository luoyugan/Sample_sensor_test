#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include "osal_time.h"
#include "hdf_log.h"
#include "uart_if.h"                 // UART标准接口头文件
#include "common.h"


// 串口端口号
static uint32_t m_uart_port = 0;
// 串口波特率
static uint32_t m_uart_baudrate = 9600;


void printArray(uint8_t arr[], int size) {
    char str[size + 1]; 
    for (int i = 0; i < size; i++) {
        str[i] = arr[i]; 
    }
    str[size] = '\0'; 
    printf("%s\n", str);
}


int main(int argc, char* argv[])
{
    DevHandle handle = NULL;
    struct UartAttribute attribute;
    int32_t ret = 0;
    uint8_t rbuff[STRING_MAXSIZE] = { 0 };
    

    attribute.dataBits = UART_ATTR_DATABIT_8;   // UART传输数据位宽，一次传输7个bit
    attribute.parity = UART_ATTR_PARITY_NONE;   // UART传输数据无校检
    attribute.stopBits = UART_ATTR_STOPBIT_1;   // UART传输数据停止位为1位
    attribute.rts = UART_ATTR_RTS_DIS;          // UART禁用RTS
    attribute.cts = UART_ATTR_CTS_DIS;          // UART禁用CTS
    attribute.fifoRxEn = UART_ATTR_RX_FIFO_EN;  // UART使能RX FIFO
    attribute.fifoTxEn = UART_ATTR_TX_FIFO_EN;  // UART使能TX FIFO

    handle = UartOpen(m_uart_port);
    if (handle == NULL) {
        PRINT_ERROR("UartOpen: open uart port %u failed!\n", m_uart_port);
        return -1;
    }
    PRINT_ERROR("UartOpen successful and uart port = %d\n", m_uart_port);

    // 设置UART波特率
    ret = UartSetBaud(handle, m_uart_baudrate);
    if (ret != HDF_SUCCESS) {
        PRINT_ERROR("UartSetBaud: set baud failed, ret %d\n", ret);
        goto ERR;
    }
    PRINT_ERROR("UartSetBaud successful and uart baudrate = %d\n", m_uart_baudrate);

    // 设置UART设备属性
    ret = UartSetAttribute(handle, &attribute);
    if (ret != HDF_SUCCESS) {
        PRINT_ERROR("UartSetAttribute: set attribute failed, ret %d\n", ret);
        goto ERR;
    }
    PRINT_ERROR("UartSetAttribute successful\n");

    // 获取UART设备属性
    ret = UartGetAttribute(handle, &attribute);
    if (ret != HDF_SUCCESS) {
        PRINT_ERROR("UartGetAttribute: get attribute failed, ret %d\n", ret);
        goto ERR;
    }
    PRINT_ERROR("UartGetAttribute successful\n");

    // 设置UART传输模式为非阻塞模式
    // ret = UartSetTransMode(handle, UART_MODE_RD_NONBLOCK);
    // 设置UART传输模式为阻塞模式
    ret = UartSetTransMode(handle, UART_MODE_RD_BLOCK);
    if (ret != HDF_SUCCESS) {
        PRINT_ERROR("UartSetTransMode: set trans mode failed, ret %d\n", ret);
        goto ERR;
    }
    PRINT_ERROR("UartSetTransMode successful\n");

    // 向UART设备写入数据
    // ret = UartWrite(handle, wbuff, (uint32_t)strlen((char *)wbuff));
    // if (ret != HDF_SUCCESS) {
    //     PRINT_ERROR("UartWrite: write data failed, ret %d\n", ret);
    //     goto ERR;
    // }
    // PRINT_ERROR("UartWrite successful and wbuff = %s\n", wbuff);

    // 从UART设备读取5字节的数据
    while(1) {
        ret = UartRead(handle, rbuff, STRING_MAXSIZE);
        OsalMDelay(1000);
        // if (ret < 0) {
        //     PRINT_ERROR("UartRead: read data failed, ret %d\n", ret);
        //     goto ERR;
        // }
        // PRINT_ERROR("UartRead successful and rbuff = %s\n", rbuff);
        printArray(rbuff, STRING_MAXSIZE);
    }

ERR:
    // 销毁UART设备句柄
    UartClose(handle);
    return ret;
}
