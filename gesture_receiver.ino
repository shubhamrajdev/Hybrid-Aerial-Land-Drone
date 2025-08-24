#include <WiFi.h>
#include <esp_now.h>

// Movement commands
#define STOP 0
#define FORWARD 1
#define BACKWARD 2
#define LEFT 3
#define RIGHT 4
#define FORWARD_LEFT 5
#define FORWARD_RIGHT 6
#define BACKWARD_LEFT 7
#define BACKWARD_RIGHT 8
#define TURN_LEFT 9
#define TURN_RIGHT 10

// Motor indexes
#define BACK_RIGHT_MOTOR 0
#define BACK_LEFT_MOTOR 1
#define FRONT_RIGHT_MOTOR 2
#define FRONT_LEFT_MOTOR 3

// Motor Pins struct
struct MOTOR_PINS
{
  int pinIN1;
  int pinIN2;
  int pinEn;
  int pwmChannel;
};

// Motor pins mapping with your exact pins
MOTOR_PINS motorPins[4] = {
  // Back Right Motor - uses IN3, IN4, ENB
  {12, 33, 26, 0},  // BACK_RIGHT_MOTOR

  // Back Left Motor - uses IN1, IN2, ENA
  {27, 14, 25, 1},  // BACK_LEFT_MOTOR

  // Front Right Motor - uses IN3, IN4, ENB (same as back right)
  {12, 33, 26, 2},  // FRONT_RIGHT_MOTOR

  // Front Left Motor - uses IN1, IN2, ENA (same as back left)
  {27, 14, 25, 3}   // FRONT_LEFT_MOTOR
};

const int PWMFreq = 1000;       // 1kHz PWM frequency
const int PWMResolution = 8;    // 8-bit PWM resolution (0-255)
const int MAX_SPEED = 200;      // Max PWM speed (0-255 range)

struct PacketData
{
  byte xAxisValue;
  byte yAxisValue;
  byte zAxisValue;
};

PacketData receiverData;

unsigned long lastRecvTime = 0;
const unsigned long SIGNAL_TIMEOUT = 1000; // 1 second

// Forward declarations
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len);
void processCarMovement(int inputValue);
void rotateMotor(int motorNumber, int motorSpeed);
void setupMotorPins();

void setup() {
  Serial.begin(115200);

  setupMotorPins();

  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    while (true) { delay(1000); } // Halt
  }
  
  esp_now_register_recv_cb(OnDataRecv);

  Serial.println("Setup complete, waiting for commands...");
}

void loop() {
  if (millis() - lastRecvTime > SIGNAL_TIMEOUT) {
    // No data received recently - stop motors
    processCarMovement(STOP);
  }
  delay(10);
}

void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  if (len != sizeof(PacketData)) return;

  memcpy(&receiverData, incomingData, sizeof(receiverData));
  lastRecvTime = millis();

  Serial.printf("Received: X=%d Y=%d Z=%d\n", receiverData.xAxisValue, receiverData.yAxisValue, receiverData.zAxisValue);

  if (receiverData.xAxisValue < 75 && receiverData.yAxisValue < 75) {
    processCarMovement(FORWARD_LEFT);
  }
  else if (receiverData.xAxisValue > 175 && receiverData.yAxisValue < 75) {
    processCarMovement(FORWARD_RIGHT);
  }
  else if (receiverData.xAxisValue < 75 && receiverData.yAxisValue > 175) {
    processCarMovement(BACKWARD_LEFT);
  }
  else if (receiverData.xAxisValue > 175 && receiverData.yAxisValue > 175) {
    processCarMovement(BACKWARD_RIGHT);
  }
  else if (receiverData.zAxisValue > 175) {
    processCarMovement(TURN_RIGHT);
  }
  else if (receiverData.zAxisValue < 75) {
    processCarMovement(TURN_LEFT);
  }
  else if (receiverData.yAxisValue < 75) {
    processCarMovement(FORWARD);
  }
  else if (receiverData.yAxisValue > 175) {
    processCarMovement(BACKWARD);
  }
  else if (receiverData.xAxisValue > 175) {
    processCarMovement(RIGHT);
  }
  else if (receiverData.xAxisValue < 75) {
    processCarMovement(LEFT);
  }
  else {
    processCarMovement(STOP);
  }
}

