class Card:

  def __init__(self, text_found=None,
                name=None, price=None, colors=['unknown'],
                img_path=None, img_name=None):
    self.text_found = text_found
    self.name = name
    self.price = price
    self.colors = colors
    self.img_path = img_path
    self.img_name = img_name
    self.error = None
    self.is_found = False

  def isFound(self):
    return bool(self.name)

  def setTextFound(self, text_found):
    self.text_found = text_found
    return self

  def setError(self, error):
    self.error = error
    return self

  def hasError(self):
    return bool(self.error)

  def getImgName(self):
    return self.img_name

  def getImgPath(self):
    return self.img_path