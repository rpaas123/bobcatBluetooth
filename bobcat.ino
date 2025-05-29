#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <ESP32Servo.h>  // Kompatybilna z ESP32

// Serwa
Servo leftWheel;
Servo rightWheel;
Servo bucket;
Servo arm;

// Piny
#define LEFT_WHEEL_PIN 3
#define RIGHT_WHEEL_PIN 4
#define BUCKET_PIN 5
#define ARM_PIN 6

// BLE
BLECharacteristic *pCharacteristic;
bool deviceConnected = false;

// Dane joysticków
int joyA = 50;
int joyB = 50;
int joyC = 50;
int joyD = 50;

class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
  }

  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
  }
};

class MyCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    String input = String(pCharacteristic->getValue().c_str());

    if (input.length() > 0) {
      Serial.print("Received: ");
      Serial.println(input);

      if (input.startsWith("A")) {
        joyA = constrain(input.substring(1).toInt(), 0, 95);
      } else if (input.startsWith("B")) {
        joyB = constrain(input.substring(1).toInt(), 0, 95);
      } else if (input.startsWith("C")) {
        joyC = constrain(input.substring(1).toInt(), 0, 95);
      } else if (input.startsWith("D")) {
        joyD = constrain(input.substring(1).toInt(), 0, 95);
      }
    }
  }
};

void setup() {
  Serial.begin(115200);

  // Serwa - ustawienia PWM dla ESP32
  leftWheel.setPeriodHertz(50);
  rightWheel.setPeriodHertz(50);
  bucket.setPeriodHertz(50);
  arm.setPeriodHertz(50);

  leftWheel.attach(LEFT_WHEEL_PIN, 500, 2500);
  rightWheel.attach(RIGHT_WHEEL_PIN, 500, 2500);
  bucket.attach(BUCKET_PIN, 500, 2500);
  arm.attach(ARM_PIN, 500, 2500);

  // BLE setup
  BLEDevice::init("BobcatESP32");
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  BLEService *pService = pServer->createService(BLEUUID((uint16_t)0xFFE0));
  pCharacteristic = pService->createCharacteristic(
                      BLEUUID((uint16_t)0xFFE1),
                      BLECharacteristic::PROPERTY_READ |
                      BLECharacteristic::PROPERTY_WRITE
                    );

  pCharacteristic->setCallbacks(new MyCallbacks());
  pCharacteristic->setValue("Bobcat Ready");
  pService->start();

  BLEAdvertising *pAdvertising = pServer->getAdvertising();
  pAdvertising->start();
}

void loop() {
  // --- TANK DRIVE ---
  int speed = map(joyD, 0, 95, -100, 100); // przód/tył
  int turn  = map(joyC, 0, 95, -100, 100); // lewo/prawo

  int leftMotor  = speed + turn;
  int rightMotor = speed - turn;

  leftMotor = constrain(leftMotor, -100, 100);
  rightMotor = constrain(rightMotor, -100, 100);

  int leftPulse  = map(leftMotor, -100, 100, 0, 180);
  int rightPulse = map(rightMotor, -100, 100, 180, 0); // odwrotna strona

  leftWheel.write(leftPulse);
  rightWheel.write(rightPulse);

  // --- ARM + BUCKET ---
  int armPos = map(joyA, 0, 95, 0, 180);
  int bucketPos = map(joyB, 0, 95, 0, 180);

  arm.write(armPos);
  bucket.write(bucketPos);

  delay(20);
}
