#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>

#include "hdf_log.h"
#include "i2c_if.h"                 // I2C标准接口头文件
#include "common.h"

// i2c控制器序号
static uint16_t m_i2c_number = 0;
// i2c从设备地址
static uint16_t m_i2c_slave_address = 0;
// i2c寄存器地址
static uint8_t m_i2c_reg_address = 0;
// i2c寄存器数值
static uint8_t m_i2c_reg_value = 0;
static uint8_t m_i2c_reg_value_h = 0;
// i2c数据长度，用于读操作，默认是1个字节
static uint16_t m_i2c_read_data_length = 1;
// i2c模式标记
static uint16_t m_i2c_flags_read = 1;
static uint16_t m_i2c_flags_addr_10bit = 0;
static uint16_t m_i2c_flags_read_no_ack = 0;
static uint16_t m_i2c_flags_ignore_no_ack = 0;
static uint16_t m_i2c_flags_no_start = 0;
static uint16_t m_i2c_flags_stop = 0;

///////////////////////////////////////////////////////////


/***************************************************************
* 函数名称: toI2cFlags
* 说    明: 将各个标记转化为i2c flags
* 参    数: 
*       @read           是否是读，0表示写，1表示读
*       @addr_10_bit    是否是10-bit从设备地址，0表示8bit，1表示10-bit
*       @read_no_ack    读操作是否没有ack，0表示有ack，1表示没有ack
*       @ignore_no_ack  是否忽略ack，0表示有ack，1表示没有ack
*       @no_start       是否有开始条件，0表示有，1表示没有
*       @stop           是否有结束条件，0表示有，1表示没有
* 返 回 值: 无
***************************************************************/
static uint16_t toI2cFlags(uint16_t read, uint16_t addr_10_bit,
                            uint16_t read_no_ack, uint16_t ignore_no_ack,
                            uint16_t no_start, uint16_t stop)
{
    uint16_t i2c_flags = 0;

    if (read) {
        i2c_flags |= I2C_FLAG_READ;
    }

    if (addr_10_bit) {
        i2c_flags |= I2C_FLAG_ADDR_10BIT;
    }

    if (read_no_ack) {
        i2c_flags |= I2C_FLAG_READ_NO_ACK;
    }

    if (ignore_no_ack) {
        i2c_flags |= I2C_FLAG_IGNORE_NO_ACK;
    }

    if (no_start) {
        i2c_flags |= I2C_FLAG_NO_START;
    }

    if (stop) {
        i2c_flags |= I2C_FLAG_STOP;
    }

    return i2c_flags;
}


/***************************************************************
* 函数名称: main_help
* 说    明: 帮助文档
* 参    数: 无
* 返 回 值: 无
***************************************************************/
void main_help(char *cmd)
{
    printf("%s: platform device i2c\n", cmd);
    printf("Version: %s\n", SOFTWARE_VERSION);
    printf("%s [options]...\n", cmd);
    printf("    -n, --number            i2c bus number\n");
    printf("    -a, --address           i2c slave address\n");
    printf("    -r, --reg_address       i2c reg address\n");
    printf("    -v, --reg_value         i2c reg value\n");
    printf("    -l, --length            the length of i2c read data from slave device\n");
    printf("    -W, --write             i2c write data to slave device. default(read)\n");
    printf("    -B, --addr_10bit        i2c address is 10bit\n");
    printf("    -R, --read_no_ack       No-Ack read\n");
    printf("    -I, --ignore_no_ack     No START condition\n");
    printf("    -S, --stop              STOP condition\n");
    printf("    -h, --help              help info\n");
    printf("\n");
}


/***************************************************************
* 函数名称: parse_opt
* 说    明: 解析参数
* 参    数: 
*           @argc:  参数数量
*           @argv:  参数变量数组
* 返 回 值: 无
***************************************************************/
void parse_opt(int argc, char *argv[])
{
    while (1) {
        struct option long_opts[] = {
            { "number",         required_argument,  NULL, 'n' },
            { "address",        required_argument,  NULL, 'a' },
            { "length",         required_argument,  NULL, 'l' },
            { "reg_address",    required_argument,  NULL, 'r' },
            { "reg_value",      required_argument,  NULL, 'v' },
            { "write",          no_argument,        NULL, 'W' },
            { "addr_10bit",     no_argument,        NULL, 'B' },
            { "read_no_ack",    no_argument,        NULL, 'R' },
            { "ignore_no_ack",  no_argument,        NULL, 'I' },
            { "stop",           no_argument,        NULL, 'S' },
            { "help",           no_argument,        NULL, 'h' },
        };
        int option_index = 0;
        int c;

        c = getopt_long(argc, argv, "n:a:l:r:v:WBRISh", long_opts, &option_index);
        if (c == -1) break;

        switch (c) {
            case 'n':
                m_i2c_number = (uint16_t)atoi(optarg);
                break;
            
            case 'a':
                m_i2c_slave_address = (uint16_t)atoi(optarg);
                break;

            case 'l':
                m_i2c_read_data_length = (uint16_t)atoi(optarg);
                break;
            
            case 'r':
                m_i2c_reg_address = (uint8_t)atoi(optarg);
                break;

            case 'v':
                m_i2c_reg_value = (uint8_t)atoi(optarg);
                break;

            case 'W':
                m_i2c_flags_read = 0;
                break;

            case 'B':
                m_i2c_reg_value_h = (uint8_t)atoi(optarg);
                break;
            
            case 'R':
                m_i2c_flags_read_no_ack = 1;
                break;

            case 'I':
                m_i2c_flags_ignore_no_ack = 1;
                break;

            case 'S':
                m_i2c_flags_no_start = 1;
                break;
                
            case 'h':
            default:
                main_help(argv[0]);
                exit(0);
                break;
        }
    }
}


