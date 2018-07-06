import RPi.GPIO as GPIO
import time

def main():
    GPIO.setmode(GPIO.BCM)
    GPIO.setup(21, GPIO.OUT)

    while True:
        GPIO.output(21, GPIO.HIGH)
        time.sleep(1)
        print('ON')
        GPIO.output(21, GPIO.LOW)
        time.sleep(1)
        print('OFF')

try:
    main()
except KeyboardInterrupt:
    GPIO.cleanup()




