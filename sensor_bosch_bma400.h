

#ifndef SENSOR_BOS_BMA400_H__
#define SENSOR_BOS_BMA400_H__

#include "sensor.h"
#include "bma400.h"

#define BMA400_ADDR_DEFAULT (BMA400_I2C_ADDRESS_SDO_LOW)

int rt_hw_bma400_init(const char *name, struct rt_sensor_config *cfg);

#endif
