import sys,tty,termios
import serial

class _Getch:
    def __call__(self):
        fd = sys.stdin.fileno()
        old_settings = termios.tcgetattr(fd)
        try:
            tty.setraw(sys.stdin.fileno())
            ch = sys.stdin.read(3)
        finally:
            termios.tcsetattr(fd, termios.TCSADRAIN, old_settings)
        return ch

def get():
    print('connecting...')
    ser = serial.Serial('/dev/tty.HC-05-SPPDev-13', 9600)
    print('connected!')

    inkey = _Getch()

    while(True):
        k=inkey()
        if k[2]=='A':
            ser.write(b'u')
            print("up")
        elif k[2]=='B':
            ser.write(b'd')
            print("down")
        elif k[2]=='C':
            ser.write(b'r')
            print("right")
        elif k[2]=='D':
            ser.write(b'l')
            print("left")
        else:
            break

if __name__=='__main__':
    get()
