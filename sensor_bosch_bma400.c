
#include "sensor_bosch_bma400.h"

#define DBG_ENABLE
#define DBG_LEVEL DBG_LOG
#define DBG_SECTION_NAME  "sensor.bosch.bma400"
#define DBG_COLOR
#include <rtdbg.h>

#define GRAVITY_EARTH (9.80665f)

static void rt_delay_ms(uint32_t period)
{
    rt_thread_mdelay(period);
}

static int8_t rt_i2c_write_reg(void *intf_ptr, uint8_t addr, uint8_t reg, uint8_t *data, uint16_t len)
{
    rt_uint8_t tmp = reg;
    struct rt_i2c_msg msgs[2];

    msgs[0].addr  = addr;             /* Slave address */
    msgs[0].flags = RT_I2C_WR;        /* Write flag */
    msgs[0].buf   = &tmp;             /* Slave register address */
    msgs[0].len   = 1;                /* Number of bytes sent */

    msgs[1].addr  = addr;             /* Slave address */
    msgs[1].flags = RT_I2C_WR | RT_I2C_NO_START;        /* Read flag */
    msgs[1].buf   = data;             /* Read data pointer */
    msgs[1].len   = len;              /* Number of bytes read */

    if (rt_i2c_transfer(intf_ptr, msgs, 2) != 2)
    {
        return -RT_ERROR;
    }

    return RT_EOK;
}

static int8_t rt_i2c_read_reg(void *intf_ptr, uint8_t addr, uint8_t reg, uint8_t *data, uint16_t len)
{
    rt_uint8_t tmp = reg;
    struct rt_i2c_msg msgs[2];

    msgs[0].addr  = addr;             /* Slave address */
    msgs[0].flags = RT_I2C_WR;        /* Write flag */
    msgs[0].buf   = &tmp;             /* Slave register address */
    msgs[0].len   = 1;                /* Number of bytes sent */

    msgs[1].addr  = addr;             /* Slave address */
    msgs[1].flags = RT_I2C_RD;        /* Read flag */
    msgs[1].buf   = data;             /* Read data pointer */
    msgs[1].len   = len;              /* Number of bytes read */

    if (rt_i2c_transfer(intf_ptr, msgs, 2) != 2)
    {
        return -RT_ERROR;
    }

    return RT_EOK;
}

static struct bma400_dev *_bma400_create(struct rt_sensor_intf *intf)
{
    struct bma400_dev *_bma400_dev = RT_NULL;
    struct rt_i2c_bus_device *i2c_bus_dev = RT_NULL;
    struct bma400_int_enable step_int;
    int8_t rslt = BMA400_OK;
    struct bma400_sensor_conf conf;

    i2c_bus_dev = (struct rt_i2c_bus_device *)rt_device_find(intf->dev_name);
    if (i2c_bus_dev == RT_NULL)
    {
        LOG_E("can not find device %s", intf->dev_name);
        return RT_NULL;
    }

    _bma400_dev = rt_calloc(1, sizeof(struct bma400_dev));
    if (_bma400_dev == RT_NULL)
    {
        LOG_E("bma400 dev memory allocation failed");
        return RT_NULL;
    }

    _bma400_dev->dev_id   = (rt_uint32_t)(intf->user_data) & 0xff;
    _bma400_dev->intf     = BMA400_I2C_INTF;
    _bma400_dev->intf_ptr = i2c_bus_dev;
    _bma400_dev->read     = rt_i2c_read_reg;
    _bma400_dev->write    = rt_i2c_write_reg;
    _bma400_dev->delay_ms = rt_delay_ms;

    rslt = bma400_init(_bma400_dev);
    if (rslt == BMA400_OK)
    {
        rslt = bma400_soft_reset(_bma400_dev);

        /* Select the type of configuration to be modified */
        conf.type = BMA400_ACCEL;

        /* Get the accelerometer configurations which are set in the sensor */
        rslt = bma400_get_sensor_conf(&conf, 1, _bma400_dev);

        /* Modify the desired configurations as per macros
         * available in bma400_defs.h file */
        conf.param.accel.odr = BMA400_ODR_100HZ;
        conf.param.accel.range = BMA400_2G_RANGE;
        conf.param.accel.data_src = BMA400_DATA_SRC_ACCEL_FILT_1;

        /* Set the desired configurations to the sensor */
        rslt = bma400_set_sensor_conf(&conf, 1, _bma400_dev);

        step_int.type = BMA400_STEP_COUNTER_INT_EN;
        step_int.conf = BMA400_ENABLE;

        rslt = bma400_enable_interrupt(&step_int, 1, _bma400_dev);

        bma400_set_power_mode(BMA400_SLEEP_MODE, _bma400_dev);

        return _bma400_dev;
    }
    else
    {
        LOG_E("bma400 init failed");
        rt_free(_bma400_dev);
        return RT_NULL;
    }
}

