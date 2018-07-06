import serial

ser = serial.Serial('/dev/tty.usbserial', 115200)

def readData():
    data = ''
    while len(data) != 8:
        data = ser.read_until(b'U')
    return data

def mpuRead():
    data = readData()
    roll = (data[5] << 8 | (data[6] & 0x00FF)) / 100.0;

    if(roll > 0):
        return roll - 180
    else:
        return roll + 180

while True:
    print(mpuRead())
