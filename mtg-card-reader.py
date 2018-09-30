#!/usr/bin/env python3

import sys, base64, os, glob, time
import itertools
import boto3, scrython
import serial
import serial.tools.list_ports
from sys import stdout


BAUD = 921600
CAPTURE_FOLDER = './captures/'
CARD_NAME = 'card*.jpg'


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
            name = lines[0]['DetectedText']
            colors = []
            error = None
            # edition = lines[-1]['DetectedText']

            try:
                card = scrython.cards.Named(exact=name)
                colors = card.color_identity()
            except Exception as e:
                error = str(e)

            return {
                'name': name,
                'colors': colors,
                'error': error
            }
        else:
            raise Exception('No lines found.')
    else:
        raise Exception('No text found.')

def capture_card(path):
    print('\nWaiting capture...')

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
                # Open output file
                outfile = open(path, 'wb')
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

    return True;

# helpers  --------------------------------------------------------------------------
def getack(port):
    stdout.write(port.readline().decode())

def sendbyte(port, value):
    port.write(bytearray([value]))


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
cards = {
  'success': [],
  'error': []
}
def main():
    # Find Arduino port
    arduino_ports = [
        p.device
        for p in serial.tools.list_ports.comports()
        if p.manufacturer is not None and 'Arduino' in p.manufacturer
    ]

    # if not arduino_ports:
    #     raise IOError("No Arduino found")
    # if len(arduino_ports) > 1:
    #     print('Multiple Arduinos found - using the first')

    # Open connection to Arduino with a timeout of two seconds
    # port = serial.Serial(arduino_ports[0], BAUD, timeout=None)

    # Report acknowledgment from camera
    # getack(port)
    time.sleep(0.2)

    print('Clearing capture folder: %s' % CAPTURE_FOLDER)
    for filename in glob.glob(CAPTURE_FOLDER + CARD_NAME):
        # os.remove(filename)

    print('Ready, waiting incoming data...')

    index = 1;
    while(1):
        path = CAPTURE_FOLDER + CARD_NAME.replace('*', str(index));

        capture_card(path)

        try:
            card_meta = analyze_card(path)
        except FileNotFoundError as e:
            print(str(e))
            time.sleep(1)
            continue

        if (card_meta['error']):
            cards['error'].append(card_meta)
        else:
            cards['success'].append(card_meta)
        # if len(response) == 1 and ord(response) == 42:
        #     print('CARD DETECTED.')
        #     # time.sleep(1);
        #     color = random.choice([
        #         CARD_RED,
        #         CARD_GREEN,
        #         CARD_BLUE,
        #         CARD_BLACK,
        #         CARD_WHITE,
        #     ])
        #     print('CARD ANALYSED: %s' % color)
        #     sendbyte(port, 43)
        #     sendbyte(port, color)
        # else:
        #     print(response)
        index = index + 1

def printStatus():
    print('---------')
    print('Card scanned: %i' % len(cards['success']))
    if len(cards['error']):
        print('Card with errors: %i' % len(cards['error']))

    print('Card list:')
    for card, group in itertools.groupby(cards['success'], key=lambda x:x['name']):
        print('%s x%i' % (card, len(list(group))))
    print('---------')

if __name__ == '__main__':
    try:
        main()
    except KeyboardInterrupt:
        printStatus()
