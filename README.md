# Touch
## 1.介绍
**Touch** 是基于RT-Thread实现的触摸屏设备软件框架。使用此框架可以简单适配各种触摸芯片，同时也可以适配不同的上层gui框架。

### 1.1 目录结构

|名称|说明|
|---|---|
| src|源程序目录|
| port |移植其他触摸芯片示例文件|



### 1.2 依赖

- **IIC总线设备** 用于通信 
- **外部中断引脚** 用于触发读取触摸中断

## 2.如何使用
- 使用 `menuconfig` 使能**Touch**软件包

```
-> RT-Thread online packages                                                                        
	-> system packages
		-> Touch
```

- 选择自己的触摸IC,如果没有可以自己添加适配并提交（后面附教程）

```
Select Touch IC
```

- 设置触摸线程采样率 (单位HZ)

```
Set the touch screen sample rate
```

- 设置IIC总线设备名称

```
Set the IIC bus device name
```

-  **以上步骤配置完成之后 **
-  使用 `pkgs --update` 命令下载软件包
-  使用 `Scons --target=mdk5` 命令更新KEIL工程


## 3.注意事项

打开配置好的KEIL工程，在分组Touch中有我们的程序源代码

- **在源代码中开启宏定义 `DBG_ENABLE` 选项时，会在msh窗口打印触摸坐标点用于测试当前驱动是否正常工作**

- **测试完毕之后，建议关闭此选项重新编译工程，因为在调试信息打印过程中会占用部分处理器资源**

- **使用TOUCH设备依赖于IIC总线 请开启IIC设备支持**

## 4.如何支持一个新的设备

### 准备工作
- fork 并 clone 本项目到本地
- 复制`PORT`目录下的`touch_device.c`文件到`src/device`目录下
- 建议使用芯片名称命名

---
### 代码修改
#### 1.修改一下宏定义

```c
#define	TOUCH_INT_PIN					定义触摸芯片外部中断引脚编号
#define	CHIP_ID_REG						定义触摸芯片ID寄存器地址
#define CHIP_ID_VALUE					定义触摸芯片ID寄存器的值
#define	TOUCH_SLAVE_ADDR				定义触摸芯片从机设备IIC地址
```

#### 2.实现函数 read_point
```c
/**
  * @brief  读取触摸坐标信息 返回到 touch_msg 结构体
  * @param  触摸信息结构
  * @retval 读取是否成功
  */
static rt_err_t read_point(touch_msg_t msg);

/* 触摸信息结构体具体内容 */
struct touch_message
{
    rt_uint16_t x;
    rt_uint16_t y;
    rt_uint8_t event;
};

```

read_point 函数需要实现的功能是读取触摸位置，以及判读触摸事件类型

触摸事件包含以下四种类型
```c
#define TOUCH_EVENT_UP      抬起事件
#define TOUCH_EVENT_DOWN    按下事件
#define TOUCH_EVENT_MOVE    移动事件
#define TOUCH_EVENT_NONE    无事件
```

### 3.修改SConscript文件内容
在SConscript文件中添加如下代码 用于在在编译时添加选中的的设备驱动文件
```python
    
if GetDepend(['DEVICE_NAME']):
    src += Glob('src/device/DEVICE_NAME.c')

```

### 4.修改ENV目录中PACKAGE的配置文件 KCONFIG
在`\packages\system\kconfig`文件中添加如下内容

本文件描述了了 `menuconfig` 的配置信息

```python
menu "Select Touch IC"

    config FT3X07
    	bool "ft3x07"

    //添加如下内容 用于在menuconfig中启动配置
    config DEVICE_NAME
    	bool "DEVICE_NAME"

endmenu   
```



### 完成以上的移植部分之后，可以在本地进行相关测试，测试通过之后，可以在GitHub中发起合并请求合并到我们的软件包中



# 5.联系方式

- 维护: [jhb](https://github.com/jhbdream)


 欢迎大家积极添加自己使用的触摸设备到此软件包，避免重复造轮子
