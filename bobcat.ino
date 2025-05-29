#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <ESP32Servo.h>

// BLE UUIDs
#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

// Serwa
Servo servoLeftDrive;   // 360°
Servo servoRightDrive;  // 360°
Servo servoArm;         // 180°
Servo servoBucket;      // 180°

// Piny
#define PIN_LEFT_DRIVE   13
#define PIN_RIGHT_DRIVE  12
#define PIN_ARM          14
#define PIN_BUCKET       15

// Ustawienia joysticka
const int JOYSTICK_MIN = 0;
const int JOYSTICK_MAX = 95;
const int JOYSTICK_MID = 50;

class MyCallbacks: public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    std::string rx = pCharacteristic->getValue();
    if (rx.length() < 2) return;

    char prefix = rx[0];
    int value = atoi(rx.substr(1).c_str());

    switch (prefix) {
      case 'C': case 'D':  // jazda (prawy joystick)
        handleDrive(value);
        break;
      case 'A':  // Ramię
        servoArm.write(map(value, JOYSTICK_MIN, JOYSTICK_MAX, 0, 180));
        break;
      case 'B':  // Łyżka / chwytak
        servoBucket.write(map(value, JOYSTICK_MIN, JOYSTICK_MAX, 0, 180));
        break;
    }
  }

  void handleDrive(int val) {
    int speed = map(val, JOYSTICK_MIN, JOYSTICK_MAX, -90, 90);  // 50 = stop
    int leftPWM = 90 + speed;
    int rightPWM = 90 - speed;

    servoLeftDrive.write(constrain(leftPWM, 0, 180));
    servoRightDrive.write(constrain(rightPWM, 0, 180));
  }
};

void setup() {
  Serial.begin(115200);

  // Serwa
  servoLeftDrive.attach(PIN_LEFT_DRIVE);
  servoRightDrive.attach(PIN_RIGHT_DRIVE);
  servoArm.attach(PIN_ARM);
  servoBucket.attach(PIN_BUCKET);

  // BLE
  BLEDevice::init("ESP32-Excavator");
  BLEServer *pServer = BLEDevice::createServer();
  BLEService *pService = pServer->createService(SERVICE_UUID);

  BLECharacteristic *pCharacteristic = pService->createCharacteristic(
    CHARACTERISTIC_UUID,
    BLECharacteristic::PROPERTY_WRITE
  );
  pCharacteristic->setCallbacks(new MyCallbacks());

  pService->start();
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->start();

  Serial.println("BLE Excavator Ready");
}

void loop() {
  // nic
}