/***************************************************************
* 函数名称: main
* 说    明: 主函数，用于i2c设备打开、读取和关闭
* 参    数: 
*           @argc:  参数数量
*           @argv:  参数变量数组
* 返 回 值: 0为成功，反之为错误
***************************************************************/
int main(int argc, char* argv[])
{
    DevHandle handle = NULL;
    int32_t ret = 0;
    struct I2cMsg msgs[2];      // 消息结构体数组
    int16_t msgs_count = 0;
    uint8_t wbuff[STRING_MAXSIZE] = { 0 };
    uint8_t rbuff[STRING_MAXSIZE] = { 0 };

    // 解析参数
    parse_opt(argc, argv);
    printf("i2c number:                 %d\n", m_i2c_number);
    printf("i2c slave address:          %d\n", m_i2c_slave_address);
    printf("i2c reg address:            %d\n", m_i2c_reg_address);
    printf("i2c reg value:              %d\n", m_i2c_reg_value);
    printf("i2c read data length:       %d\n", m_i2c_read_data_length);
    printf("i2c flags read:             %d\n", m_i2c_flags_read);
    printf("i2c flags addr 10bit:       %d\n", m_i2c_flags_addr_10bit);
    printf("i2c flags read no ack:      %d\n", m_i2c_flags_read_no_ack);
    printf("i2c flags ignore no ack:    %d\n", m_i2c_flags_ignore_no_ack);
    printf("i2c flags no start:         %d\n", m_i2c_flags_no_start);
    printf("i2c flags stop:             %d\n", m_i2c_flags_stop);

    // 打开i2c控制器
    handle = I2cOpen(m_i2c_number);
    if (handle == NULL) {
        printf("I2cOpen failed\n");
        return -1;
    }

    if (m_i2c_flags_read == 1) {
        // 读操作
        // 设置msgs数组有效数目
        msgs_count = 2;
        // 初始化msgs[0]，该部分为主设备发送从设备的i2c内容
        msgs[0].addr = m_i2c_slave_address;
        // msgs[0].flags = toI2cFlags(0, m_i2c_flags_addr_10bit, m_i2c_flags_read_no_ack, m_i2c_flags_ignore_no_ack, m_i2c_flags_no_start, m_i2c_flags_stop);
        msgs[0].flags = 0;
        msgs[0].len = 1;
        wbuff[0] = m_i2c_reg_address;           // 本案例的i2c从设备是第1字节是寄存器地址
        msgs[0].buf = wbuff;
        // 初始化msgs[1]，该部分为主设备读取从设备发送的i2c内容
        msgs[1].addr = m_i2c_slave_address;
        // msgs[1].flags = toI2cFlags(1, m_i2c_flags_addr_10bit, m_i2c_flags_read_no_ack, m_i2c_flags_ignore_no_ack, m_i2c_flags_no_start, m_i2c_flags_stop);
        msgs[1].flags = 1;
        msgs[1].len = m_i2c_read_data_length;
        msgs[1].buf = rbuff;
        printf("m_i2c_slave_address i2c write = [%x]!\n", m_i2c_slave_address);
        printf("m_i2c_reg_address = 0x%x\n", m_i2c_reg_address);
        printf("IMU_data_l = 0x%x\n", wbuff[0]);
        printf("IMU_data_h = 0x%x\n", rbuff[1]);
        // i2c数据传输，传输次数为2次
        ret = I2cTransfer(handle, msgs, msgs_count);
        if (ret != msgs_count) {
            printf("I2cTransfer(read) failed and ret = %d\n", ret);
            goto out;
        }
        printf("I2cTransfer success and read data length = %d\n", strlen((char *)rbuff));
        for (uint32_t i = 0; i < strlen((char *)rbuff); i++) {
            printf("rbuff[%d] = 0x%x\n", i, rbuff[i]);
        }
        goto out;
    } else {
        // 写操作
        // 设置msgs数组有效数目
        msgs_count = 1;
        // 初始化msgs[0]，该部分为主设备发送从设备的i2c内容
        msgs[0].addr = m_i2c_slave_address;
        msgs[0].flags = toI2cFlags(0, m_i2c_flags_addr_10bit, m_i2c_flags_read_no_ack, m_i2c_flags_ignore_no_ack, m_i2c_flags_no_start, m_i2c_flags_stop);
        msgs[0].len = 2;
        wbuff[0] = m_i2c_reg_address;       // 本案例的i2c从设备是第1字节是寄存器地址
        wbuff[1] = m_i2c_reg_value;         // 本案例的i2c从设备是第2字节是寄存器数值

        msgs[0].buf = wbuff;
        // i2c数据传输，传输次数为2次
        ret = I2cTransfer(handle, msgs, msgs_count);
        if (ret != msgs_count) {
            printf("I2cTransfer(write) failed and ret = %d\n", ret);
            goto out;
        }

        printf("I2cTransfer success and write reg(%d), data(%d)\n", m_i2c_reg_address, m_i2c_reg_value);
    }
    return ret;

out:
    memset(rbuff, 0 , sizeof(rbuff));
    // 关闭i2c控制器
    I2cClose(handle);
   
    return ret;
}
