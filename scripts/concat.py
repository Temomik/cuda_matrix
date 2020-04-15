import numpy as np
import PIL
from PIL import Image
import os

def get_concat_h(im1, im2):
    dst = Image.new('RGB', (im1.width + im2.width, max(im1.height,im2.height)))
    dst.paste(im1, (0, 0))
    dst.paste(im2, (im1.width, 0))
    return dst

def get_concat_v(im1, im2):
    dst = Image.new('RGB', (max(im1.width,im2.width), im1.height + im2.height))
    dst.paste(im1, (0, 0))
    dst.paste(im2, (0, im1.height))
    return dst

images_path = []

for root, dirs, files in os.walk("img"):
    for file in sorted(files):
        images_path.append(os.path.join(root, file))

imgs  = [ Image.open(i) for i in images_path ]
# get_concat_v(get_concat_h(imgs[1000],imgs[2131]),imgs[5000]).save("test.jpg")
height = int(np.sqrt(len(images_path)))
width = int(len(images_path)/int(np.sqrt(len(images_path)))) + 1
length = len(imgs)
# print(int(np.sqrt(len(images_path))))
# print(len(images_path)/int(np.sqrt(len(images_path))))
height = 50
width = 50
print(len(images_path))
print(height)
print(width)
outImage = Image.new('RGB', (0, 0))
for i in range(height):
    tmpImage = Image.new('RGB', (0, 0))
    for j in range(width):
        tmpIndex = i*width+j
        if tmpIndex < length:
            tmpImage = get_concat_h(tmpImage,imgs[tmpIndex])
    outImage = get_concat_v(outImage,tmpImage)
outImage.save("out.png")