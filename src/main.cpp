#include <Arduino.h>
#include "config.h"
#include <GlucoseCalculator.h>
#include <GlucosePredictor.h> // <-- 引入预测器

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

  // 开启信号源
  LedController::getInstance().startPulsing();
  DemodulatorController::getInstance().start();

  Serial.println("System ready. Place your finger on the sensor.");
}

void loop() {
  GlucoseCalculator::Status status = GlucoseCalculator::getInstance().performMeasurement();

  if (status == GlucoseCalculator::Status::SUCCESS) {
    float glucose = GlucoseCalculator::getInstance().getLatestGlucoseValue();
    Serial.print("Measured: "); Serial.print(glucose, 2);

    // 将有效的测量值添加到预测器的历史记录中
    GlucosePredictor::getInstance().addGlucoseReading(glucose);

    // 如果数据足够，就进行预测
    if (GlucosePredictor::getInstance().isReadyToPredict()) {
      float predicted_glucose = GlucosePredictor::getInstance().predict();
      Serial.print(" | Predicted: "); Serial.print(predicted_glucose, 2);
    } else {
      Serial.print(" | Collecting data for prediction...");
    }
    Serial.println();

  } else if (status == GlucoseCalculator::Status::ERROR_NO_FINGER) {
    Serial.println("No finger detected.");
  }

  delay(2000); // 每2秒测量一次
}