import RPi.GPIO as GPIO
import time

class L298:
    def __init__(self, pinEnable, pinIN1, pinIN2):
        GPIO.setup(pinEnable, GPIO.OUT)
        GPIO.setup(pinIN1, GPIO.OUT)
        GPIO.setup(pinIN2, GPIO.OUT)

        self.pwm = GPIO.PWM(pinEnable, 50)
        self.pinIN1 = pinIN1
        self.pinIN2 = pinIN2
        self.pwm.start(0)

    def forward(self, speed=100):
        GPIO.output(self.pinIN1, GPIO.HIGH)
        GPIO.output(self.pinIN2, GPIO.LOW)

        self.pwm.ChangeDutyCycle(speed)

    def backward(self, speed=100):
        GPIO.output(self.pinIN1, GPIO.LOW)
        GPIO.output(self.pinIN2, GPIO.HIGH)

        self.pwm.ChangeDutyCycle(speed)

    def go(self, speed):
        if speed > 0:
            self.forward(min(speed, 100))
        else:
            self.backward(min(-1 * speed, 100))