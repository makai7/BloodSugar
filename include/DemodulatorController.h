#ifndef DEMODULATOR_CONTROLLER_H
#define DEMODULATOR_CONTROLLER_H

#include "config.h"

/**
 * @class DemodulatorController
 * @brief 管理同步解调模块的参考信号。
 * * 采用单例模式，使用ESP32的LEDC外设生成与LED脉冲同步的方波。
 */
class DemodulatorController {
public:
    /**
     * @brief 获取DemodulatorController的全局唯一实例。
     */
    static DemodulatorController& getInstance();

    // 禁止拷贝
    DemodulatorController(const DemodulatorController&) = delete;
    DemodulatorController& operator=(const DemodulatorController&) = delete;

    /**
     * @brief 初始化参考信号发生器。
     */
    void begin();

    /**
     * @brief 开始生成参考信号。
     * * 信号频率与LED脉冲频率相同，占空比为50%。
     */
    void start();

    /**
     * @brief 停止生成参考信号。
     */
    void stop();

private:
    // 私有构造函数
    DemodulatorController();

    const int _pin;
    const int _pwmChannel;
    const int _frequency;
    const int _resolution;
};

#endif // DEMODULATOR_CONTROLLER_H