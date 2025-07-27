#include "Dht22Controller.h"

// 获取单例实例
Dht22Controller& Dht22Controller::getInstance() {
    static Dht22Controller instance;
    return instance;
}

// 私有构造函数，初始化DHT对象和成员变量
Dht22Controller::Dht22Controller() :
    _dht(PIN_DHT22_DATA, DHT22), // 告诉库我们用的是哪个引脚和哪个型号(DHT22)
    _lastTemperature(NAN),       // 使用NAN (Not-A-Number) 表示无效读数
    _lastHumidity(NAN),
    _lastReadTime(0)
{
}

void Dht22Controller::begin() {
    _dht.begin();
}

bool Dht22Controller::readData() {
    // DHT22传感器两次读取之间至少需要2秒间隔。
    // millis() 返回开机以来的毫秒数。
    // 如果距离上次读取不足2000毫秒，则直接返回true，不执行新的硬件读取。
    if (millis() - _lastReadTime < 2000) {
        return true; 
    }

    // 读取湿度和温度
    float humidity = _dht.readHumidity();
    // 在读取湿度后立即读取温度可以得到最好的效果
    float temperature = _dht.readTemperature();

    // 更新读取时间戳
    _lastReadTime = millis();

    // 检查读取是否失败 (库在失败时会返回 NAN)
    if (isnan(humidity) || isnan(temperature)) {
        return false; // 报告读取失败
    } else {
        // 读取成功，更新缓存的值
        _lastHumidity = humidity;
        _lastTemperature = temperature;
        return true; // 报告读取成功
    }
}

float Dht22Controller::getTemperature() {
    return _lastTemperature;
}

float Dht22Controller::getHumidity() {
    return _lastHumidity;
}