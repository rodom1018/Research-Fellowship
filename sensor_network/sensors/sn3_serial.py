import serial
import time
from datetime import datetime

ser = serial.Serial(
   port='/dev/ttyUSB0',\
   baudrate=115200,\
   parity=serial.PARITY_NONE,\
   stopbits=serial.STOPBITS_ONE,\
   bytesize=serial.EIGHTBITS,\
   timeout=0)
print("connected to: " + ser.portstr)

t=datetime.now()
hour=t.hour
minute=t.minute
second=t.second

ser.write(str(hour))
ser.write("\n")

ser.write(str(minute))
ser.write("\n")

ser.write(str(second))
ser.write("\n")

while True:
	time.sleep(0.1)
	line = ser.readline();
	if line:
		print(line),
ser.close()