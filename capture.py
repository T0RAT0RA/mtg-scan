#!/usr/bin/env python3
'''
jpgstream.py : display live ArduCAM JPEG images using OpenCV

Copyright (C) Simon D. Levy 2017

This file is part of BreezyArduCAM.

BreezyArduCAM is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

BreezyArduCAM is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
You should have received a copy of the GNU General Public License
along with BreezyArduCAM.  If not, see <http://www.gnu.org/licenses/>.
'''

import time
import serial
import serial.tools.list_ports
from sys import stdout
from PIL import Image

# Modifiable params --------------------------------------------------------------------
BAUD = 921600       # Change to 115200 for Due

# helpers  --------------------------------------------------------------------------

def num2bytes(n):
    return [n&0xFF, (n>>8)&0xFF, (n>>16)&0xFF, (n>>24)&0XFF]

def dump(msg):
    stdout.write(msg)
    stdout.flush()

def getack(port):
    stdout.write(port.readline().decode())

def sendbyte(port, value):
    port.write(bytearray([value]))

def ackcheck(port, msg):
    line = port.readline().decode()
    assert(msg in line)


OUTFILENAME = './card.jpg'
def capture_card():
    print('\nStart capturing')
    # Send "start capture" message
    sendbyte(port, 1)

    # Open output file
    outfile = open(OUTFILENAME, 'wb')

    # Loop over bytes from Arduino for a single image
    written = False
    prevbyte = None
    done = False
    while not done:

        # Read a byte from Arduino
        currbyte = port.read(1)

        # If we've already read one byte, we can check pairs of bytes
        if prevbyte:

            # Start-of-image sentinel bytes: write previous byte to temp file
            if ord(currbyte) == 0xd8 and ord(prevbyte) == 0xff:
                outfile.write(prevbyte)
                written = True

            # Inside image, write current byte to file
            if written:
                outfile.write(currbyte)

            # End-of-image sentinel bytes: close temp file and display its contents
            if ord(currbyte) == 0xd9 and ord(prevbyte) == 0xff:
                outfile.close()
                done = True

        # Track previous byte
        prevbyte = currbyte

    # Send "stop" message
    sendbyte(port, 0)

    print('\nDone capturing')


def average_image_color(filename):
    i = Image.open(filename)
    h = i.histogram()

    # split into red, green, blue
    r = h[0:256]
    g = h[256:256*2]
    b = h[256*2: 256*3]

    # perform the weighted average of each channel:
    # the *index* is the channel value, and the *value* is its weight
    return (
        sum( i*w for i, w in enumerate(r) ) / sum(r),
        sum( i*w for i, w in enumerate(g) ) / sum(g),
        sum( i*w for i, w in enumerate(b) ) / sum(b)
    )
# main ------------------------------------------------------------------------------

if __name__ == '__main__':
    # Find Arduino port
    arduino_ports = [
        p.device
        for p in serial.tools.list_ports.comports()
        if p.manufacturer is not None and 'Arduino' in p.manufacturer
    ]

    if not arduino_ports:
        raise IOError("No Arduino found")
    if len(arduino_ports) > 1:
        warnings.warn('Multiple Arduinos found - using the first')

    # Open connection to Arduino with a timeout of two seconds
    port = serial.Serial(arduino_ports[0], BAUD, timeout=2)

    # Report acknowledgment from camera
    getack(port)

    # Wait a spell
    time.sleep(0.2)

    while(1):
        input('Press enter to capture img')
        capture_card()
        print(average_image_color(OUTFILENAME))
        time.sleep(0.1)
