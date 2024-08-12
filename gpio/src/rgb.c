/*
 * Copyright (c) 2022 FuZhou Lockzhiner Electronic Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "stdio.h"
#include "unistd.h"
#include "stdlib.h"
#include "hdf_base.h"
#include "hdf_io_service_if.h"
#include "hilog/log.h"

#undef  LOG_TAG
#undef  LOG_DOMAIN
#define LOG_TAG                                             "rgbled"
#define LOG_DOMAIN                                          0xD002022

#define ARGS_NUM                                            2

#define RGB_LED_SERVICE_NAME                                "rgb_led_service"
#define RGB_LED_WRITE                                       1
#define RGB_LED_RED_BIT                                     0x1
#define RGB_LED_GREEN_BIT                                   0x2
#define RGB_LED_BLUE_BIT                                    0x4

int main(int argc, char* argv[])
{
    int ret = HDF_SUCCESS;
    int32_t mode = -1;
    if (argc == ARGS_NUM) {
        mode = atoi(argv[1]);
        /*low-3bits*/
        mode &= 0x7;
        // HILOG_INFO(LOG_APP, "[%s] main enter: mode[%s%s%s][0x%X]",
        //     LOG_TAG,
        //     (mode&RGB_LED_BLUE_BIT)?"B":"-",
        //     (mode&RGB_LED_GREEN_BIT)?"G":"-",
        //     (mode&RGB_LED_RED_BIT)?"R":"-",
        //     mode);
        printf("RGB mode[%s%s%s][0x%X]\n",
            (mode&RGB_LED_BLUE_BIT)?"B":"-",
            (mode&RGB_LED_GREEN_BIT)?"G":"-",
            (mode&RGB_LED_RED_BIT)?"R":"-",
            mode);
    } else {
        printf( "[%s] main enter: auto test RGB LED\n", LOG_TAG);
        printf("auto test RGB LED\n");
    }

    struct HdfIoService *serv = HdfIoServiceBind(RGB_LED_SERVICE_NAME);
    if (serv == NULL) {
        printf("get service %s failed\n", RGB_LED_SERVICE_NAME);
        return -1;
    }
    struct HdfSBuf *data = HdfSbufObtainDefaultSize();
    if (data == NULL) {
        printf( "obtain data failed\n");
        return -1;
    }

    if (mode == -1) {
        mode = 0x8;
        while (mode) {
            HdfSbufFlush(data);
            if (!HdfSbufWriteInt32(data, --mode)) {
                printf("write data failed\n");
                return -1;
            }
            ret = serv->dispatcher->Dispatch(&serv->object, RGB_LED_WRITE, data, NULL);
            sleep(1);
        }
    } else {
        if (!HdfSbufWriteInt32(data, mode)) {
            printf("write data failed\n");
            return -1;
        }
        ret = serv->dispatcher->Dispatch(&serv->object, RGB_LED_WRITE, data, NULL);
    }

    HdfSbufRecycle(data);
    HdfIoServiceRecycle(serv);
    printf("[%s] main exit.\n", LOG_TAG);

    return ret;
}