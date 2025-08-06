#ifndef BLUETOOTH_CONTROLLER_H
#define BLUETOOTH_CONTROLLER_H

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <string>

// Forward declaration to avoid including the whole GlucosePredictor header
class GlucosePredictor;

class BluetoothController {
public:
    static BluetoothController& getInstance();
    BluetoothController(const BluetoothController&) = delete;
    BluetoothController& operator=(const BluetoothController&) = delete;

    void begin(const std::string& deviceName = "BloodSugar-Monitor");
    void updateHeartRate(float heartRate);
    void updateSpO2(float spO2);
    void updateGlucose(float glucose);
    void updatePredictionCurve(float* curveData, int curveSize);
    bool isDeviceConnected();

private:
    BluetoothController();

    BLEServer* pServer;
    BLECharacteristic* pHeartRateCharacteristic;
    BLECharacteristic* pSpO2Characteristic;
    BLECharacteristic* pGlucoseCharacteristic;
    BLECharacteristic* pPredictionCharacteristic;
    
    bool deviceConnected;

    // Callback class to handle connect/disconnect events
    class ServerCallbacks : public BLEServerCallbacks {
    public:
        ServerCallbacks(bool& connectedFlag);
        void onConnect(BLEServer* pServer) override;
        void onDisconnect(BLEServer* pServer) override;
    private:
        bool& connectedFlag;
    };
};

#endif // BLUETOOTH_CONTROLLER_H