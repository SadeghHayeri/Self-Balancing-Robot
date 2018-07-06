import RPi.GPIO as GPIO
from motorDriver import L298
from gy25 import GY25
from pid import PID
from tuner import Tuner
from controller import Controller
import time

GPIO.setmode(GPIO.BCM)

rightMotorINT1 = 24
rightMotorINT2 = 23
rightMotorPWM = 18

leftMotorINT1 = 19
leftMotorINT2 = 26
leftMotorPWM = 13

gy25Port = '/dev/ttyS0'

def main():
    balancingPID = PID(20, 0, 40)
    # movementPID = PID(0.1, 0, 0)
    rotationPID = PID(1, 0, 0)

    rightMotor = L298(rightMotorPWM, rightMotorINT1, rightMotorINT2)
    leftMotor = L298(leftMotorPWM, leftMotorINT1, leftMotorINT2)
    gy25 = GY25(gy25Port)

    lastTime = time.time()
    displacement = [0]
    x = 0

    # tunerThread = Tuner(balancingPID)
    # tunerThread.start()
    controllerProperty = [0, 0]
    controllerThread = Controller(controllerProperty)
    controllerThread.start()

    while True:
        # diffAngle = movementPID.update(displacement[0])
        # diffAngle = min(2, diffAngle)
        # diffAngle = max(-2, diffAngle)
        # print(diffAngle, displacement)
        # balancingPID.setPoint(diffAngle)

        gy25Data = gy25.readData()
        roll = gy25Data[2] - 2.5
        pitch = gy25Data[0]
        pwmOut = -1 * balancingPID.update(roll)

        diffPWM = rotationPID.update((pitch + 180 + controllerProperty[0]) % 360 - 180)
        diffPWM = min(20, diffPWM)
        diffPWM = max(-20, diffPWM)
        # print(gy25Data)

        if controllerProperty[0] > 0:
            rightMotor.go(pwmOut + 40)
            leftMotor.go(pwmOut - 40)
        elif controllerProperty[0] < 0:
            rightMotor.go(pwmOut - 40)
            leftMotor.go(pwmOut + 40)
        else:
            rightMotor.go(pwmOut)
            leftMotor.go(pwmOut)
        controllerProperty[0] = 0

        dt = time.time() - lastTime
        displacement[0] += pwmOut * dt
        # x += 1
        # if x == 1000:
        #     displacement = 0
        #     x = 0

        # print(displacement)
        lastTime = time.time()

if __name__ == '__main__':
    try:
        main()
    except KeyboardInterrupt:
        GPIO.cleanup()
    finally:
        GPIO.cleanup()