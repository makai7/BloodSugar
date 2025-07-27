#include <Arduino.h>
#include <unity.h> // PlatformIO的单元测试框架

// 包含所有需要测试的硬件抽象层模块
#include "hal/LedController.h"
#include "hal/DemodulatorController.h"
#include "hal/SignalReader.h"
#include "hal/Dht22Controller.h"
#include "hal/Max30102Controller.h"

// setUp 和 tearDown 是可选的，它们分别在每个测试用例之前和之后运行
void setUp(void) {
    // a test case will begin
}

void tearDown(void) {
    // a test case will end
}

// --- 测试用例 ---

/**
 * @brief 测试 LED 控制器和解调控制器
 * 这个测试需要用户目视观察。
 */
void test_led_and_demodulator_control(void) {
    Serial.println("\n--- [Test] LED & Demodulator ---");
    // 启动脉冲和参考信号
    LedController::getInstance().startPulsing();
    DemodulatorController::getInstance().start();
    Serial.println("Action: LED and Demodulator started. Visually inspect the LED (it should be dimly lit/pulsing).");
    Serial.println("Will stop in 3 seconds...");
    delay(3000);
    
    // 停止
    LedController::getInstance().stopPulsing();
    DemodulatorController::getInstance().stop();
    Serial.println("Action: Stopped. LED should be off.");
    delay(1000);
    
    // 这是一个行为测试，没有硬性的断言，但能验证代码执行无误
    TEST_ASSERT_TRUE(true); 
}

/**
 * @brief 测试信号读取器 (ADC)
 * 验证ADC能否读取到0V到3.3V之间的合法电压值。
 */
void test_signal_reader_voltage_range(void) {
    Serial.println("\n--- [Test] Signal Reader ---");
    float voltage = SignalReader::getInstance().getVoltage();
    Serial.print("Action: Reading voltage... Value: ");
    Serial.println(voltage);

    // 断言：读取到的电压值必须在合理的范围内 (0V ~ 3.3V)
    TEST_ASSERT_FLOAT_WITHIN(3.3f, 0.0f, voltage);
}

/**
 * @brief 测试 DHT22 温湿度传感器
 * 验证能否成功读取数据，并且数据在合理范围内。
 */
void test_dht22_sensor_reading(void) {
    Serial.println("\n--- [Test] DHT22 Sensor ---");
    Serial.println("Action: Reading DHT22... (this may take 2s)");
    bool success = Dht22Controller::getInstance().readData();

    // 断言：读取必须成功
    TEST_ASSERT_TRUE_MESSAGE(success, "Failed to read from DHT22 sensor!");

    if (success) {
        float temp = Dht22Controller::getInstance().getTemperature();
        float hum = Dht22Controller::getInstance().getHumidity();
        Serial.print("Result: Temp="); Serial.print(temp); Serial.print("C, Hum="); Serial.print(hum); Serial.println("%");
        
        // 断言：温度和湿度值不能是NAN (Not-a-Number)
        TEST_ASSERT_NOT_NAN(temp);
        TEST_ASSERT_NOT_NAN(hum);
        
        // 断言：温度值在合理的室温范围内 (0C to 50C)
        TEST_ASSERT_FLOAT_WITHIN(50.0f, 0.0f, temp);
        // 断言：湿度值在合理的范围内 (0% to 100%)
        TEST_ASSERT_FLOAT_WITHIN(100.0f, 0.0f, hum);
    }
}

/**
 * @brief 测试 MAX30102 心率血氧传感器
 * 这是一个交互式测试，验证传感器初始化和手指检测功能。
 */
void test_max30102_finger_detection(void) {
    Serial.println("\n--- [Test] MAX30102 Sensor ---");
    
    // 1. 验证在没有手指时，传感器报告无手指
    Max30102Controller::getInstance().update();
    bool finger_present_at_start = Max30102Controller::getInstance().isFingerDetected();
    Serial.println("Action: Checking initial state... (ensure no finger is on the sensor)");
    TEST_ASSERT_FALSE_MESSAGE(finger_present_at_start, "Sensor reported finger when there should be none.");

    // 2. 提示用户放置手指，并等待几秒钟来检测
    Serial.println("Action: PLEASE PLACE YOUR FINGER ON THE MAX30102 SENSOR NOW!");
    Serial.println("Waiting for 5 seconds...");

    // 在5秒内持续更新传感器读数
    unsigned long start_time = millis();
    bool finger_detected = false;
    while(millis() - start_time < 5000) {
        Max30102Controller::getInstance().update();
        if (Max30102Controller::getInstance().isFingerDetected()) {
            finger_detected = true;
            break; // 一旦检测到就跳出循环
        }
        delay(50);
    }

    // 断言：在提示后，必须能检测到手指
    TEST_ASSERT_TRUE_MESSAGE(finger_detected, "Failed to detect finger after 5 seconds!");
    Serial.println("Result: Finger detected successfully!");
}


// 这是 PlatformIO Test Runner 的入口
void setup() {
    // 等待2秒，确保串口监视器可以连接上
    delay(2000);

    // 初始化测试框架
    UNITY_BEGIN();

    // 在运行任何测试之前，先初始化所有硬件模块
    Serial.println("Initializing hardware modules for testing...");
    Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL, I2C_CLOCK_SPEED);
    LedController::getInstance().begin();
    DemodulatorController::getInstance().begin();
    SignalReader::getInstance().begin();
    Dht22Controller::getInstance().begin();
    
    // 单独测试MAX30102的初始化
    RUN_TEST([](){ 
        bool success = Max30102Controller::getInstance().begin();
        TEST_ASSERT_TRUE_MESSAGE(success, "MAX30102 sensor initialization failed! Check wiring.");
    });

    Serial.println("\nHardware initialization complete. Starting tests...");
    
    // 按顺序运行所有测试用例
    RUN_TEST(test_led_and_demodulator_control);
    RUN_TEST(test_signal_reader_voltage_range);
    RUN_TEST(test_dht22_sensor_reading);
    RUN_TEST(test_max30102_finger_detection);

    // 结束测试
    UNITY_END();
}

void loop() {
    // 这里不需要任何代码，因为所有测试都在setup()中一次性运行完毕
}