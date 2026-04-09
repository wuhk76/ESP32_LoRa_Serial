import serial
class Esplora:
    def __init__(self, port, baud, timeout = 1):
        self.esp = serial.Serial(port, baud, timeout = timeout)
    def send(self, data):
        stx = bytes([0x02])
        size = len(data).to_bytes(4, byteorder = 'big')
        self.esp.write(stx + size + data)
    def receive(self):
        etx = bytes([0x03])
        self.esp.write(etx)
        size = self.esp.read(4)
        size = int.from_bytes(size, byteorder = 'big')
        data = self.esp.read(size)
        return data