#include <Wire.h>
#include <L298N.h>
#include <AutoPID.h>
#include <SoftwareSerial.h>
//#include <HCSR04.h>
#include <stdio.h>

////////////// PINS //////////////
#define EN 13

#define BLUETOOTH_RX_PIN 3
#define BLUETOOTH_TX_PIN 4

#define RIGHT_MOTOR_IN1_PIN 8
#define RIGHT_MOTOR_IN2_PIN 7
#define RIGHT_MOTOR_EN_PIN 6
#define RIGHT_MOTOR_SHAFT_PIN -1

#define LEFT_MOTOR_IN1_PIN 10
#define LEFT_MOTOR_IN2_PIN 9
#define LEFT_MOTOR_EN_PIN 11
#define LEFT_MOTOR_SHAFT_PIN -1
///////////////////////////////////

////// pid settings and gains //////
#define MOTOR_OUTPUT_MAX 255
#define MOTOR_OUTPUT_MIN -255

double MOTOR_PID_KP = 25;
double MOTOR_PID_KI = 0;
double MOTOR_PID_KD = 45;

double angle, pwm;
double setPoint = 0;
double DIFF_ANGLE = 1.25;
////////////////////////////////////

L298N rightMotor(RIGHT_MOTOR_EN_PIN, RIGHT_MOTOR_IN1_PIN, RIGHT_MOTOR_IN2_PIN);
L298N leftMotor(LEFT_MOTOR_EN_PIN, LEFT_MOTOR_IN1_PIN, LEFT_MOTOR_IN2_PIN);

AutoPID pwmPID(&angle, &setPoint, &pwm, MOTOR_OUTPUT_MIN, MOTOR_OUTPUT_MAX, MOTOR_PID_KP, MOTOR_PID_KI, MOTOR_PID_KD);

SoftwareSerial bluetoothSerial(BLUETOOTH_RX_PIN, BLUETOOTH_TX_PIN);

inline void setSpeed(L298N& motor, int speed) {
    motor.setSpeed(min(abs(speed), 255));
    if(speed >= 0) {
        motor.forward();
    } else {
        motor.backward();
    }
}

void setup() {
  Serial.begin(115200);
  bluetoothSerial.begin(115200);
  
  delay(100);
  pwmPID.setTimeStep(1);
}

char serialChar() {
  while(!Serial.available());
  return Serial.read();
}

String readUntil(char terminator) {
  String data = "";
  char c;
  do {
    c = serialChar();
    data += c;
  } while(c != terminator);
  return data;
}

String readData() {
  String data = "";
  while(data.length() != 8) {
    data = readUntil('U');
  }
  return data;
}

float MPUData() {
  String data = readData();
  float pitch = (data[1] << 8 | (data[2] & 0x00FF)) / 100.0;
  float heading = (data[3] << 8 | (data[4] & 0x00FF)) / 100.0;
  float roll = (data[5] << 8 | (data[6] & 0x00FF)) / 100.0;

  if(roll > 0)
    return roll - 180;
  else
    return roll + 180;
}
/*
void checkForCommands() {
  if(bluetoothSerial.available()) {
    char command = bluetoothSerial.read();
    
    switch(command) {
      /////// MOTOR //////
      case 'q':
        MOTOR_PID_KP += 1;
        break;
      case 'a':
        if(MOTOR_PID_KP > 0)
          MOTOR_PID_KP -= 1;
        break;
      case 'w':
        MOTOR_PID_KI += 1;
        break;
      case 's':
        if(MOTOR_PID_KI > 0)
          MOTOR_PID_KI -= 1;
        break;
      case 'e':
        MOTOR_PID_KD += 1;
        break;
      case 'd':
        if(MOTOR_PID_KD > 0)
          MOTOR_PID_KD -= 1;
        break;
        
      //// DIFF ANDLE ////
      case 'z':
        DIFF_ANGLE += 0.1;
        break;
      case 'x':
        DIFF_ANGLE -= 0.1;
        break;
    }
    
    pwmPID.setGains(MOTOR_PID_KP, MOTOR_PID_KI, MOTOR_PID_KD);
    Serial.print(angle);Serial.print(" - ");Serial.print(DIFF_ANGLE);Serial.print(" MP:");Serial.print(MOTOR_PID_KP);Serial.print(" MI:");Serial.print(MOTOR_PID_KI);Serial.print(" MD:");Serial.println(MOTOR_PID_KD);
  }
}
*/

void loop() {
  angle = MPUData() - DIFF_ANGLE;
  pwmPID.run();
  setSpeed(rightMotor, pwm);
  setSpeed(leftMotor, pwm);
//  checkForCommands();
//  Serial.println(angle);
}
