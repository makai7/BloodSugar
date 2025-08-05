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
    _irValue(0),
    _redValue(0)
{
}

bool Max30102Controller::begin() {
    if (!_particleSensor.begin(Wire, I2C_CLOCK_SPEED)) {
        return false;
    }
    
    // Configure sensor settings for SpO2 calculation
    uint8_t sampleAverage = 4;      // Options: 1, 2, 4, 8, 16, 32
    uint8_t ledMode = 2;            // Options: 1 = Red only, 2 = Red + IR, 3 = Red + IR + Green. We need 2.
    uint16_t sampleRate = 100;      // Options: 50, 100, 200, 400... Must match FS in algorithm.
    uint16_t pulseWidth = 411;      // Options: 69, 118, 215, 411. This fixes the warning.
    uint16_t adcRange = 4096;       // Options: 2048, 4096, 8192, 16384
    uint8_t ledBrightness = 0x24;   // A good starting point (~7mA). Range: 0-255.

    _particleSensor.setup(ledBrightness, sampleAverage, ledMode, sampleRate, pulseWidth, adcRange);
    
    // It's good practice to clear the FIFO buffer before starting measurements
    _particleSensor.clearFIFO(); 

    return true;
}

void Max30102Controller::update() {
    // Check the sensor for new data
    _particleSensor.check();

    // Process all available samples from the FIFO buffer
    while (_particleSensor.available()) {
        _irValue = _particleSensor.getIR();
        _redValue = _particleSensor.getRed();
        _spo2_calculator.update(_irValue, _redValue);
        _particleSensor.nextSample();
    }

    _heartRate = _spo2_calculator.get_heart_rate();
    _spO2 = _spo2_calculator.get_spo2();
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
    // A simple check. For a real product, a more robust finger detection
    // algorithm would be needed (e.g., checking signal quality).
    return (_irValue > 50000);
}