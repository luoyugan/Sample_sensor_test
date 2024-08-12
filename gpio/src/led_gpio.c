#include <stdio.h>
#include "stdlib.h"
#include "hdf_base.h"
#include "hdf_io_service_if.h"
#include "osal_time.h"
#include "gpio/gpio_service.h"    

int main(int argc, char* argv[]) {
    char *RGB_LED_SERVICE_NAME = "HDF_PLATFORM_GPIO_MANAGER";
    int ret;
    uint16_t gpio = 15;
    uint16_t dir = 1;
    uint16_t value_1 = 1;
    uint16_t value_0 = 0;

    struct HdfIoService *serv = HdfIoServiceBind(RGB_LED_SERVICE_NAME);
    if (serv == NULL) {
        printf( "get service failed\n");
        return -1;
    }
    struct HdfSBuf *data_setdir = HdfSbufObtainDefaultSize();
    struct HdfSBuf *data_write_1 = HdfSbufObtainDefaultSize();
    struct HdfSBuf *data_write_0 = HdfSbufObtainDefaultSize();
    if (data_setdir == NULL || data_write_1 == NULL || data_write_0 == NULL) {
        printf("obtain data failed\n");
        return -1;
    }

    HdfSbufWriteUint16(data_setdir, gpio); // 写入GPIO引脚编号
    HdfSbufWriteUint16(data_setdir, dir); // 写入方向值

    HdfSbufWriteUint16(data_write_1, gpio); // 写入GPIO引脚编号
    HdfSbufWriteUint16(data_write_1, value_1); // 写入值

    HdfSbufWriteUint16(data_write_0, gpio); // 写入GPIO引脚编号
    HdfSbufWriteUint16(data_write_0, value_0); // 写入值


    ret = serv->dispatcher->Dispatch(&serv->object, GPIO_IO_SETDIR, data_setdir, NULL);

    for(int i = 0; i < 60; i++) {
        ret = serv->dispatcher->Dispatch(&serv->object, GPIO_IO_WRITE, data_write_1, NULL);
        OsalSleep(1);
        ret = serv->dispatcher->Dispatch(&serv->object, GPIO_IO_WRITE, data_write_0, NULL);
        OsalSleep(1);
    }
    

    HdfSbufRecycle(data_write_0);
    HdfSbufRecycle(data_write_1);
    HdfSbufRecycle(data_setdir);
    HdfIoServiceRecycle(serv);
    printf("main exit!\n\r");
 
    return ret;
}