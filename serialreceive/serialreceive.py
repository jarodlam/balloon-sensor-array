#!/usr/bin/env python3
import serial
import datetime
import csv

# Options
SERIAL_PORT = "/dev/cu.usbmodem14101"         # Arduino Uno
#SERIAL_PORT = "/dev/cu.usbserial-AR0K0FDK"    # FTDI Basic
#SERIAL_PORT = "/dev/cu.usbserial-D3070O11"    # Base station
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

def writeBytesToFile(filename, bytestream):
    """Write a byte array to file"""
    with open(filename, 'w+b') as f:
        f.write(bytestream)

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

def currentDatetimestamp():
    return datetime.datetime.now().strftime("%Y-%m-%dT%H.%M.%S")

def currentTimestamp():
    return datetime.datetime.now().strftime("%H:%M:%S")

# Main code
outfilename = currentDatetimestamp() + ".csv";

with serial.Serial(SERIAL_PORT, SERIAL_BAUDRATE, timeout=30) as ser:
    with open(outfilename, 'w', newline='') as csvfile:
        csvWriter = csv.writer(csvfile)
        
        while True:
            line = ser.readline()
        
            # Normal mode
            if stringInBytes(line, SERIAL_START_STRING):
                (key, value) = readSensorValue(line.decode())
                print(key, value)
                csvWriter.writerow([currentTimestamp(), key, value]);
        
            # Image mode
            elif stringInBytes(line, IMAGE_START_STRING):
                filename = currentDatetimestamp() + ".jpg"
                buf = readImage(ser)
                print("Writing to {}...".format(filename))
                writeBytesToFile(filename, buf)

            # Garbage mode, just print the stuff
            else:
                try:
                    print("{} > {}".format(SERIAL_PORT, line.decode()), end="")
                except:
                    pass