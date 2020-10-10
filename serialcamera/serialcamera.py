import serial
import datetime

# Options
SERIAL_PORT = "/dev/tty.usbmodem14101"
SERIAL_BAUDRATE = 115200
IMAGE_MAX_SIZE = 300000
IMAGE_START_STRING = "START JPEG IMAGE"
IMAGE_END_STRING = "END JPEG IMAGE"

def stringInBytes(byteStream, stringKey):
    """Checks if a string is contained in an array of bytes"""
    try:
        return stringKey in byteStream.decode()
    except:
        return False

def writeBytesToFile(filename, bytes):
    """Writes a byte array to file"""
    with open(filename, 'w+b') as f:
        f.write(buf)

# Main code
with serial.Serial(SERIAL_PORT, SERIAL_BAUDRATE, timeout=30) as ser:
    while True:

        # Look for the line "START JPEG IMAGE"
        line = ser.readline()
        
        if stringInBytes(line, IMAGE_START_STRING):
            print("Reading image from serial...")

            # Read the image, terminating when end string is found
            startTime = datetime.datetime.now()
            buf = ser.read_until(IMAGE_END_STRING.encode(), size=IMAGE_MAX_SIZE)

            # Truncate the end string
            buf = buf[0:-len(IMAGE_END_STRING)]

            deltaMs = round((datetime.datetime.now() - startTime).total_seconds() * 1000)
            print("Received {} bytes in {} ms.".format(len(buf), deltaMs))

            # Write to file
            filename = startTime.strftime("%Y-%m-%dT%H.%M.%S.jpg")
            print("Writing to {}...".format(filename))
            writeBytesToFile(filename, buf)
        
        else:
            # Print whatever is received from ESP32 for debugging
            try:
                print("{} > {}".format(SERIAL_PORT, line.decode()), end="")
            except:
                pass