#include <Arduino.h>
#include "config.h"
#include <GlucoseCalculator.h>
#include <GlucosePredictor.h>

// 1. 引入我们新建的蓝牙控制器头文件
#include "BluetoothController.h" 
#include "LedController.h"
#include "DemodulatorController.h"


void setup() {
  Serial.begin(SERIAL_BAUD_RATE);
  Serial.println("\n--- Non-invasive Glucose Monitor with Prediction ---");

  // 初始化I2C
  Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL, I2C_CLOCK_SPEED);

  // 初始化HAL层
  LedController::getInstance().begin();
  DemodulatorController::getInstance().begin();
  SignalReader::getInstance().begin();
  Dht22Controller::getInstance().begin();
  if (!Max30102Controller::getInstance().begin()) {
    Serial.println("FATAL: MAX30102 sensor not found!"); while (1);
  }

  // 初始化Core层
  GlucoseCalculator::getInstance().begin();
  
  // 初始化Prediction层
  if (!GlucosePredictor::getInstance().begin()) {
      Serial.println("FATAL: Failed to initialize TensorFlow Lite!"); while(1);
  }

  // 2. 初始化蓝牙控制器，并设置设备名称
  BluetoothController::getInstance().begin("ESP32-Glucose-Monitor"); 

  // 开启信号源
  LedController::getInstance().startPulsing();
  DemodulatorController::getInstance().start();

  Serial.println("System ready. Place your finger on the sensor.");
}

void loop() {
  GlucoseCalculator::Status status = GlucoseCalculator::getInstance().performMeasurement();

  if (status == GlucoseCalculator::Status::SUCCESS) {
    // --- 步骤 1: 获取所有传感器和计算数据 ---
    float glucose = GlucoseCalculator::getInstance().getLatestGlucoseValue();
    float heartRate = Max30102Controller::getInstance().getHeartRate();
    float spO2 = Max30102Controller::getInstance().getSpO2();

    // --- 步骤 2: 在串口监视器打印调试信息 ---
    Serial.print("Glucose: "); Serial.print(glucose, 2);
    Serial.print(" mg/dL | HR: "); Serial.print(heartRate, 1);
    Serial.print(" bpm | SpO2: "); Serial.print(spO2, 1); Serial.print("%");

    // --- 步骤 3: 通过蓝牙发送实时数据 ---
    // 获取蓝牙控制器实例
    BluetoothController& ble = BluetoothController::getInstance();
    if (ble.isDeviceConnected()) {
        ble.updateGlucose(glucose);
        ble.updateHeartRate(heartRate);
        ble.updateSpO2(spO2);
    }
    
    // --- 步骤 4: 处理并发送预测数据 ---
    GlucosePredictor::getInstance().addGlucoseReading(glucose);
    if (GlucosePredictor::getInstance().isReadyToPredict()) {
      // 注意: 当前 predict() 返回单个浮点数。
      // 将来您实现曲线预测后，它会返回一个浮点数数组。
      float predicted_glucose = GlucosePredictor::getInstance().predict();
      
      Serial.print(" | Predicted: "); Serial.print(predicted_glucose, 2);
      
      // 通过蓝牙发送预测数据
      // 为了兼容，我们暂时将这个单点预测值作为只有一个元素的“曲线”来发送
      if (ble.isDeviceConnected()) {
          ble.updatePredictionCurve(&predicted_glucose, 1);
      }
    } else {
      Serial.print(" | Collecting data for prediction...");
    }
    Serial.println(); // 换行

  } else if (status == GlucoseCalculator::Status::ERROR_NO_FINGER) {
    Serial.println("No finger detected. Please place your finger on the sensor.");
  }

  delay(2000); // 每2秒测量一次
}