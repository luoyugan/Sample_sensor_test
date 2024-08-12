#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include "hdf_log.h"
#include "hdf_sbuf.h"
#include "hdf_io_service_if.h"

// 打印信息标签
#define PRINT_ERROR(fmt, args...)       printf("%s, %s, %d, error: "fmt, __FILE__, __func__, __LINE__, ##args)
#define PRINT_INFO(fmt, args...)        printf("%s, %s, %d, info: "fmt, __FILE__, __func__, __LINE__, ##args)

// 定义驱动名称
#define SAMPLE_SERVICE_NAME             "rk3568_sample_driver"
// 定义读写内容
#define SAMPLE_WRITE_READ               123

// 重复尝试的标记变量
static int g_replyFlag = 0;


/***************************************************************
 * 函数名称: OnDevEventReceived
 * 说    明: 驱动上报事件的回调函数
 * 参    数: 
 *      @priv       priv指向绑定到此侦听器的私有数据的指针
 *      @id         id发生驱动事件的序列号
 *      @data       data驱动事件内容数据指针
 * 返 回 值: HDF_SUCCESS为成功，反之为失败
***************************************************************/
static int OnDevEventReceived(void *priv,  uint32_t id, struct HdfSBuf *data)
{
    const char *string = HdfSbufReadString(data);
    if (string == NULL) {
        PRINT_ERROR("fail to read string in event data\n");
        g_replyFlag = 1;
        return HDF_FAILURE;
    }
    
    PRINT_INFO("%s: dev event received: %u %s\n",  (char *)priv, id, string);
    g_replyFlag = 1;
    return HDF_SUCCESS;
}

/***************************************************************
 * 函数名称: SendEvent
 * 说    明: 与驱动进行数据交互
 * 参    数: 
 *      @serv           hdf句柄
 *      @eventData      交互数据字符串
 * 返 回 值: 0为成功，反之为错误
***************************************************************/
static int SendEvent(struct HdfIoService *serv, char *eventData)
{
    int ret = 0;

    // 申请HdfSbuf结构体，用于应用层数据变量
    struct HdfSBuf *data = HdfSbufObtainDefaultSize();
    if (data == NULL) {
        PRINT_ERROR("fail to obtain sbuf data\n");
        return 1;
    }

    // 申请HdfSbuf结构体，用于读取驱动数据的变量
    struct HdfSBuf *reply = HdfSbufObtainDefaultSize();
    if (reply == NULL) {
        PRINT_ERROR("fail to obtain sbuf reply\n");
        ret = HDF_DEV_ERR_NO_MEMORY;
        goto out;
    }

    // 写入字符串到HdfSbuf结构体data
    if (!HdfSbufWriteString(data, eventData)) {
        PRINT_ERROR("fail to write sbuf\n");
        ret = HDF_FAILURE;
        goto out;
    }

    // 通过Dispatch函数调用，与驱动进行数据交互
    ret = serv->dispatcher->Dispatch(&serv->object, SAMPLE_WRITE_READ, data, reply);
    if (ret != HDF_SUCCESS) {
        PRINT_ERROR("fail to send service call\n");
        goto out;
    }

    // 将从HdfSbuf结构体replyData读取int32类型的变量
    int replyData = 0;
    if (!HdfSbufReadInt32(reply, &replyData)) {
        PRINT_ERROR("fail to get service call reply\n");
        ret = HDF_ERR_INVALID_OBJECT;
        goto out;
    }
    PRINT_INFO("Get reply is: %d\n", replyData);
    
out:
    HdfSbufRecycle(data);
    HdfSbufRecycle(reply);
    return ret;
}


/***************************************************************
 * 函数名称: main
 * 说    明: 主函数
 * 参    数: 
 * 返 回 值: 0为成功，反之为错误
***************************************************************/
int main()
{
    char *sendData = "default event info";
    struct HdfIoService *serv = NULL;
    static struct HdfDevEventlistener listener = {
        .callBack = OnDevEventReceived,
        .priv ="Service0"
    };

    // 绑定HDF驱动
    serv = HdfIoServiceBind(SAMPLE_SERVICE_NAME);
    if (serv == NULL) {
        PRINT_ERROR("fail to get service %s\n", SAMPLE_SERVICE_NAME);
        return HDF_FAILURE;
    }

    // 用户态程序注册接收驱动上报事件的操作方法
    if (HdfDeviceRegisterEventListener(serv, &listener) != HDF_SUCCESS) {
        PRINT_ERROR("fail to register event listener\n");
        return HDF_FAILURE;
    }
    if (SendEvent(serv, sendData)) {
        PRINT_ERROR("fail to send event\n");
        return HDF_FAILURE;
    }

    // 等待
    while (g_replyFlag == 0) {
        sleep(1);
    }

    // 注销驱动上报事件
    if (HdfDeviceUnregisterEventListener(serv, &listener)) {
        PRINT_ERROR("fail to  unregister listener\n");
        return HDF_FAILURE;
    }

    // 释放驱动服务
    HdfIoServiceRecycle(serv);
    return HDF_SUCCESS;
}

