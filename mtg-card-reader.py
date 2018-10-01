#!/usr/bin/env python3

import os, re, time
import itertools
import arrow, click
import boto3, scrython
import serial
import serial.tools.list_ports
from sys import stdout


BAUD = 921600
CAPTURE_FOLDER = './captures/'
CARD_EXT = '.jpg'


def rename_file(path, name):
    if not os.path.isfile(path):
        print('File %s does not exist, skipping.' % path)
        return False


    folder = os.path.dirname(path)
    card_count = [f for f in os.listdir(folder) if re.search(r'[0-9]+' + name + '[0-9]+' + CARD_EXT, f)]
    name = name + str(len(card_count) + 1)
    new_filename = re.sub(CARD_EXT, name + CARD_EXT, path)
    print('Renaming %s to % s' % (path, new_filename))
    os.rename(path, new_filename)

def analyze_card(path):
    print('\nStart analyzing card')
    start_analyze = arrow.now()
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
            print('Name detected: %s' % name)

            try:
                card = scrython.cards.Named(exact=name)
                colors = card.color_identity()
                rename_file(path, name)
                print('Card found. Colors: %s' % colors)
            except Exception as e:
                error = str(e)
                print('Card not found. %s' % str(e))

            duration = (arrow.now() - start_analyze).seconds
            print('Analyze done in %is.' % duration)
            return {
                'name': name,
                'colors': colors,
                'error': error
            }
        else:
            raise Exception('No lines found.')
    else:
        raise Exception('No text found.')


def capture_card(port, path):
    print('\nWaiting capture...')

    # Loop over bytes from Arduino for a single image
    written = False
    prevbyte = None
    done = False
    start_capturing = None
    while not done:

        # Read a byte from Arduino
        currbyte = port.read(1)

        # If we've already read one byte, we can check pairs of bytes
        if prevbyte:

            # Start-of-image sentinel bytes: write previous byte to temp file
            if ord(currbyte) == 0xd8 and ord(prevbyte) == 0xff:
                print('Start capturing.')
                start_capturing = arrow.now()
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

    duration = (arrow.now() - start_capturing).seconds
    print('Capture done in %is.' % duration)
    return True

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

@click.command()
def main():
    # Find Arduino port
    arduino_ports = [
        p.device
        for p in serial.tools.list_ports.comports()
        if p.manufacturer is not None and 'Arduino' in p.manufacturer
    ]

    if not arduino_ports:
        raise IOError("No Arduino found")
    if len(arduino_ports) > 1:
        print('Multiple Arduinos found - using the first')

    # Open connection to Arduino with a timeout of two seconds
    port = serial.Serial(arduino_ports[0], BAUD, timeout=None)

    # Report acknowledgment from camera
    getack(port)
    time.sleep(0.2)

    CAPTURES = CAPTURE_FOLDER + arrow.now().format('YYYY-MM-DD HH:mm:ss') + '/'
    if not os.path.exists(CAPTURES):
        os.makedirs(CAPTURES)

    print('Creating capture folder: %s' % CAPTURES)

    print('Ready.')

    index = 1;
    while(1):
        path = CAPTURES + str(index) + CARD_EXT;

        capture_card(port, path)

        try:
            card_meta = analyze_card(path)
        except Exception as e:
            print(str(e))
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
    except click.Abort:
        printStatus()