static rt_err_t _bma400_set_odr(rt_sensor_t sensor, rt_uint16_t odr)
{
    struct bma400_dev *_bma400_dev = sensor->parent.user_data;
    struct bma400_sensor_conf conf;
    uint8_t odr_ctr;

    if (odr < 13)
        odr_ctr = BMA400_ODR_12_5HZ;
    else if (odr < 25)
        odr_ctr = BMA400_ODR_25HZ;
    else if (odr < 50)
        odr_ctr = BMA400_ODR_50HZ;
    else if (odr < 100)
        odr_ctr = BMA400_ODR_100HZ;
    else if (odr < 200)
        odr_ctr = BMA400_ODR_200HZ;
    else if (odr < 400)
        odr_ctr = BMA400_ODR_400HZ;
    else
        odr_ctr = BMA400_ODR_800HZ;

    if (sensor->info.type == RT_SENSOR_CLASS_ACCE)
    {
        conf.type = BMA400_ACCEL;

        /* Get the accelerometer configurations which are set in the sensor */
        bma400_get_sensor_conf(&conf, 1, _bma400_dev);

        conf.param.accel.odr = odr_ctr;

        /* Set the desired configurations to the sensor */
        bma400_set_sensor_conf(&conf, 1, _bma400_dev);
        return RT_EOK;
    }

    return RT_EOK;
}
static rt_err_t _bma400_set_range(rt_sensor_t sensor, rt_uint16_t range)
{
    struct bma400_dev *_bma400_dev = sensor->parent.user_data;

    if (sensor->info.type == RT_SENSOR_CLASS_ACCE)
    {
        struct bma400_sensor_conf conf;
        uint8_t range_ctr;

        if (range < 2000)
            range_ctr = BMA400_2G_RANGE;
        else if (range < 4000)
            range_ctr = BMA400_4G_RANGE;
        else if (range < 8000)
            range_ctr = BMA400_8G_RANGE;
        else
            range_ctr = BMA400_16G_RANGE;

        conf.type = BMA400_ACCEL;

        /* Get the accelerometer configurations which are set in the sensor */
        bma400_get_sensor_conf(&conf, 1, _bma400_dev);

        conf.param.accel.range = range_ctr;

        /* Set the desired configurations to the sensor */
        bma400_set_sensor_conf(&conf, 1, _bma400_dev);
        return RT_EOK;
    }

    return RT_EOK;
}
static rt_err_t _bma400_set_power(rt_sensor_t sensor, rt_uint8_t power)
{
    struct bma400_dev *_bma400_dev = sensor->parent.user_data;
    int8_t rslt = 0;

    if (power == RT_SENSOR_POWER_DOWN)
    {
        rslt = bma400_set_power_mode(BMA400_SLEEP_MODE, _bma400_dev);
    }
    else if (power == RT_SENSOR_POWER_NORMAL)
    {

        rslt = bma400_set_power_mode(BMA400_NORMAL_MODE, _bma400_dev);
    }
    else if (power == RT_SENSOR_POWER_LOW)
    {
        rslt = bma400_set_power_mode(BMA400_LOW_POWER_MODE, _bma400_dev);
    }
    else
    {
        LOG_W("Unsupported mode, code is %d", power);
        return -RT_ERROR;
    }
    return rslt;
}

static rt_size_t bma400_fetch_data(struct rt_sensor_device *sensor, void *buf, rt_size_t len)
{
    struct bma400_dev *_bma400_dev = sensor->parent.user_data;
    struct rt_sensor_data *data = buf;

    if (sensor->info.type == RT_SENSOR_CLASS_ACCE)
    {
        struct bma400_sensor_data comp_data;
        bma400_get_accel_data(BMA400_DATA_SENSOR_TIME, &comp_data, _bma400_dev);

        data->type = RT_SENSOR_CLASS_ACCE;
        data->data.acce.x = comp_data.x;
        data->data.acce.y = comp_data.y;
        data->data.acce.z = comp_data.z;
        data->timestamp = rt_sensor_get_ts();
    }
    if (sensor->info.type == RT_SENSOR_CLASS_STEP)
    {
        uint32_t step_count;
        uint8_t activity;
        bma400_get_steps_counted(&step_count, &activity, _bma400_dev);

        data->type = RT_SENSOR_CLASS_STEP;
        data->data.step = step_count;
        data->timestamp = rt_sensor_get_ts();
    }
    return 1;
}

