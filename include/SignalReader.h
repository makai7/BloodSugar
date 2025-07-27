#ifndef SIGNAL_READER_H
#define SIGNAL_READER_H

#include "config.h"

/**
 * @class SignalReader
 * @brief 管理来自同步解调模块的最终模拟信号的读取。
 * * 采用单例模式设计。
 * 封装了ESP32的ADC读取、多次采样平均以降低噪声等功能。
 */
class SignalReader {
public:
    /**
     * @brief 获取SignalReader的全局唯一实例。
     * @return SignalReader对象的引用。
     */
    static SignalReader& getInstance();

    // 禁止拷贝
    SignalReader(const SignalReader&) = delete;
    SignalReader& operator=(const SignalReader&) = delete;

    /**
     * @brief 初始化ADC引脚和相关参数。
     * 应在主程序的 setup() 中调用一次。
     */
    void begin();

    /**
     * @brief 执行一次完整的测量，返回多次采样平均后的ADC原始值。
     * @return uint16_t 平均后的ADC值 (0-4095 for 12-bit)。
     */
    uint16_t getRawValue();

    /**
     * @brief 执行一次完整的测量，并将其转换为电压值。
     * @return float 测量到的电压 (V)。
     */
    float getVoltage();

private:
    // 私有构造函数
    SignalReader(); 

    const uint8_t _pin; // ADC输入引脚
};

#endif // SIGNAL_READER_H