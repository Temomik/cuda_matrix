import numpy as np
import PIL
from PIL import Image
import os

img1  = Image.open("../cpu.jpg")
img2  = Image.open("../gpu.jpg")
# print(img1)
# print(img2)
if img1 == img2:
    print("ok")
else:
    print("(")
    width, height = img1.size
    for i in range(height):
        for j in range(width):
            if img1.getpixel((i, j)) != img2.getpixel((i, j))  and (i < 450) and (j < 450):
                print(img1.getpixel((i, j)),img2.getpixel((i, j)), i,j)