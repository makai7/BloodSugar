#include "LedController.h"

// 在类的实现文件中定义静态实例
// C++11及以后的标准保证了这种方式的线程安全性
LedController& LedController::getInstance() {
    static LedController instance; // 静态局部变量，只会被初始化一次
    return instance;
}

// 构造函数现在是私有的
LedController::LedController() :
    _pin(PIN_LED_CTRL),
    _pwmChannel(0),
    _frequency(OPTICAL_SIGNAL_FREQ_HZ),
    _resolution(8)
{
    // 构造函数体为空
}

// 其他方法的实现保持完全不变...
void LedController::begin() {
    ledcSetup(_pwmChannel, _frequency, _resolution);
    ledcAttachPin(_pin, _pwmChannel);
    stopPulsing();
}

void LedController::startPulsing() {
    ledcAttachPin(_pin, _pwmChannel);
    ledcWrite(_pwmChannel, LED_PULSE_DUTY_CYCLE);
}

void LedController::stopPulsing() {
    ledcWrite(_pwmChannel, 0);
}

void LedController::forceOn() {
    ledcDetachPin(_pin);
    pinMode(_pin, OUTPUT);
    digitalWrite(_pin, HIGH);
}

void LedController::forceOff() {
    ledcDetachPin(_pin);
    pinMode(_pin, OUTPUT);
    digitalWrite(_pin, LOW);
}