import serial

class GY25:
    def __init__(self, serialPort, baudrate=115200):
        self.ser = serial.Serial(serialPort, baudrate)

    def _rawData(self):
        data = ''
        while len(data) != 8:
            data = self.ser.read_until(b'U')
        return data

    def _normalize(self, val, nbits=16):
        if val < 0:
            val = (1 << nbits) + val
        else:
            if (val & (1 << (nbits - 1))) != 0:
                # If sign bit is set.
                # compute negative value.
                val = val - (1 << nbits)
        return val / 100

    def readData(self):
        data = self._rawData()

        pitch = self._normalize((data[1] & 0x00FF) << 8 | (data[2] & 0x00FF))
        heading = self._normalize((data[3] & 0x00FF) << 8 | (data[4] & 0x00FF))
        roll = self._normalize((data[5] & 0x00FF) << 8 | (data[6] & 0x00FF))

        if roll > 0:
            roll -= 180
        else:
            roll += 180

        return pitch, heading, roll

if __name__ == '__main__':
    # gy25 = GY25('/dev/tty.usbserial')
    gy25 = GY25('/dev/ttyS0')

    while True:
        print(gy25.readData())