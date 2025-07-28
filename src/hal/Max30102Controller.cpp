#include <Max30102Controller.h>

// 获取单例实例
Max30102Controller& Max30102Controller::getInstance() {
    static Max30102Controller instance;
    return instance;
}

// 私有构造函数
Max30102Controller::Max30102Controller() :
    _heartRate(0.0f),
    _spO2(0.0f),
    _irValue(0)
{
}

bool Max30102Controller::begin() {
    // 初始化I2C总线 (如果之前没有初始化过)
    // Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL, I2C_CLOCK_SPEED);
    // 注意: 通常I2C总线在main.cpp中统一初始化一次即可。
    // 这里假设外部已调用Wire.begin()。

    // 初始化传感器
    if (!_particleSensor.begin(Wire, I2C_CLOCK_SPEED)) {
        return false; // 传感器未找到
    }

    // --- 配置传感器 ---
    // 这些设置是获得良好读数的关键，可能需要根据实际情况微调

    // 设置LED电流 (可接受范围: 0-255, 对应 0-50mA)
    // 一个经验值是从较低的电流开始，如 7mA (对应约35)
    _particleSensor.setPulseAmplitudeRed(0x24); // 红色LED电流
    _particleSensor.setPulseAmplitudeIR(0x24);  // 红外LED电流

    // 设置采样率和脉冲宽度
    // 采样率: 100 samples per second
    // 脉冲宽度: 411 microseconds
    // ADC范围: 4096
    _particleSensor.setup(4096, 4, 100, 411);

    return true;
}

void Max30102Controller::update() {
    // 从传感器FIFO缓冲区检查并读取数据
    // 这个函数应该在loop中被频繁调用
    _particleSensor.check(); 

    // 只有当有新样本被处理时，才更新我们的值
    if (_particleSensor.available()) {
        _irValue = _particleSensor.getIR();
        // 库内部会自动计算心率和血氧，但需要一定数量的样本后才稳定
        _heartRate = _particleSensor.getHeartRate();
        _spO2 = _particleSensor.getSpO2();

        // 读取下一个样本，为下一次available()做准备
        _particleSensor.nextSample();
    }
}

float Max30102Controller::getHeartRate() {
    return _heartRate;
}

float Max30102Controller::getSpO2() {
    return _spO2;
}

uint32_t Max30102Controller::getIRValue() {
    return _irValue;
}

bool Max30102Controller::isFingerDetected() {
    // 通常IR值大于一个阈值（例如50000）可以认为有手指存在
    return (_irValue > 50000);
}