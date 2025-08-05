#ifndef MAX30102_CONTROLLER_H
#define MAX30102_CONTROLLER_H

#include "config.h"
#include <Wire.h>
#include "MAX30105.h" // 库名是MAX30105，但它完美兼容MAX30102
#include "spo2_algorithm.h"

/**
 * @class Max30102Controller
 * @brief 管理MAX30102心率血氧传感器。
 * * 采用单例模式，封装了SparkFun的库。
 * * 提供心率、血氧(SpO2)和IR原始值的读取。
 */
class Max30102Controller {
public:
    /**
     * @brief 获取Max30102Controller的全局唯一实例。
     */
    static Max30102Controller& getInstance();

    // 禁止拷贝
    Max30102Controller(const Max30102Controller&) = delete;
    Max30102Controller& operator=(const Max30102Controller&) = delete;

    /**
     * @brief 初始化传感器。
     * * 这会配置I2C总线和传感器内部参数。
     * @return bool - 如果传感器初始化成功，返回true，否则返回false。
     */
    bool begin();

    /**
     * @brief 从传感器读取新数据并更新内部值。
     * * 应在主循环中尽可能频繁地调用此函数，以保持传感器FIFO缓冲区不溢出。
     */
    void update();

    /**
     * @brief 获取最后计算出的心率值 (BPM)。
     * @return float - 心率值。在读数稳定前可能为0。
     */
    float getHeartRate();

    /**
     * @brief 获取最后计算出的血氧饱和度 (SpO2)。
     * @return float - SpO2百分比。在读数稳定前可能为0。
     */
    float getSpO2();

    /**
     * @brief 获取红外(IR)LED的原始读数。
     * * 这个值与血液灌流量相关，对血糖算法校准可能很有用。
     * @return uint32_t - IR传感器的原始ADC值。
     */
    uint32_t getIRValue();

    /**
     * @brief 检查是否有手指放在传感器上。
     * @return bool - 如果检测到手指，返回true。
     */
    bool isFingerDetected();


private:
    // 私有构造函数
    Max30102Controller();

    MAX30105 _particleSensor; // 来自库的传感器对象
    SpO2Algorithm _spo2_calculator;

    float _heartRate; // 缓存的心率
    float _spO2;      // 缓存的血氧
    uint32_t _irValue; // 缓存的IR值
    uint32_t _redValue;
};

#endif // MAX30102_CONTROLLER_H