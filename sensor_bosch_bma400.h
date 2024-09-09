

#ifndef SENSOR_BOS_BMA400_H__
#define SENSOR_BOS_BMA400_H__

#include <rtthread.h>
#if defined(RT_VERSION_CHECK)
    #if (RTTHREAD_VERSION >= RT_VERSION_CHECK(5, 0, 2))
        #define RT_SIZE_TYPE   rt_ssize_t
    #else
        #define RT_SIZE_TYPE   rt_size_t
    #endif

    #if (RTTHREAD_VERSION >= RT_VERSION_CHECK(5, 1, 0))
        #include "drivers/sensor.h"
    #else
        #include "sensor.h"
    #endif
#endif
#include "bma400.h"

#define BMA400_ADDR_DEFAULT (BMA400_I2C_ADDRESS_SDO_LOW)

int rt_hw_bma400_init(const char *name, struct rt_sensor_config *cfg);

#endif
