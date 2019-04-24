import os, re, glob, arrow
import boto3, scrython
import random
from shutil import copyfile
from Card import Card

class CardCaptures:

  IMG_EXT = '.jpg'

  def __init__(self, capture_folder):
    self.capture_folder = capture_folder
    self.cards = []


  def capture(self):
    # Create capture folder if doesn't exist
    if not os.path.exists(self.capture_folder):
      try:
        os.makedirs(self.capture_folder)
      except OSError:
          print ("Creation of the directory %s failed" % self.capture_folder)

    img_count = len(os.listdir(self.capture_folder))
    '{0:03d}'.format(img_count) + self.IMG_EXT
    img_name = '{0:03d}'.format(img_count) + self.IMG_EXT
    img_path = self.capture_folder + img_name

    if not self.takeImg(img_path):
      return False


    card = Card(img_name=img_name, img_path=img_path)
    self.cards.append(card)

    return card

  def takeImg(self, img_path):
    tmp_dir = "captures/2019-01-03 23:39:20/"
    tmp = tmp_dir + random.choice(os.listdir(tmp_dir))
    copyfile(tmp, img_path)
    return True

  def getImages(self):
    if not self.capture_folder:
      return []

    images = []
    for image in glob.glob("{}*{}".format(self.capture_folder, self.IMG_EXT)):
      images.append(image.replace(self.capture_folder, ''))

    return sorted(images, reverse=True)


  def renameCard(self, path, name):
    if not os.path.isfile(path):
        return False

    folder = os.path.dirname(path)
    card_count = [f for f in os.listdir(folder) if re.search(r'[0-9]+-' + name + '-[0-9]+' + self.IMG_EXT, f)]
    name = '-' + name + '-{0:03d}'.format(len(card_count) + 1)
    new_filename = re.sub(self.IMG_EXT, name + self.IMG_EXT, path)
    # print('Renaming %s to % s' % (path, new_filename))
    os.rename(path, new_filename)

    return new_filename

  def analyzeCard(self, card):

    # start_analyze = arrow.now()
    with open(card.img_path, 'rb') as content_file:
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

            card.text_found = lines[0]['DetectedText']

            try:
              card_meta = scrython.cards.Named(exact=card.text_found)

              card.name = card_meta.name()
              card.price = card_meta.prices('usd')
              card.colors = card_meta.color_identity()
              card.img_path = self.renameCard(card.img_path, card.name)
              card.img_name = card.img_path.replace(self.capture_folder, '')
            except Exception as e:
                card.setError(str(e))

            return card
        else:
            card.setError('No lines found.')
    else:
        card.setError('No text found.')


  def getCardList(self):
    cards = {}
    for card in self.cards:
      if not card.isFound():
        continue

      if not card.name in cards:
        cards[card.name] = {
          'name': card.name,
          'cards': [card],
          'quantity': 1,
        }
      else:
        cards[card.name]['cards'].append(card)
        cards[card.name]['quantity'] += 1

    return cards


  def getCardByImgName(self, img_name):
    matching_cards = [c for c in self.cards if c.img_name == img_name]
    if matching_cards:
      return matching_cards[0]

    return None