#include "DemodulatorController.h"

// 获取单例实例
DemodulatorController& DemodulatorController::getInstance() {
    static DemodulatorController instance;
    return instance;
}

// 私有构造函数
DemodulatorController::DemodulatorController() :
    _pin(PIN_DEMOD_REF),
    _pwmChannel(1), // !!重要!! 为解调信号选择一个不同的LEDC通道（例如通道1）
    _frequency(OPTICAL_SIGNAL_FREQ_HZ), // !!重要!! 频率必须与LED脉冲完全相同
    _resolution(8)
{
}

void DemodulatorController::begin() {
    // 设置LEDC通道
    ledcSetup(_pwmChannel, _frequency, _resolution);

    // 将GPIO引脚附加到已设置的LEDC通道
    ledcAttachPin(_pin, _pwmChannel);

    // 初始化时，确保信号是停止的
    stop();
}

void DemodulatorController::start() {
    // 写入占空比。对于参考信号，通常使用50%的占空比。
    // 8位分辨率下，50% 对应的值是 255 / 2 = 127.5，取128。
    ledcWrite(_pwmChannel, 128);
}

void DemodulatorController::stop() {
    // 将占空比设置为0即可停止信号
    ledcWrite(_pwmChannel, 0);
}

