#ifndef LED_CONTROLLER_H
#define LED_CONTROLLER_H

#include "config.h"

class LedController {
public:
    /**
     * @brief 获取LedController的全局唯一实例。
     * @return LedController对象的引用。
     */
    static LedController& getInstance();

    // 删除拷贝构造函数和赋值运算符，防止复制实例
    LedController(const LedController&) = delete;
    LedController& operator=(const LedController&) = delete;

    // 其他公共方法保持不变
    void begin();
    void startPulsing();
    void stopPulsing();
    void forceOn();
    void forceOff();

private:
    /**
     * @brief 私有构造函数。
     * 外部代码不能直接创建LedController对象。
     */
    LedController();

    // 私有成员变量保持不变
    const int _pin;
    const int _pwmChannel;
    const int _frequency;
    const int _resolution;
};

#endif // LED_CONTROLLER_H