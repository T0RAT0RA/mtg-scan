#!/usr/bin/env python3

import sys, base64, glob
import boto3


def compute_card(path):
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


for card in glob.glob('card*'):
    try:
        card_meta = compute_card(card)
        print(card_meta)
    except Exception as inst:
        print({"error": card, "message": str(inst)})
