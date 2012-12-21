from PIL import Image

import socket
import sys
import argparse

parser = argparse.ArgumentParser('Send images to WS2811 LEDs')
parser.add_argument('images', metavar='image_file', nargs='+')
parser.add_argument('--swap', action='store_true', help='swap red and green')
parser.add_argument('--gamma', type=float, default=2.2, help='set the gamma')
args = parser.parse_args()

GAMMA = args.gamma
def gamma(x):
    return chr(int(255 * (x/255.) ** GAMMA))

def rgb(r, g, b):
    if args.swap:
        t = r
        r = g
        g = t
    return gamma(r) + gamma(g) + gamma(b)

def send_image(ifile):
    print ifile

    im = Image.open(ifile)
    print im.size

    im2 = im.resize((int(im.size[0]*(240.0/im.size[1])), 240), Image.BILINEAR)
    print im2.size

    im2.save("test.png")

    pix = im2.load()
    for c in range(im2.size[0]):
        s = ""
        for r in range(240):
            p = pix[c, r]
            #        print c, r, p
            s += rgb(p[0], p[1], p[2])
        sock = socket.socket()
        sock.connect(("192.168.1.112", 222))
        sock.send(s)
        print "Sent", len(s), c
        ack = sock.recv(1)
        assert len(ack) == 1
        print "Ack", len(ack), ack

for img in args.images:
    send_image(img)
