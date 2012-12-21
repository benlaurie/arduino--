import socket
import time
import colorsys
import argparse

parser = argparse.ArgumentParser('Send test patterns to WS2811 LEDs')
parser.add_argument('--swap', action='store_true', help='swap red and green')
parser.add_argument('--gamma', type=float, default=2.2, help='set the gamma')
parser.add_argument('--red', action='store_const', const=1, default=0)
parser.add_argument('--green', action='store_const', const=1, default=0)
parser.add_argument('--blue', action='store_const', const=1, default=0)
parser.add_argument('--rainbow', action='store_true')
args = parser.parse_args()

# gamma function, yield 0-255 (input 0-255)
GAMMA = args.gamma
def gamma(x):
    return chr(int(255 * (x/255.) ** GAMMA))

def rgb(r, g, b):
    if args.swap:
        t = r
        r = g
        g = t
    return gamma(r) + gamma(g) + gamma(b)

x = 0
hoff = 0
while True:
    # At the moment, the Nanode TCP stack seems to autoclose after 1 packet.
    sock = socket.socket()
    sock.connect(("192.168.1.112", 222))
    print "Connected"
    str = ""

    if args.rainbow:
        h = 0
        while h < 1:
            c = colorsys.hsv_to_rgb(h + hoff, 1, 1)
            str += rgb(c[0] * 255, c[1] * 255, c[2] * 255)
            h += 1./240 + 1e-10
        hoff += 1./240
    else:
        for n in range(240):
            str += rgb(args.red * x, args.green * x, args.blue * x)

    print ord(str[0])
    sock.send(str)
    print "Sent", len(str), x
    ack = sock.recv(1)
    assert len(ack) == 1
    print "Ack", len(ack), ack
    x += 1
    if x > 255:
        x = 0
