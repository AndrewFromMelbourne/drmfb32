#!/usr/bin/env python3

from PIL import Image

num_key_frames = 8

with Image.open('6.gif') as im:
    for i in range(im.n_frames):
        im.seek(i)
        im.save('6_{}.png'.format(i))
