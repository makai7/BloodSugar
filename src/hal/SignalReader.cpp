#include <SignalReader.h>

// 获取单例实例
SignalReader& SignalReader::getInstance() {
    static SignalReader instance;
    return instance;
}

// 私有构造函数
SignalReader::SignalReader() :
    _pin(PIN_ADC_IN) 
{
    // 构造函数体为空
}

void SignalReader::begin() {
    // 为指定的ADC引脚设置衰减。
    // ADC_ATTENUATION 在 config.h 中定义为 ADC_ATTEN_DB_11，
    // 这允许ADC测量高达约3.3V的电压。
    analogSetPinAttenuation(_pin, (adc_attenuation_t)ADC_ATTENUATION);
}

uint16_t SignalReader::getRawValue() {
    uint32_t sum = 0;
    
    // 进行多次采样以求平均值，有效滤除高频噪声
    for (int i = 0; i < ADC_SAMPLES_TO_AVERAGE; i++) {
        sum += analogRead(_pin);
        // 短暂延时可能有助于提高某些情况下ADC的稳定性，但对于快速采样可以省略
        // delayMicroseconds(20); 
    }

    return (uint16_t)(sum / ADC_SAMPLES_TO_AVERAGE);
}

float SignalReader::getVoltage() {
    // 1. 获取平均后的原始ADC值
    uint16_t rawValue = getRawValue();

    // 2. 将12位ADC原始值 (0-4095) 转换为电压值。
    // 注意：ESP32的V_REF（参考电压）理论上是3.3V，但实际上可能存在偏差。
    // 精确的测量可能需要对参考电压进行校准。这里我们使用3.3V作为标准值。
    return (float)rawValue * (3.3f / 4095.0f);
}