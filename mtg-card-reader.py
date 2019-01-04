#!/usr/bin/env python3

import os, re, time
import itertools
import arrow, click
import boto3, scrython
import serial
import serial.tools.list_ports


BAUD = 921600
CAPTURE_FOLDER = './captures/'
CARD_EXT = '.jpg'

CARD_UNKNOWN = 'X'
SERIAL_DELIMITER = "|"

def rename_file(path, name, debug=False):
    if not os.path.isfile(path):
        if debug:
            print('File %s does not exist, skipping.' % path)
        return False


    folder = os.path.dirname(path)
    card_count = [f for f in os.listdir(folder) if re.search(r'[0-9]+-' + name + '-[0-9]+' + CARD_EXT, f)]
    name = '-' + name + '-{0:03d}'.format(len(card_count) + 1)
    new_filename = re.sub(CARD_EXT, name + CARD_EXT, path)
    #print('Renaming %s to % s' % (path, new_filename))
    os.rename(path, new_filename)

def analyze_card(path, debug=False):
    if debug:
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
            name_found = lines[0]['DetectedText']
            colors = [CARD_UNKNOWN]
            colors_text = []
            price = None
            error = None
            # edition = lines[-1]['DetectedText']
            # if debug:
            #     click.echo('Name found: ', nl=False)
            #     click.secho(name_found, fg='yellow')

            try:
                card = scrython.cards.Named(exact=name_found)
                name = card.name()
                price = card.currency('usd')
                colors = card.color_identity()
                rename_file(path, name, debug)

                colors_mapping = {
                    'U': {'bg': 'blue'},
                    'R': {'bg': 'red'},
                    'W': {'bg': 'white', 'fg': 'black'},
                    'G': {'bg': 'green'},
                    'B': {'bg': 'black'},
                }
                colors_text = [click.style(c, **colors_mapping[c]) for c in colors]
            except Exception as e:
                error = str(e)
                name = 'Card not found'
                if debug:
                    click.secho('Card not found: ')
                    click.secho(error, fg='red')

            fg = 'green' if error is None else 'red'
            printRow((name_found, (name, fg), '[{}]'.format(', '.join(colors)), price))

            duration = (arrow.now() - start_analyze).seconds
            if debug:
                print('Analyze done in %is.' % duration)
            return {
                'name': name,
                'colors': colors,
                'error': error
            }
        else:
            click.echo('%-20s' % (click.style("No lines found.", fg='red')))
            raise Exception('No lines found.')
    else:
        click.echo('%-20s' % (click.style("No text found.", fg='red')))
        raise Exception('No text found.')


def capture_card(port, path, debug=False):
    if debug:
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
                if debug:
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
            if written and ord(currbyte) == 0xd9 and ord(prevbyte) == 0xff:
                outfile.close()
                done = True

        # Track previous byte
        prevbyte = currbyte

    duration = (arrow.now() - start_capturing).seconds
    if debug:
        print('Capture done in %is.' % duration)
    return True

# helpers  --------------------------------------------------------------------------
def sendbyte(port, value):
    port.write(bytearray([value]))

columns_padding = (28, 28, 20, 10)
truncated_string = '...'
def printRow(row):
    for i, col in enumerate(row):
        max_length = columns_padding[i]
        text = col
        fg = None
        if type(col) == tuple:
            text = col[0]
            fg = col[1]
        text = (text[:max_length - len(truncated_string)] + truncated_string) if len(text) > max_length else text
        text = text + ' ' * (max_length - len(text))

        # print('%s|%s|%s' % (max_length, len(text), padding))
        click.secho(text, fg=fg, nl=False)

    print('')

# main  --------------------------------------------------------------------------
cards = {
  'success': [],
  'error': []
}

@click.command()
@click.option('--debug', default=False, is_flag=True)
def main(debug):
    DEBUG = debug

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
    ack = port.readline().decode()
    print(ack)
    if 'error' in ack.lower():
        exit()

    time.sleep(0.2)

    CAPTURES = CAPTURE_FOLDER + arrow.now().format('YYYY-MM-DD HH:mm:ss') + '/'
    if not os.path.exists(CAPTURES):
        os.makedirs(CAPTURES)

    if debug:
        print('Creating capture folder: %s' % CAPTURES)

    index = 1
    printRow(('TEXT', 'CARD', 'COLORS', 'USD'))
    while(1):
        path = CAPTURES + '{0:03d}'.format(index) + CARD_EXT

        capture_card(port, path, debug)

        try:
            card_meta = analyze_card(path, debug)
        except Exception as e:
            if debug:
                print(str(e))
            card_meta = {
                "name": "Card not found",
                "colors": [CARD_UNKNOWN],
                "error": "Card not found",
            }


        try:
            color = card_meta['colors']
        except Exception as e:
            color = 'X'


        if debug:
            print('Sending %s' % str.encode(card_meta['name'] + SERIAL_DELIMITER))
        port.write(str.encode(card_meta['name'] + SERIAL_DELIMITER))
        if debug:
            print('Sending %s' % str.encode("".join(color) + SERIAL_DELIMITER))
        port.write(str.encode("".join(color) + SERIAL_DELIMITER))

        if (card_meta['error']):
            cards['error'].append(card_meta)
        else:
            cards['success'].append(card_meta)
        index = index + 1

def printStatus():
    click.secho('---------------------------------------------', fg='yellow')
    click.echo('Card scanned: ', nl=False)
    click.secho(str(len(cards['success'])), fg='green')
    if len(cards['error']):
        click.echo('Card with errors: ', nl=False)
        click.secho(str(len(cards['error'])), fg='red')

        click.echo('Card list:')
    for card, group in itertools.groupby(cards['success'], key=lambda x:x['name']):
        click.echo('  {}'.format(len(list(group))), nl=False)
        click.secho(' {}'.format(card), fg='cyan')

    click.secho('---------------------------------------------', fg='yellow')

import atexit
atexit.register(printStatus)

if __name__ == '__main__':
    main()