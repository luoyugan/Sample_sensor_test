#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>

#include "hdf_log.h"
#include "i2c_if.h"                 // I2C标准接口头文件


// 字符串最大长度
// #define STRING_MAXSIZE              128
static uint8_t IMU_flags = 0; //读标记----1

// 10轴 IMU 通讯协议 写格式
// 1.包头
uint8_t IMU_head_number1 = 0;
uint8_t IMU_head_number2 = 0;
// 2.寄存器地址
static uint8_t IMU_reg_addr = 0;
// 3.数据低八位
uint8_t IMU_data_l = 0;
// 4.数据高八位-----IMU_data=(short)((short)IMU_data_h<<8|IMU_data_l
uint8_t IMU_data_h = 0;


// i2c控制器序号
static uint16_t m_i2c_number = 0;
// i2c从设备地址
static uint16_t m_i2c_slave_address = 0x50;
// i2c寄存器地址
// static uint8_t m_i2c_reg_address = 0;
// // i2c寄存器数值
// static uint8_t m_i2c_reg_value = 0;
// i2c数据长度，用于读操作，默认是1个字节
static uint16_t m_i2c_read_data_length = 1;
// i2c模式标记
// static uint16_t m_i2c_flags_read = 1;
static uint16_t m_i2c_flags_addr_10bit = 0;
static uint16_t m_i2c_flags_read_no_ack = 0;
static uint16_t m_i2c_flags_ignore_no_ack = 0;
static uint16_t m_i2c_flags_no_start = 0;

static void printI2cMsg(struct I2cMsg* i2cMsg) {
    printf("I2cMsg Information:\n");
    printf("Length: %x\n", i2cMsg->len);
    printf("Address: %x\n", i2cMsg->addr);
    printf("Flags: %x\n", i2cMsg->flags);
    printf("Buffer: ");
    for (int i = 0; i < i2cMsg->len; i++) {
        printf("%02X ", i2cMsg->buf[i]);
    }
    printf("\n");
}

void main_help(char *cmd)
{
    printf("%s: platform device i2c\n", cmd);
    printf("Version: %s\n", "jy901b");
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
    int i = 1; // Start from the second argument (skip program name)
    while (i < argc) {
        char *optarg = argv[i];
        char *endptr;
        long value = strtol(optarg, &endptr, 16); // Convert hexadecimal string to long

        if (*endptr != '\0') {
            printf("Invalid hexadecimal input: %s\n", optarg);
            exit(1);
        }

        switch (i) {
            case 1:
                IMU_head_number1 = (uint8_t)value;
                break;
            case 2:
                IMU_head_number2 = (uint8_t)value;
                break;
            case 3:
                IMU_reg_addr = (uint8_t)value;
                break;
            case 4:
                IMU_data_l = (uint8_t)value;
                break;
            case 5:
                IMU_data_h = (uint8_t)value;
                break;
            case 6:
                IMU_flags = (uint8_t)value;
                break;
            case 7:
                m_i2c_flags_addr_10bit = (uint8_t)value;
                break;
            case 8:
                m_i2c_flags_read_no_ack = (uint8_t)value;
                break;
            case 9:
                m_i2c_flags_ignore_no_ack = (uint8_t)value;
                break;
            case 10:
                m_i2c_flags_no_start = (uint8_t)value;
                break;
            default:
                main_help(argv[0]);
                exit(0);
                break;
        }

        i++;
    }

}
void set_unlock(DevHandle handle) {
    uint16_t ret = 0;
    uint8_t msgs_count = 2;
    struct I2cMsg msgs_unlock[2]; 
    msgs_unlock[0].addr = m_i2c_slave_address;
    msgs_unlock[0].flags = 0;
    uint8_t wbuff1[1] = {0x69};
    msgs_unlock[0].len = 1;
    msgs_unlock[0].buf = wbuff1;
    
    msgs_unlock[1].addr = m_i2c_slave_address;
    msgs_unlock[1].flags = 0;
    uint8_t wbuff2[2] = {0x88, 0xB5};
    msgs_unlock[1].len = 2;
    msgs_unlock[1].buf = wbuff2;
    ret = I2cTransfer(handle, msgs_unlock, msgs_count);
    if (ret != msgs_count) {
        printf("I2cTransfer(write) failed and ret = %d\n", ret);
    }
    printI2cMsg(msgs_unlock);
    printI2cMsg(msgs_unlock+1);
    usleep(100000);
}