static rt_err_t bma400_control(struct rt_sensor_device *sensor, int cmd, void *args)
{
    struct bma400_dev *_bma400_dev = sensor->parent.user_data;
    rt_err_t result = RT_EOK;

    switch (cmd)
    {
    case RT_SENSOR_CTRL_GET_ID:
        *(rt_uint8_t *)args = _bma400_dev->chip_id;
        break;
    case RT_SENSOR_CTRL_SET_ODR:
        result = _bma400_set_odr(sensor, (rt_uint32_t)args & 0xffff);
        break;
    case RT_SENSOR_CTRL_SET_RANGE:
        result = _bma400_set_range(sensor, (rt_uint32_t)args);
        break;
    case RT_SENSOR_CTRL_SET_POWER:
        result = _bma400_set_power(sensor, (rt_uint32_t)args & 0xff);
        break;
    case RT_SENSOR_CTRL_SELF_TEST:
        /* TODO */
        result = -RT_EINVAL;
        break;
    default:
        return -RT_EINVAL;
    }
    return result;
}

static struct rt_sensor_ops sensor_ops =
{
    bma400_fetch_data,
    bma400_control
};

int rt_hw_bma400_init(const char *name, struct rt_sensor_config *cfg)
{
    rt_int8_t result;
    rt_sensor_t sensor_acce = RT_NULL, sensor_step = RT_NULL;
    struct bma400_dev *_bma400_dev = RT_NULL;

    _bma400_dev = _bma400_create(&cfg->intf);
    if (_bma400_dev == RT_NULL)
    {
        LOG_E("sensor create failed");
        return -RT_ERROR;
    }

#ifdef PKG_USING_BMA400_ACCE
    /* accelerometer sensor register */
    {
        sensor_acce = rt_calloc(1, sizeof(struct rt_sensor_device));
        if (sensor_acce == RT_NULL)
            return -RT_ERROR;

        sensor_acce->info.type       = RT_SENSOR_CLASS_ACCE;
        sensor_acce->info.vendor     = RT_SENSOR_VENDOR_BOSCH;
        sensor_acce->info.model      = "bma400_acce";
        sensor_acce->info.unit       = RT_SENSOR_UNIT_PA;
        sensor_acce->info.intf_type  = RT_SENSOR_INTF_I2C;
        sensor_acce->info.range_max  = 110000;
        sensor_acce->info.range_min  = 30000;
        sensor_acce->info.period_min = 100;

        rt_memcpy(&sensor_acce->config, cfg, sizeof(struct rt_sensor_config));
        sensor_acce->ops = &sensor_ops;

        result = rt_hw_sensor_register(sensor_acce, name, RT_DEVICE_FLAG_RDWR, _bma400_dev);
        if (result != RT_EOK)
        {
            LOG_E("device register err code: %d", result);
            rt_free(sensor_acce);
            return -RT_ERROR;
        }
    }
#endif
#ifdef PKG_USING_BMA400_STEP
    /* step sensor register */
    {
        sensor_step = rt_calloc(1, sizeof(struct rt_sensor_device));
        if (sensor_step == RT_NULL)
            goto __exit;

        sensor_step->info.type       = RT_SENSOR_CLASS_STEP;
        sensor_step->info.vendor     = RT_SENSOR_VENDOR_BOSCH;
        sensor_step->info.model      = "bma400_step";
        sensor_step->info.unit       = RT_SENSOR_UNIT_ONE;
        sensor_step->info.intf_type  = RT_SENSOR_INTF_I2C;
        sensor_step->info.period_min = 100;

        rt_memcpy(&sensor_step->config, cfg, sizeof(struct rt_sensor_config));
        sensor_step->ops = &sensor_ops;

        result = rt_hw_sensor_register(sensor_step, name, RT_DEVICE_FLAG_RDWR, _bma400_dev);
        if (result != RT_EOK)
        {
            LOG_E("device register err code: %d", result);
            goto __exit;
        }
    }
#endif
    return RT_EOK;

__exit:
    rt_free(sensor_acce);
    rt_device_unregister(&sensor_acce->parent);
    if (sensor_step)
        rt_free(sensor_step);
    return -RT_ERROR;
}
