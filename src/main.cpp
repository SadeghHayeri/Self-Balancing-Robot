#include <Arduino.h>
#include <Arduino_FreeRTOS.h>
#include "mpu6050.h"
#include <L298N.h>

////////////// PINS //////////////
#define EN 13

#define BLUETOOTH_WRITE_PIN 2
#define BLUETOOTH_READ_PIN 3

#define RIGHT_MOTOR_SPEED_PIN 8
#define RIGHT_MOTOR_DIR_PIN 9
#define RIGHT_MOTOR_SHAFT_PIN -1

#define LEFT_MOTOR_SPEED_PIN 10
#define LEFT_MOTOR_DIR_PIN 11
#define LEFT_MOTOR_SHAFT_PIN -1

#define ULTRASONIC_ECHO_PIN 4
#define ULTRASONIC_TRIG_PIN 5
///////////////////////////////////

#define T 0.3

L298N rightMotor(EN, RIGHT_MOTOR_DIR_PIN, RIGHT_MOTOR_SPEED_PIN);
L298N leftMotor(EN, LEFT_MOTOR_DIR_PIN, LEFT_MOTOR_SPEED_PIN);

double ROLL = 0;
double PITCH = 0;

double TARGET_PITCH = 0;

int RIGHT_MOTOR_SPEED = 0;
int LEFT_MOTOR_SPEED = 0;

void setSpeed(L298N motor, int speed) {
    if(speed >= 0) {
        motor.forward();
        motor.setSpeed(speed);
    } else {
        motor.backward();
        motor.setSpeed(-1 * speed);
    }
}

void pidController(void *pvParameters __attribute__((unused))) {
    while (true) {
        double angelDiff = PITCH - TARGET_PITCH;

        setSpeed(rightMotor, T * angelDiff);
        setSpeed(leftMotor, T * angelDiff);
    }
}

inline void initSerial() {
    Serial.begin(9600);
    while (!Serial);
}

void printRoll(void *pvParameters __attribute__((unused))) {
    Serial.println(ROLL);
    vTaskDelay(1);
}

void setup() {
    initSerial();
    TaskCalcAngleSetup();

    xTaskCreate (
        TaskCalcAngle
        ,(const portCHAR *)"MPU6050"
        ,128
        ,NULL
        ,0
        ,NULL
    );

    xTaskCreate (
        pidController
        ,(const portCHAR *)"PID"
        ,128
        ,NULL
        ,0
        ,NULL
    );

    xTaskCreate (
        printRoll
        ,(const portCHAR *)"print"
        ,128
        ,NULL
        ,0
        ,NULL
    );
}

void loop() {
    // motor.setSpeed(100);
    // motor.backward();
}
