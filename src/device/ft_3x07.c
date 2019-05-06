/*
 * Copyright (c) 2006-2019, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-02-08     Zhangyihong  the first version
 * 2019-05-06     jhb          Adaptation package
 */

#include <rtthread.h>
#include <rthw.h>
#include <rtdevice.h>
#include "touch.h"
#include <string.h>

#ifdef PKG_USING_TOUCH

/*Enable debug information*/
#define DBG_ENABLE

#ifdef  DBG_ENABLE
#define DBG_SECTION_NAME  "TOUCH.ft"
#define DBG_LEVEL         DBG_LOG
#define DBG_COLOR
#include <rtdbg.h>
#endif

#define IIC_RETRY_NUM 2

#define CHIP_ID_REG                 0xA8U
#define CHIP_ID_VALUE               0x11U
#define TOUCH_SLAVE_ADDR            0x38U

static struct rt_i2c_bus_device *ft_i2c_bus;
static struct touch_drivers ft_driver;

static int ft_read(struct rt_i2c_bus_device *i2c_bus, rt_uint8_t addr, rt_uint8_t *buffer, rt_size_t length)
{
    int ret = -1;
    int retries = 0;

    struct rt_i2c_msg msgs[] =
    {
        {
            .addr   = TOUCH_SLAVE_ADDR,
            .flags  = RT_I2C_WR,
            .len    = 1,
            .buf    = &addr,
        },
        {
            .addr   = TOUCH_SLAVE_ADDR,
            .flags  = RT_I2C_RD,
            .len    = length,
            .buf    = buffer,
        },
    };

    while (retries < IIC_RETRY_NUM)
    {
        ret = rt_i2c_transfer(i2c_bus, msgs, 2);
        if (ret == 2)break;
        retries++;
    }

    if (retries >= IIC_RETRY_NUM)
    {
        LOG_E("%s i2c read error: %d", __func__, ret);
        return -1;
    }

    return ret;
}

static void ft_write(touch_drv_t driver, struct rt_i2c_bus_device *i2c_bus, rt_uint8_t addr, rt_uint8_t *buffer, rt_size_t length)
{

    rt_uint8_t *send_buffer = rt_malloc(length + 1);

    RT_ASSERT(send_buffer);

    send_buffer[0] = addr;
    memcpy(send_buffer + 1, buffer, length);

    struct rt_i2c_msg msgs[] =
    {
        {
            .addr   = TOUCH_SLAVE_ADDR,
            .flags  = RT_I2C_WR,
            .len    = length + 1,
            .buf    = send_buffer,
        }
    };

    length = rt_i2c_transfer(i2c_bus, msgs, 1);
    rt_free(send_buffer);
    send_buffer = RT_NULL;
}

static void ft_isr_enable(rt_bool_t enable)
{
    rt_pin_irq_enable(PKG_TOUCH_INT_PIN, enable);
}

static void ft_touch_isr(void *parameter)
{
    ft_isr_enable(RT_FALSE);
    rt_touch_release_sem();
}

static rt_err_t ft_read_point(touch_msg_t msg)
{
    int ret = -1;
    uint8_t point_num = 0;
    static uint8_t s_tp_down = 0;
    uint8_t point[6];
    ret = ft_read(ft_i2c_bus, 0x02, &point_num, 1);
    if (ret < 0)
    {
        return RT_ERROR;
    }
    
    if (point_num == 0)
    {
        if (s_tp_down)
        {
            s_tp_down = 0;
            msg->event = TOUCH_EVENT_UP;
            return RT_EOK;
        }
        msg->event = TOUCH_EVENT_NONE;
        return RT_ERROR;
    }
    
    ret = ft_read(ft_i2c_bus, 0x03, point, 6);
    if (ret < 0)
    {
        return RT_ERROR;
    }

    msg->y = (point[0]&0x0F) << 8 | point[1];
    msg->x = (point[2]&0x0F) << 8 | point[3];
    if (s_tp_down)
    {
        msg->event = TOUCH_EVENT_MOVE;
        return RT_EOK;
    }
    msg->event = TOUCH_EVENT_DOWN;
    s_tp_down = 1;
    
    return RT_EOK;
}

static void ft_init(struct rt_i2c_bus_device *i2c_bus)
{
    if (ft_i2c_bus == RT_NULL)
    {
        ft_i2c_bus = i2c_bus;
    }

    rt_pin_mode(PKG_TOUCH_INT_PIN, PIN_MODE_INPUT_PULLUP);
    rt_pin_attach_irq(PKG_TOUCH_INT_PIN, PIN_IRQ_MODE_FALLING, ft_touch_isr, RT_NULL);

    rt_thread_mdelay(200);
}

static void ft_deinit(void)
{
    
}

struct touch_ops ft_ops =
{
    ft_isr_enable,
    ft_read_point,
    ft_init,
    ft_deinit,
};

static rt_bool_t ft_probe(struct rt_i2c_bus_device *i2c_bus)
{
    int err = 0;
    uint8_t cid = 0xFF;

    ft_i2c_bus = i2c_bus;
    err = ft_read(ft_i2c_bus, 0xA3, (uint8_t *)&cid, 1);
    if (err < 0)
    {
        LOG_E("%s failed: %d", __func__, err);
        return RT_FALSE;
    }
    LOG_I("touch CID:%02X", cid);
    if(cid == 0x54)
    {
        return RT_TRUE;
    }
    return RT_FALSE;
}

int ft_driver_register(void)
{
    ft_driver.probe = ft_probe;
    ft_driver.ops = &ft_ops;
    ft_driver.user_data = RT_NULL;
    rt_touch_drivers_register(&ft_driver);
    return 0;
}

INIT_DEVICE_EXPORT(ft_driver_register);

#endif
