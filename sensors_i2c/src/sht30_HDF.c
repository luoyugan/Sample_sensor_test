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


//创建回调函数
static int32_t SensorDataCallback(const struct SensorEvents *event)
{
    if (event == NULL) {
        return HDF_FAILURE;
    }

    float sensorData =*((float *)event->data);
    printf("sensor id [%d], data : %.2f\n", event->sensorId, sensorData);

    printf("Sht30Callback\n\r");

    return HDF_SUCCESS;
}

void SensorGet(void) 
{
    int ret;
    struct SensorInformation *sensorInfo = NULL;
    int32_t count = 0;
    int32_t sensorInterval = 200000000; /* 数据采样率设置200毫秒，单位纳秒 */
    int32_t reportInterval = 400000000;

    //1.创建传感器接口实例
    const struct SensorInterface *sensorDev = NewSensorInterfaceInstance();
    if (sensorDev == NULL) {
        return;
    }
    printf("NewSensorInterfaceInstance success\n");

    //2.订阅者注册传感器数据回调处理函数
    ret = sensorDev->Register(TRADITIONAL_SENSOR_TYPE, SensorDataCallback);
    if (ret != 0) {
        return;
    }
    printf("Register success\n");

    //3.获取设备支持的Sensor列表
    ret = sensorDev->GetAllSensors(&sensorInfo, &count);
    if (ret != 0) {
        return;
    }

    printf("GetAllSensors count: %d\n", count);

     for (int i = 0; i < count; i++)
    {
        printf("sensor [%d] : sensorName:%s, vendorName:%s, sensorTypeId:%d, sensorId:%d\n", i,
               sensorInfo[i].sensorName, sensorInfo[i].vendorName, sensorInfo[i].sensorTypeId, sensorInfo[i].sensorId);
    }

    for (int i = 0; i < count; i++)
    {
         //4.设置传感器采样率
        ret = sensorDev->SetBatch(sensorInfo[i].sensorId, sensorInterval, reportInterval);
        if (ret != 0) {
            printf("SetBatch failed\n ,ret: %d",ret);
            continue;
        }
        printf("SetBatch success\n");

        // 5.使能传感器 
        ret = sensorDev->Enable(sensorInfo[i].sensorId);
        if (ret != 0) {
            continue;
        }
        printf("Enable success\n");

        OsalSleep(1);
        //6.去使能传感器 
        ret = sensorDev->Disable(sensorInfo[i].sensorId);
        if (ret != 0) {
            continue;
        }
        printf("Disable success\n");
    }
    
    //7.取消传感器数据订阅函数 
    ret = sensorDev->Unregister(TRADITIONAL_SENSOR_TYPE, SensorDataCallback);
    if (ret != 0) {
        return;
    }
    printf("Unregister success\n");

    //8.释放传感器接口实例 
    ret = FreeSensorInterfaceInstance();
    if (ret != 0) {
        return;
    }
    printf("FreeSensorInterfaceInstance success\n");
}

int main(int argc, char *argv[])
{
    SensorGet();
    return HDF_SUCCESS;
}






