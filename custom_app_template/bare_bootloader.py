import sys, time, getopt, serial
from xmodem import XMODEM

name=''
port=''

myopts, args = getopt.getopt(sys.argv[1:],"p:f:")

if len(sys.argv) < 3:
    print("Usage: %s -p port -f file" % sys.argv[0])
    sys.exit()
    
for o, a in myopts:
    if o == '-p':
        port=a
    elif o == '-f':
        name=a
    else:
        print("Usage: %s -p port -f file" % sys.argv[0])
        sys.exit()

print ("Portname: %s; File: %s" % (port,name) )
try:
    stream = open(name, 'rb')
except:
    print("File error")
    sys.exit()
print("File opened")
print("Waiting for device", end="", flush=True)
    
while 1:
    try:
        ser = serial.Serial(port, 115200, timeout = 1)
    except serial.serialutil.SerialException:
        print('.', end="", flush=True)
        time.sleep(1)
        continue
    break

print("\r\nPort opened")

ser.write(b'wait\r\n')
print("Wait response: ")
print(ser.readline().rstrip())

ser.write(b'download\r\n')
print("Download response: ")
print(ser.readline().rstrip())

def getc(size, timeout=1):
    return ser.read(size)

def putc(data, timeout=1):
    ser.write(data)
    time.sleep(0.001) # give device time to send ACK

print("Sending file")
modem = XMODEM(getc, putc)
if( modem.send(stream) == True):
    print("File succesfully downloaded")
else:
    print("Download fail")

stream.close()
ser.close()
