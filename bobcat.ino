#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <ESP32Servo360.h>

// UUIDs dla BLE
#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

// Obiekty BLE
BLEServer* pServer = nullptr;
BLECharacteristic* pCharacteristic = nullptr;

// Obiekty serwomechanizmów
ESP32Servo360 leftServo;
ESP32Servo360 rightServo;

// Piny serwomechanizmów
const int leftServoPin = 4;
const int rightServoPin = 5;

// Funkcja mapująca wartości z zakresu 0–100 na -100 do 100
int mapJoystickValue(int value) {
  return map(value, 0, 100, -100, 100);
}

// Callback dla odbierania danych BLE
class MyCallbacks: public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) override {
    std::string rxValue = pCharacteristic->getValue();
    if (rxValue.length() < 3) return;

    char prefix = rxValue[0];
    int value = atoi(rxValue.substr(1).c_str());
    int speed = mapJoystickValue(value);

    switch (prefix) {
      case 'A': // Lewy joystick poziomo
        leftServo.setSpeed(speed);
        break;
      case 'B': // Lewy joystick pionowo
        rightServo.setSpeed(speed);
        break;
      case 'C': // Prawy joystick poziomo
        // Można dodać dodatkowe funkcje
        break;
      case 'D': // Prawy joystick pionowo
        // Można dodać dodatkowe funkcje
        break;
    }
  }
};

void setup() {
  Serial.begin(115200);

  // Inicjalizacja serwomechanizmów
  leftServo.attach(leftServoPin);
  rightServo.attach(rightServoPin);

  // Inicjalizacja BLE
  BLEDevice::init("ESP32-C3 Excavator");
  pServer = BLEDevice::createServer();
  BLEService *pService = pServer->createService(SERVICE_UUID);
  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_WRITE
                    );
  pCharacteristic->setCallbacks(new MyCallbacks());
  pService->start();
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->start();
}

void loop() {
  // Główna pętla może być pusta, ponieważ sterowanie odbywa się przez BLE
}
