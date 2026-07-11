#ifndef SOL_GPIO_WRAPPER_H
#define SOL_GPIO_WRAPPER_H

#include <stdint.h>
#include <stdbool.h>

void gpio_wrapper_init(bool keep_power_en);
// GPIO操作接口 
void gpio_wrapper_set_level(uint32_t gpio_num, uint8_t level);
uint8_t gpio_wrapper_get_level(uint32_t gpio_num);

// 所有的GPIO定义
// 内部 Flash 绑定引脚:
// GPIO11, GPIO12, GPIO13, GPIO14, GPIO15, GPIO16, GPIO17

// USB 绑定引脚:
// GPIO18 (USB_D-), GPIO19 (USB_D+)

// 默认串口引脚（有条件可用）
// GPIO20 (RX0), GPIO21 (TX0)

// 启动引脚
// GPIO2, GPIO8 (RESET), GPIO9 (BOOT)

//// 可用 GPIO:
// RTC GPIO: 
// GPIO0, GPIO1, GPIO2(小心！), GPIO3, GPIO4, GPIO5
// 通用GPIO:
// GPIO6, GPIO7, GPIO10, GPIO18, GPIO19

//// Power相关GPIO定义
// 测量电池电压的输入
#define GPIO_PIN_POWER_BATTERY_ADC 0
// 测量电池电压使能
#define GPIO_PIN_POWER_BATTERY_EN 6
// 使能总电源，必须是RTC GPIO
#define GPIO_PIN_POWER_EN 5
// 探测USB是否连接，必须是RTC GPIO
#define GPIO_PIN_POWER_ADP_ON 4
// 检查STDBY状态
#define GPIO_PIN_POWER_STDBY 7

// I2C相关GPIO定义
#define GPIO_PIN_I2C_SCL 18
#define GPIO_PIN_I2C_SDA 19

// OLED电源
#define GPIO_PIN_OLED_EN 10

// 按钮，必须是RTC GPIO
#define GPIO_PIN_KEY 3

// 电容测量（湿度）
// PWM输出
#define GPIO_PIN_HUMIDITY_PWM 2
// ADC输入
#define GPIO_PIN_HUMIDITY_ADC 1

#endif //SOL_GPIO_WRAPPER_H
