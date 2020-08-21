import struct
import ctypes
import io


class Padding(ctypes.LittleEndianStructure):
    _fields_ = [('p', ctypes.c_int8),]


class MyStruct(ctypes.LittleEndianStructure):
    _fields_ = [
        ('a', ctypes.c_int8),
        ('c', ctypes.c_int16),
        ('b', ctypes.c_int32),
    ]


x = MyStruct(0xFF, 0x1, 0xFFFF)
p = Padding(0x0)

print(x.a, x.b, x.c)

print(ctypes.sizeof(x))

bytes_stream = io.BytesIO()
bytes_stream.write(x)

for _ in range(16 - ctypes.sizeof(x)):
    bytes_stream.write(p)

bytes_stream.flush()
bytes_stream.seek(0)

bytes = bytes_stream.read()

print(len(bytes))

bytecode_hex = ":".join("{:02x}".format(ord(c)) for c in bytes)
print("[SCP/ACP] %s" % bytecode_hex)
