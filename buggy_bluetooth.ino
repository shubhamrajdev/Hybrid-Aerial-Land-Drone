#include "BluetoothSerial.h"

HardwareSerial HC05(2); // Using UART2
BluetoothSerial BT_Serial;

#define enA 13  // Enable1 L298 Pin enA
#define in1 12  // Motor1 L298 Pin in1
#define in2 14  // Motor1 L298 Pin in2
#define in3 27  // Motor2 L298 Pin in3
#define in4 26  // Motor2 L298 Pin in4
#define enB 25  // Enable2 L298 Pin enB

int bt_data; // variable to receive data from the serial port
int Speed = 130;

void setup() { // put your setup code here, to run once
  pinMode(enA, OUTPUT); // declare as output for L298 Pin enA
  pinMode(in1, OUTPUT); // declare as output for L298 Pin in1
  pinMode(in2, OUTPUT); // declare as output for L298 Pin in2
  pinMode(in3, OUTPUT); // declare as output for L298 Pin in3
  pinMode(in4, OUTPUT); // declare as output for L298 Pin in4
  pinMode(enB, OUTPUT); // declare as output for L298 Pin enB

  Serial.begin(115200);
  BT_Serial.begin("ESP32_Master"); // Bluetooth name
  Serial.println("ESP32 ready.");
  HC05.begin(9600, SERIAL_8N1, 22, 23); // RX=22, TX=23
  Serial.println("Master-Slave ready.");
}

void loop() {
  if (HC05.available()) {
    String feedback = HC05.readStringUntil('\n');
    feedback.trim();
    Serial.println("From Slave: " + feedback);
    BT_Serial.println("Slave: " + feedback);
  }

  if (BT_Serial.available() > 0) {
    bt_data = BT_Serial.read();
    Serial.print("Received from Phone: ");
    Serial.println(bt_data);
    HC05.println(bt_data); // Forward to slave
    
    if (bt_data > 20) { 
      Speed = bt_data; 
    }

    analogWrite(enA, Speed); // Write The Duty Cycle 0 to 255 Enable Pin A for Motor1 Speed
    analogWrite(enB, Speed); // Write The Duty Cycle 0 to 255 Enable Pin B for Motor2 Speed
    
    if (bt_data == 1) {
      forward();
    } // if the bt_data is '1' the DC motor will go forward
    else if (bt_data == 2) {
      backward();
    } // if the bt_data is '2' the motor will Reverse
    else if (bt_data == 3) {
      turnLeft();
    } // if the bt_data is '3' the motor will turn left
    else if (bt_data == 4) {
      turnRight();
    } // if the bt_data is '4' the motor will turn right
    else if (bt_data == 5) {
      Stop();
    } // if the bt_data '5' the motor will Stop
    else if (bt_data == 6) {
      turnLeft(); 
      delay(400); 
      bt_data = 5;
    }
    else if (bt_data == 7) {
      turnRight(); 
      delay(400); 
      bt_data = 5;
    }
  }
}

void forward() {
  digitalWrite(in1, LOW);  // RIGHT Motor backward Pin
  digitalWrite(in2, HIGH); // RIGHT Motor forward Pin
  digitalWrite(in3, LOW);  // LEFT Motor BACKWARD Pin
  digitalWrite(in4, HIGH); // LEFT Motor FORWARD Pin
}

void backward() {
  digitalWrite(in1, HIGH);
  digitalWrite(in2, LOW);
  digitalWrite(in3, HIGH);
  digitalWrite(in4, LOW);
}

void turnRight() {
  digitalWrite(in1, HIGH);
  digitalWrite(in2, LOW);
  digitalWrite(in3, LOW);
  digitalWrite(in4, HIGH);
}

void turnLeft() {
  digitalWrite(in1, LOW);
  digitalWrite(in2, HIGH);
  digitalWrite(in3, HIGH);
  digitalWrite(in4, LOW);
}

void Stop() {
  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW);
  digitalWrite(in3, LOW);
  digitalWrite(in4, LOW);
}
