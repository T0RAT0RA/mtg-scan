#!/usr/bin/env python3

import sys, base64, glob, time
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
        print('reading')
        print(content_file)
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

    # Write BMP header
    outfile.write(bytearray(header))

    # Read bytes from serial and write them to file
    for k in range(320*240*2):
        c = outfile.write(port.read())

    # Send "stop" message
    sendbyte(port, 0)

    # Close output file
    outfile.close()

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



if __name__ == '__main__':
    # Find Arduino port
    arduino_ports = [
        p.device
        for p in serial.tools.list_ports.comports()
        if 'Arduino' in p.manufacturer
    ]

    if not arduino_ports:
        raise IOError("No Arduino found")
    if len(arduino_ports) > 1:
        warnings.warn('Multiple Arduinos found - using the first')

    # Open connection to Arduino with a timeout of two seconds
    port = serial.Serial(arduino_ports[0], BAUD, timeout=2)

    # Report acknowledgment from camera
    getack(port)
    time.sleep(0.2)

    while(1):
        response = port.readline()
        if (input("prompt") == 'y' or response == b'CARD DETECTED\r\n'):
            print('CARD DETECTED.')
            # capture_card()
            try:
                card_meta = analyze_card(OUTFILENAME)
                print(card_meta)
            except Exception as inst:
                print({"error": str(inst)})

        else:
            print(response)
        time.sleep(0.1)