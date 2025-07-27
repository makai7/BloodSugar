#ifndef DHT22_CONTROLLER_H
#define DHT22_CONTROLLER_H

#include "config.h"
#include <DHT.h> // 引入刚才添加的库

/**
 * @class Dht22Controller
 * @brief 管理DHT22温湿度传感器。
 * * 采用单例模式，并封装了Adafruit的DHT库。
 * * 实现了非阻塞的、带缓存的读取机制。
 */
class Dht22Controller {
public:
    /**
     * @brief 获取Dht22Controller的全局唯一实例。
     */
    static Dht22Controller& getInstance();

    // 禁止拷贝
    Dht22Controller(const Dht22Controller&) = delete;
    Dht22Controller& operator=(const Dht22Controller&) = delete;

    /**
     * @brief 初始化DHT传感器。
     */
    void begin();

    /**
     * @brief 从传感器读取新数据。
     * * 内部包含2秒的读取间隔限制，可安全地在主循环中频繁调用。
     * * 只有当距离上次成功读取超过2秒时，才会真正执行硬件读取。
     * @return bool - 如果读取成功或仍在2秒冷却期内，返回true；如果硬件读取失败，返回false。
     */
    bool readData();

    /**
     * @brief 获取最后一次成功读取的温度值。
     * @return float - 温度 (摄氏度, °C)。如果从未成功读取，返回NAN。
     */
    float getTemperature();

    /**
     * @brief 获取最后一次成功读取的湿度值。
     * @return float - 相对湿度 (百分比, %)。如果从未成功读取，返回NAN。
     */
    float getHumidity();

private:
    // 私有构造函数
    Dht22Controller();

    DHT _dht; // 来自库的DHT对象实例

    float _lastTemperature; // 缓存的温度值
    float _lastHumidity;    // 缓存的湿度值
    
    unsigned long _lastReadTime; // 上次读取的时间戳
};

#endif // DHT22_CONTROLLER_H