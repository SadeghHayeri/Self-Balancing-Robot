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

class Controller(Thread):
    def __init__(self, controllerProperty):
        Thread.__init__(self)
        self.controllerProperty = controllerProperty
        self.getch = _Getch()

    def run(self):
        while True:
            command = self.getch()
            if command == ';':
                self.controllerProperty[0] = 1

            elif command == 'k':
                self.controllerProperty[0] = -1

            elif command == 'p':
                break;

            print("property:", self.controllerProperty)