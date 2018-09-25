#!/usr/bin/env python3

import sys, base64, glob, time
import random
import boto3
import warnings
import serial
import serial.tools.list_ports
from sys import stdout


BAUD = 921600
OUTFILENAME = './captures/card.jpg'


def analyze_card(path):
    print('\nStart analyzing card')
    with open(path, 'rb') as content_file:
        image = content_file.read()

    client = boto3.client('rekognition')
    response = client.detect_text(
        Image={
            'Bytes': image,
        }
    )

    if response['TextDetections']:
        lines = [line for line in response['TextDetections'] if line['Type'] == 'LINE']
        if lines:
            return {
                "img": path,
                "name": lines[0]['DetectedText'],
                "edition": lines[-1]['DetectedText'],
            }
        else:
            raise Exception('No lines found.')
    else:
        raise Exception('No text found.')


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



CARD_NONE = 0;
CARD_UNKNOWN = -1;
CARD_RED = 1;
CARD_GREEN = 2;
CARD_BLUE = 3;
CARD_BLACK = 4;
CARD_WHITE = 5;
CARD_COLORLESS = 6;
CARD_OTHER = 7;

# main  --------------------------------------------------------------------------
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
    port = serial.Serial(arduino_ports[0], BAUD, timeout=5)

    # Report acknowledgment from camera
    getack(port)
    time.sleep(0.2)

    print('Ready, waiting incoming data...')
    while(1):
        response = port.read(1)
        if len(response) == 1 and ord(response) == 42:
            print('CARD DETECTED.')
            # time.sleep(1);
            color = random.choice([
                CARD_RED,
                CARD_GREEN,
                CARD_BLUE,
                CARD_BLACK,
                CARD_WHITE,
            ])
            print('CARD ANALYSED: %s' % color)
            sendbyte(port, 43)
            sendbyte(port, color)
        else:
            print(response)
        time.sleep(0.1)
