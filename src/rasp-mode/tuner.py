from threading import Thread

class _Getch:
    def __init__(self):
        try:
            self.impl = _GetchWindows()
        except ImportError:
            self.impl = _GetchUnix()

    def __call__(self): return self.impl()

class _GetchUnix:
    def __init__(self):
        import tty, sys

    def __call__(self):
        import sys, tty, termios
        fd = sys.stdin.fileno()
        old_settings = termios.tcgetattr(fd)
        try:
            tty.setraw(sys.stdin.fileno())
            ch = sys.stdin.read(1)
        finally:
            termios.tcsetattr(fd, termios.TCSADRAIN, old_settings)
        return ch


class _GetchWindows:
    def __init__(self):
        import msvcrt

    def __call__(self):
        import msvcrt
        return msvcrt.getch()

class Tuner(Thread):
    def __init__(self, pid, dis=None):
        Thread.__init__(self)
        self.pid = pid
        self.getch = _Getch()
        self.dis = dis

    def run(self):
        while True:
            command = self.getch()

            if command == 'q':
                self.pid.setKp(max(self.pid.Kp + .01, 0))
            elif command == 'a':
                self.pid.setKp(max(self.pid.Kp - .01, 0))

            elif command == 'w':
                self.pid.setKi(max(self.pid.Ki + .001, 0))
            elif command == 's':
                self.pid.setKi(max(self.pid.Ki - .001, 0))

            elif command == 'e':
                self.pid.setKd(max(self.pid.Kd + .01, 0))
            elif command == 'd':
                self.pid.setKd(max(self.pid.Kd - .01, 0))

            elif command == 'p':
                break;

            print(self.dis, " P:", self.pid.Kp, " I:", self.pid.Ki, " D:", self.pid.Kd)