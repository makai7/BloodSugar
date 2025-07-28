#include <GlucoseCalculator.h>
#include <config.h> // 引入配置文件以使用校准参数

// 获取单例实例
GlucoseCalculator& GlucoseCalculator::getInstance() {
    static GlucoseCalculator instance;
    return instance;
}

// 私有构造函数
GlucoseCalculator::GlucoseCalculator() :
    _latestGlucoseValue(0.0f),
    _currentStatus(Status::IDLE)
{
}

void GlucoseCalculator::begin() {
    // 可以在这里进行一些初始化，目前为空
    _currentStatus = Status::IDLE;
}

GlucoseCalculator::Status GlucoseCalculator::performMeasurement() {
    _currentStatus = Status::MEASURING;

    // 1. 更新所有传感器数据
    Max30102Controller::getInstance().update();
    if (!Dht22Controller::getInstance().readData()) {
        _currentStatus = Status::ERROR_SENSOR_READ;
        return _currentStatus;
    }

    // 2. 检查测量的先决条件 (例如：必须有手指放在传感器上)
    if (!Max30102Controller::getInstance().isFingerDetected()) {
        _currentStatus = Status::ERROR_NO_FINGER;
        return _currentStatus;
    }
    
    // 3. 获取所有需要的输入数据
    float mainSignal = SignalReader::getInstance().getVoltage();
    float temp = Dht22Controller::getInstance().getTemperature();
    uint32_t ir = Max30102Controller::getInstance().getIRValue();
    float hr = Max30102Controller::getInstance().getHeartRate();

    // 4. 调用核心算法进行计算
    _latestGlucoseValue = calculate(mainSignal, temp, ir, hr);

    _currentStatus = Status::SUCCESS;
    return _currentStatus;
}

float GlucoseCalculator::getLatestGlucoseValue() const {
    return _latestGlucoseValue;
}

GlucoseCalculator::Status GlucoseCalculator::getCurrentStatus() const {
    return _currentStatus;
}


// =======================================================================
// ==                     核心算法占位符 (Placeholder)                     ==
// =======================================================================
float GlucoseCalculator::calculate(float mainSignalV, float temperature, uint32_t irValue, float heartRate) {
    // 
    // !!! 注意：这是一个非常基础的线性模型示例 !!!
    // !!! 您需要用您自己通过实验数据校准的真实模型来替换它 !!!
    //
    // 一个真实的模型可能会是这样:
    // glucose = f(mainSignalV, temperature, irValue, heartRate, ...)
    // f(...) 可能是一个复杂的多项式、查找表，或者一个神经网络模型。
    //
    // --- 示例开始 ---
    // 假设血糖值与主信号电压呈线性关系，并受温度轻微影响。
    // 我们使用 config.h 中定义的占位符校准参数。
    
    // 基础值，由主信号电压决定
    // float baseGlucose = (mainSignalV * CALIBRATION_SLOPE) + CALIBRATION_OFFSET;
    
    // 温度补偿：假设温度每比25度高1度，血糖读数就降低一个系数
    // float tempCorrection = (temperature - 25.0f) * TEMP_COMPENSATION_COEFFICIENT;

    // 最终估算值
    // float finalGlucose = baseGlucose - tempCorrection;

    // --- 为演示目的，我们先返回一个和多个输入相关的模拟值 ---
    // 例如：(主信号电压 * 100) + (温度) - (IR值 / 10000.0)
    float simulatedGlucose = (mainSignalV * 100.0f) + temperature - (irValue / 20000.0f);
    
    // 保证结果非负
    if (simulatedGlucose < 0) {
      simulatedGlucose = 0;
    }

    return simulatedGlucose;
    // --- 示例结束 ---
}