void processCarMovement(int inputValue) {
  switch(inputValue) {
    case FORWARD:
      rotateMotor(FRONT_RIGHT_MOTOR, MAX_SPEED);
      rotateMotor(BACK_RIGHT_MOTOR, MAX_SPEED);
      rotateMotor(FRONT_LEFT_MOTOR, MAX_SPEED);
      rotateMotor(BACK_LEFT_MOTOR, MAX_SPEED);
      break;
      
    case BACKWARD:
      rotateMotor(FRONT_RIGHT_MOTOR, -MAX_SPEED);
      rotateMotor(BACK_RIGHT_MOTOR, -MAX_SPEED);
      rotateMotor(FRONT_LEFT_MOTOR, -MAX_SPEED);
      rotateMotor(BACK_LEFT_MOTOR, -MAX_SPEED);
      break;
      
    case LEFT:
      rotateMotor(FRONT_RIGHT_MOTOR, MAX_SPEED);
      rotateMotor(BACK_RIGHT_MOTOR, -MAX_SPEED);
      rotateMotor(FRONT_LEFT_MOTOR, -MAX_SPEED);
      rotateMotor(BACK_LEFT_MOTOR, MAX_SPEED);
      break;
      
    case RIGHT:
      rotateMotor(FRONT_RIGHT_MOTOR, -MAX_SPEED);
      rotateMotor(BACK_RIGHT_MOTOR, MAX_SPEED);
      rotateMotor(FRONT_LEFT_MOTOR, MAX_SPEED);
      rotateMotor(BACK_LEFT_MOTOR, -MAX_SPEED);
      break;
      
    case FORWARD_LEFT:
      rotateMotor(FRONT_RIGHT_MOTOR, MAX_SPEED);
      rotateMotor(BACK_RIGHT_MOTOR, 0);
      rotateMotor(FRONT_LEFT_MOTOR, 0);
      rotateMotor(BACK_LEFT_MOTOR, MAX_SPEED);
      break;
      
    case FORWARD_RIGHT:
      rotateMotor(FRONT_RIGHT_MOTOR, 0);
      rotateMotor(BACK_RIGHT_MOTOR, MAX_SPEED);
      rotateMotor(FRONT_LEFT_MOTOR, MAX_SPEED);
      rotateMotor(BACK_LEFT_MOTOR, 0);
      break;
      
    case BACKWARD_LEFT:
      rotateMotor(FRONT_RIGHT_MOTOR, 0);
      rotateMotor(BACK_RIGHT_MOTOR, -MAX_SPEED);
      rotateMotor(FRONT_LEFT_MOTOR, -MAX_SPEED);
      rotateMotor(BACK_LEFT_MOTOR, 0);
      break;
      
    case BACKWARD_RIGHT:
      rotateMotor(FRONT_RIGHT_MOTOR, -MAX_SPEED);
      rotateMotor(BACK_RIGHT_MOTOR, 0);
      rotateMotor(FRONT_LEFT_MOTOR, 0);
      rotateMotor(BACK_LEFT_MOTOR, -MAX_SPEED);
      break;
      
    case TURN_LEFT:
      rotateMotor(FRONT_RIGHT_MOTOR, MAX_SPEED);
      rotateMotor(BACK_RIGHT_MOTOR, MAX_SPEED);
      rotateMotor(FRONT_LEFT_MOTOR, -MAX_SPEED);
      rotateMotor(BACK_LEFT_MOTOR, -MAX_SPEED);
      break;
      
    case TURN_RIGHT:
      rotateMotor(FRONT_RIGHT_MOTOR, -MAX_SPEED);
      rotateMotor(BACK_RIGHT_MOTOR, -MAX_SPEED);
      rotateMotor(FRONT_LEFT_MOTOR, MAX_SPEED);
      rotateMotor(BACK_LEFT_MOTOR, MAX_SPEED);
      break;
      
    case STOP:
    default:
      rotateMotor(FRONT_RIGHT_MOTOR, 0);
      rotateMotor(BACK_RIGHT_MOTOR, 0);
      rotateMotor(FRONT_LEFT_MOTOR, 0);
      rotateMotor(BACK_LEFT_MOTOR, 0);
      break;
  }
}

void rotateMotor(int motorNumber, int motorSpeed) {
  if (motorSpeed > 0) {
    digitalWrite(motorPins[motorNumber].pinIN1, HIGH);
    digitalWrite(motorPins[motorNumber].pinIN2, LOW);
  } 
  else if (motorSpeed < 0) {
    digitalWrite(motorPins[motorNumber].pinIN1, LOW);
    digitalWrite(motorPins[motorNumber].pinIN2, HIGH);
  } 
  else {
    digitalWrite(motorPins[motorNumber].pinIN1, LOW);
    digitalWrite(motorPins[motorNumber].pinIN2, LOW);
  }

  // PWM speed control (absolute value)
  ledcWrite(motorPins[motorNumber].pwmChannel, abs(motorSpeed));
}

void setupMotorPins() {
  for (int i = 0; i < 4; i++) {
    pinMode(motorPins[i].pinIN1, OUTPUT);
    pinMode(motorPins[i].pinIN2, OUTPUT);

    ledcSetup(motorPins[i].pwmChannel, PWMFreq, PWMResolution);
    ledcAttachPin(motorPins[i].pinEn, motorPins[i].pwmChannel);

    rotateMotor(i, 0); // Stop motors at setup
  }
}