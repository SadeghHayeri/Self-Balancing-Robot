#include <Arduino.h>
#include <Arduino_FreeRTOS.h>
#include <Wire.h>
#include <Kalman.h> // Source: https://github.com/TKJElectronics/KalmanFilter
#include "I2C.h"
#include "mpu6050.h"

Kalman kalmanX; // Create the Kalman instances
Kalman kalmanY;

/* IMU Data */
double accX, accY, accZ;
double gyroX, gyroY, gyroZ;
int16_t tempRaw;

double gyroXangle, gyroYangle; // Angle calculate using the gyro only
double compAngleX, compAngleY; // Calculated angle using a complementary filter
double kalAngleX, kalAngleY; // Calculated angle using a Kalman filter

uint32_t timer;
uint8_t i2cData[14]; // Buffer for I2C data

void TaskCalcAngleSetup() {
    Wire.begin();
#if ARDUINO >= 157
    Wire.setClock(400000UL); // Set I2C frequency to 400kHz
#else
    TWBR = ((F_CPU / 400000UL) - 16) / 2; // Set I2C frequency to 400kHz
#endif

    i2cData[0] = 7; // Set the sample rate to 1000Hz - 8kHz/(7+1) = 1000Hz
    i2cData[1] = 0x00; // Disable FSYNC and set 260 Hz Acc filtering, 256 Hz Gyro filtering, 8 KHz sampling
    i2cData[2] = 0x00; // Set Gyro Full Scale Range to ±250deg/s
    i2cData[3] = 0x00; // Set Accelerometer Full Scale Range to ±2g
    while (i2cWrite(0x19, i2cData, 4, false)); // Write to all four registers at once
    while (i2cWrite(0x6B, 0x01, true)); // PLL with X axis gyroscope reference and disable sleep mode

    while (i2cRead(0x75, i2cData, 1));
    if (i2cData[0] != 0x68) { // Read "WHO_AM_I" register
        Serial.print(F("Error reading sensor"));
        while (1);
    }

    delay(100); // Wait for sensor to stabilize

    /* Set kalman and gyro starting angle */
    while (i2cRead(0x3B, i2cData, 6));
    accX = (int16_t)((i2cData[0] << 8) | i2cData[1]);
    accY = (int16_t)((i2cData[2] << 8) | i2cData[3]);
    accZ = (int16_t)((i2cData[4] << 8) | i2cData[5]);

    ROLL  = atan2(accY, accZ) * RAD_TO_DEG;
    PITCH = atan(-accX / sqrt(accY * accY + accZ * accZ)) * RAD_TO_DEG;

    kalmanX.setAngle(ROLL); // Set starting angle
    kalmanY.setAngle(PITCH);
    gyroXangle = ROLL;
    gyroYangle = PITCH;
    compAngleX = ROLL;
    compAngleY = PITCH;

    timer = micros();
}

void TaskCalcAngle(void *pvParameters __attribute__((unused))) {
    while(true) {
        while (i2cRead(0x3B, i2cData, 14));
        accX = (int16_t)((i2cData[0] << 8) | i2cData[1]);
        accY = (int16_t)((i2cData[2] << 8) | i2cData[3]);
        accZ = (int16_t)((i2cData[4] << 8) | i2cData[5]);
        tempRaw = (int16_t)((i2cData[6] << 8) | i2cData[7]);
        gyroX = (int16_t)((i2cData[8] << 8) | i2cData[9]);
        gyroY = (int16_t)((i2cData[10] << 8) | i2cData[11]);
        gyroZ = (int16_t)((i2cData[12] << 8) | i2cData[13]);;

        double dt = (double)(micros() - timer) / 1000000; // Calculate delta time
        timer = micros();

        // Source: http://www.freescale.com/files/sensors/doc/app_note/AN3461.pdf eq. 25 and eq. 26
        // atan2 outputs the value of -π to π (radians) - see http://en.wikipedia.org/wiki/Atan2
        // It is then converted from radians to degrees
        ROLL  = atan2(accY, accZ) * RAD_TO_DEG;
        PITCH = atan(-accX / sqrt(accY * accY + accZ * accZ)) * RAD_TO_DEG;

        double gyroXrate = gyroX / 131.0; // Convert to deg/s
        double gyroYrate = gyroY / 131.0; // Convert to deg/s

        // This fixes the transition problem when the accelerometer angle jumps between -180 and 180 degrees
        if ((ROLL < -90 && kalAngleX > 90) || (ROLL > 90 && kalAngleX < -90)) {
            kalmanX.setAngle(ROLL);
            compAngleX = ROLL;
            kalAngleX = ROLL;
            gyroXangle = ROLL;
        } else
            kalAngleX = kalmanX.getAngle(ROLL, gyroXrate, dt); // Calculate the angle using a Kalman filter

        if (abs(kalAngleX) > 90)
            gyroYrate = -gyroYrate; // Invert rate, so it fits the restriced accelerometer reading
        kalAngleY = kalmanY.getAngle(PITCH, gyroYrate, dt);

        gyroXangle += gyroXrate * dt; // Calculate gyro angle without any filter
        gyroYangle += gyroYrate * dt;
        //gyroXangle += kalmanX.getRate() * dt; // Calculate gyro angle using the unbiased rate
        //gyroYangle += kalmanY.getRate() * dt;

        compAngleX = 0.93 * (compAngleX + gyroXrate * dt) + 0.07 * ROLL; // Calculate the angle using a Complimentary filter
        compAngleY = 0.93 * (compAngleY + gyroYrate * dt) + 0.07 * PITCH;

        // Reset the gyro angle when it has drifted too much
        if (gyroXangle < -180 || gyroXangle > 180)
            gyroXangle = kalAngleX;
        if (gyroYangle < -180 || gyroYangle > 180)
            gyroYangle = kalAngleY;
    }
}
