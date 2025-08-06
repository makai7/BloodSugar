#include "BluetoothController.h"
#include <BLE2902.h>
#include <Arduino.h> // For String and dtostrf

// You can generate your own unique UUIDs using an online generator
#define SERVICE_UUID           "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define HEARTRATE_CHAR_UUID    "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define SPO2_CHAR_UUID         "c8c36394-8f48-472e-874b-632467a83a21"
#define GLUCOSE_CHAR_UUID      "9e3b7e4c-6a8a-479c-897c-b35d37af2137"
#define PREDICTION_CHAR_UUID   "a2e8a15a-e0a9-4888-a8a5-c344a178d076"

// --- ServerCallbacks Implementation ---
BluetoothController::ServerCallbacks::ServerCallbacks(bool& connectedFlag) : connectedFlag(connectedFlag) {}

void BluetoothController::ServerCallbacks::onConnect(BLEServer* pServer) {
    connectedFlag = true;
    Serial.println("BLE Client Connected");
}

void BluetoothController::ServerCallbacks::onDisconnect(BLEServer* pServer) {
    connectedFlag = false;
    Serial.println("BLE Client Disconnected");
    // Restart advertising to allow new connections
    pServer->getAdvertising()->start();
}

// --- BluetoothController Implementation ---
BluetoothController& BluetoothController::getInstance() {
    static BluetoothController instance;
    return instance;
}

BluetoothController::BluetoothController() : pServer(nullptr), deviceConnected(false) {}

void BluetoothController::begin(const std::string& deviceName) {
    BLEDevice::init(deviceName);
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new ServerCallbacks(deviceConnected));
    
    BLEService *pService = pServer->createService(SERVICE_UUID);

    // Create Heart Rate Characteristic
    pHeartRateCharacteristic = pService->createCharacteristic(HEARTRATE_CHAR_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
    pHeartRateCharacteristic->addDescriptor(new BLE2902());

    // Create SpO2 Characteristic
    pSpO2Characteristic = pService->createCharacteristic(SPO2_CHAR_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
    pSpO2Characteristic->addDescriptor(new BLE2902());
    
    // Create Glucose Characteristic
    pGlucoseCharacteristic = pService->createCharacteristic(GLUCOSE_CHAR_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
    pGlucoseCharacteristic->addDescriptor(new BLE2902());

    // Create Prediction Curve Characteristic
    pPredictionCharacteristic = pService->createCharacteristic(PREDICTION_CHAR_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
    pPredictionCharacteristic->addDescriptor(new BLE2902());
    
    pService->start();
    
    // Start advertising
    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(true);
    BLEDevice::startAdvertising();
    
    Serial.println("Bluetooth service started. Waiting for a client connection...");
}

bool BluetoothController::isDeviceConnected() {
    return deviceConnected;
}

// Helper function to convert float to string and update characteristic
void updateCharacteristic(BLECharacteristic* pChar, float value) {
    char buffer[10];
    dtostrf(value, 4, 2, buffer); // Convert float to string, 4 total width, 2 decimal places
    pChar->setValue(buffer);
    pChar->notify();
}

void BluetoothController::updateHeartRate(float heartRate) {
    if (deviceConnected) {
        updateCharacteristic(pHeartRateCharacteristic, heartRate);
    }
}

void BluetoothController::updateSpO2(float spO2) {
    if (deviceConnected) {
        updateCharacteristic(pSpO2Characteristic, spO2);
    }
}

void BluetoothController::updateGlucose(float glucose) {
    if (deviceConnected) {
        updateCharacteristic(pGlucoseCharacteristic, glucose);
    }
}

void BluetoothController::updatePredictionCurve(float* curveData, int curveSize) {
    if (deviceConnected) {
        String payload = "";
        for (int i = 0; i < curveSize; i++) {
            payload += String(curveData[i], 1); // Append value with 1 decimal place
            if (i < curveSize - 1) {
                payload += ","; // Use comma as a separator
            }
        }
        pPredictionCharacteristic->setValue(payload.c_str());
        pPredictionCharacteristic->notify();
    }
}