#!/usr/bin/env python3
import serial
import datetime

# Options
# SERIAL_PORT = "/dev/cu.usbmodem14101"
SERIAL_PORT = "/dev/cu.usbserial-AR0K0FDK"
SERIAL_BAUDRATE = 115200
IMAGE_MAX_SIZE = 300000
IMAGE_START_STRING = "START JPEG IMAGE"
IMAGE_END_STRING = "END JPEG IMAGE"
SERIAL_START_STRING = "<"
SERIAL_END_STRING = ">"
SERIAL_DELIMITER = ":"

def stringInBytes(byteStream, stringKey):
    """Check if a string is contained in an array of bytes"""
    try:
        return stringKey in byteStream.decode()
    except:
        return False

def writeBytesToFile(filename, bytes):
    """Write a byte array to file"""
    with open(filename, 'w+b') as f:
        f.write(buf)

def readSensorValue(line):
    """Parse a sensor value from serial"""
    stripped = line.strip("\r\n " + SERIAL_START_STRING + SERIAL_END_STRING);
    strings = stripped.split(SERIAL_DELIMITER);
    if len(strings) > 1:
        return (strings[0], strings[1])
    else:
        return (strings[0], "")

def readImage(ser):
    """Read the image bitstream from serial"""
    print("Reading image from serial...")

    # Read the image, terminating when end string is found
    startTime = datetime.datetime.now()
    buf = ser.read_until(IMAGE_END_STRING.encode(), size=IMAGE_MAX_SIZE)

    # Truncate the end string
    buf = buf[0:-len(IMAGE_END_STRING)]

    deltaMs = round((datetime.datetime.now() - startTime).total_seconds() * 1000)
    bytesPerSec = round(len(buf) / deltaMs * 1000);
    print("Received {} bytes in {} ms ({} bytes/s).".format(len(buf), deltaMs, bytesPerSec))

    return buf

# Main code
with serial.Serial(SERIAL_PORT, SERIAL_BAUDRATE, timeout=30) as ser:
    while True:
        line = ser.readline()
        
        # Normal mode
        if stringInBytes(line, SERIAL_START_STRING):
            (key, value) = readSensorValue(line.decode())
            print(key, value)
        
        # Image mode
        elif stringInBytes(line, IMAGE_START_STRING):
            filename = datetime.datetime.now().strftime("%Y-%m-%dT%H.%M.%S.jpg")
            buf = readImage(ser)
            print("Writing to {}...".format(filename))
            writeBytesToFile(filename, buf)

        # Garbage mode, just print the stuff
        else:
            try:
                print("{} > {}".format(SERIAL_PORT, line.decode()), end="")
            except:
                pass