int main(int argc, char* argv[])
{
    DevHandle handle = NULL;
    int32_t ret = 0;
    struct I2cMsg msgs[2];      // 消息结构体数组
    int16_t msgs_count = 0;
    // buff定义
    uint8_t wbuff[128] = { 0 };
    uint8_t wbuff_1[1] = { 0 };
    uint8_t wbuff_2[2] = { 0 };
    uint8_t readbuff[128] = { 0 };


    // 解析参数
    parse_opt(argc, argv);

    // 打开i2c控制器
    handle = I2cOpen(m_i2c_number);
    if (handle == NULL) {
        printf("I2cOpen failed\n");
        return -1;
    }
    if(IMU_head_number1 == 0xFF) {
        if (IMU_head_number2 == 0xAA) {
            if (IMU_flags == 1) {
                // 读操作
                // 设置msgs数组有效数目
                msgs_count = 2;
                // 初始化msgs[0]，该部分为主设备发送从设备的i2c内容
                msgs[0].addr = m_i2c_slave_address;
                msgs[0].flags = 0;
                msgs[0].len = 1;
                wbuff[0] = IMU_reg_addr;          
                msgs[0].buf = wbuff;
                // 初始化msgs[1]，该部分为主设备读取从设备发送的i2c内容
                msgs[1].addr = m_i2c_slave_address;
                msgs[1].flags = IMU_flags;
                msgs[1].len = m_i2c_read_data_length;
                msgs[1].buf = readbuff;
                // i2c数据传输，传输次数为2次
                ret = I2cTransfer(handle, msgs, msgs_count);
                if (ret != msgs_count) {
                    printf("I2cTransfer(read) failed and ret = %d\n", ret);
                    goto out;
                }
                printf("I2cTransfer success and read data length = %d\n", strlen((char *)readbuff));
                for (uint32_t i = 0; i < strlen((char *)readbuff); i++) {
                    printf("rbuff[%d] = 0x%x\n", i, readbuff[i]);
                }
                goto out;
            } else {
                printf("enter i2c write = [%x]!\n", IMU_flags);
                printf("IMU_reg_addr = 0x%x\n", IMU_reg_addr);
                printf("IMU_data_l = 0x%x\n", IMU_data_l);
                printf("IMU_data_h = 0x%x\n", IMU_data_h);
                set_unlock(handle);
                // 写操作
                // 设置msgs数组有效数目
                msgs_count = 2;
                // 初始化msgs[0]，该部分为主设备发送从设备的i2c内容
                msgs[0].addr = m_i2c_slave_address;
                msgs[0].flags = IMU_flags;
                msgs[0].len = 1;
                wbuff_1[0] = IMU_reg_addr;
                msgs[0].buf = wbuff_1;

                msgs[1].addr = m_i2c_slave_address;
                msgs[1].flags = IMU_flags;

                wbuff_2[0] = IMU_data_l;
                wbuff_2[1] = IMU_data_h;
                msgs[1].len = 2;
                msgs[1].buf = wbuff_2;

                printI2cMsg(msgs);
                printI2cMsg(msgs+1);
                
                ret = I2cTransfer(handle, msgs, msgs_count);
                if (ret != msgs_count) {
                    printf("I2cTransfer(write) failed and ret = %d\n", ret);
                    goto out;
                }
                printf("I2cTransfer success and write reg=[%d], datal=[%d],datah=[%d],len=[%d]\n", IMU_reg_addr, IMU_data_l, IMU_data_h,msgs[0].len);
            }
        } else {
            printf("protocols head2 is not 0xAA!\n");
            goto out;
        }
    } else {
        printf("protocols head1 is not 0xFF!\n");
        goto out;
    }
    return ret;
out:
    memset(readbuff, 0 , sizeof(readbuff));
    // 关闭i2c控制器
    I2cClose(handle);
   
    return ret;
}
