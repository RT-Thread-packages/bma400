# BMA400

## 简介

本软件包是为 Bosch BMA400 加速度传感器提供的通用传感器驱动包。通过使用此软件包，开发者可以快速的利用 RT-Thread 将此传感器驱动起来。

本篇文档主要内容如下：

- 传感器介绍
- 支持情况
- 使用说明

## 传感器介绍

BMA400 是 Bosch（博世）公司专为可穿戴设备和 IOT 市场开发的一款超低功耗加速度传感器，尺寸小巧且内置计数器。它的功耗在计步模式下只需要 4μA，数据输出速率 12.5-800HZ 可调。

## 支持情况

| 包含设备         | 加速度计 | 计步计 |
| ---------------- | -------- | ------ |
| **通讯接口**     |          |        |
| IIC              | √        | √      |
| SPI              |          |        |
| **工作模式**     |          |        |
| 轮询             | √        | √      |
| 中断             |          |        |
| FIFO             |          |        |
| **电源模式**     |          |        |
| 掉电             | √        | √      |
| 低功耗           | √        | √      |
| 普通             | √        | √      |
| 高功耗           |          |        |
| **数据输出速率** | √        |        |
| **测量范围**     | √        |        |
| **自检**         |          |        |
| **多实例**       | √        | √      |

## 使用说明

### 依赖

- RT-Thread 4.0.0+
- Sensor 组件
- IIC 驱动：BMA400 设备使用 IIC 进行数据通讯，需要系统 IIC 驱动框架支持；
- PIN 驱动：用于处理设备中断引脚；

### 获取软件包

使用 BMA400 软件包需要在 RT-Thread 的包管理中选中它，具体路径如下：

```
RT-Thread online packages --->
    peripheral libraries and drivers --->
        sensors drivers --->
            [*] BMA400: BMA400 sensor driver package, support: accelerometer, step.
                [*]   Enable bma400 acce
                [*]   Enable bma400 step 
                    Version (latest)  --->
```

**Enable bma400 acce**： 配置开启加速度计功能

**Enable bma400 step**：配置开启计步计功能

**Version**：软件包版本选择

### 使用软件包

BMA400 软件包初始化函数如下所示：

```
int rt_hw_bma400_init(const char *name, struct rt_sensor_config *cfg);
```

该函数需要由用户调用，函数主要完成的功能有，

- 设备配置和初始化（根据传入的配置信息，配置接口设备和中断引脚）；
- 注册相应的传感器设备，完成 BMA400 设备的注册；

#### 初始化示例

```
#include "sensor_bosch_bma400.h"

int bma400_port(void)
{
    struct rt_sensor_config cfg;
    
    cfg.intf.dev_name = "i2c1";
    cfg.intf.user_data = (void *)BMA400_ADDR_DEFAULT;
    cfg.irq_pin.pin = RT_PIN_NONE;

    rt_hw_bma400_init("bma400", &cfg);
    return 0;
}
INIT_APP_EXPORT(bma400_port);
```

## 注意事项

暂无

## 联系人信息

维护人:

- [guozhanxin](https://github.com/Guozhanxin) 
