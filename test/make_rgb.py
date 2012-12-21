import colorsys

gamma = 2.2
def g(x):
    return int(255 * (x/255.) ** gamma)

h = 0
while h < 1:
    print '// ' + str(h)
    c = colorsys.hsv_to_rgb(h, 1, 1)
    print g(c[0] * 255), ',', g(c[1] * 255), ',', g(c[2] * 255), ','
    h += 1./240 + 1e-10
