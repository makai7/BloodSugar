#ifndef GLUCOSE_CALCULATOR_H
#define GLUCOSE_CALCULATOR_H

#include "hal/SignalReader.h"
#include "hal/Dht22Controller.h"
#include "hal/Max30102Controller.h"
// 注意：我们暂时还没有创建DemodulatorController，所以先不包含它

/**
 * @class GlucoseCalculator
 * @brief 核心业务逻辑类，负责整合传感器数据并计算血糖值。
 * 采用单例模式。
 */
class GlucoseCalculator {
public:
    enum class Status {
        IDLE,
        MEASURING,
        SUCCESS,
        ERROR_NO_FINGER,
        ERROR_SENSOR_READ
    };

    /**
     * @brief 获取GlucoseCalculator的全局唯一实例。
     */
    static GlucoseCalculator& getInstance();

    // 禁止拷贝
    GlucoseCalculator(const GlucoseCalculator&) = delete;
    GlucoseCalculator& operator=(const GlucoseCalculator&) = delete;

    /**
     * @brief 初始化计算器。
     */
    void begin();

    /**
     * @brief 执行一次完整的血糖测量流程。
     * @return Status - 返回本次测量的最终状态。
     */
    Status performMeasurement();

    /**
     * @brief 获取最近一次成功测量的血糖值。
     * @return float - 血糖值 (单位需要您根据模型确定，例如 mg/dL)。
     */
    float getLatestGlucoseValue() const;
    
    /**
     * @brief 获取当前计算器的状态。
     */
    Status getCurrentStatus() const;


private:
    // 私有构造函数
    GlucoseCalculator();

    /**
     * @brief 内部计算函数，包含核心算法。
     * @param mainSignalV 主光学信号的电压值。
     * @param temperature 环境/体表温度值。
     * @param irValue MAX30102的IR读数，反映血液灌流情况。
     * @param heartRate 心率值。
     * @return float - 计算出的血糖值。
     */
    float calculate(float mainSignalV, float temperature, uint32_t irValue, float heartRate);

    float _latestGlucoseValue;
    Status _currentStatus;
};

#endif // GLUCOSE_CALCULATOR_H