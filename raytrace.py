import math
from vec3 import vec3
from PIL import Image


WIDTH = 400
HEIGHT = 300
FOV = math.pi / 2.0
CAMERA_POS = vec3(0, 0, 0)


class Sphere(object):

    def __init__(self, center, radius):
        self.__center = center
        self.__radius = radius

    def intersects(self, posFrom, direction):
        r = self.__radius
        c = self.__center
        l = c - posFrom
        tca = direction * l
        d2 = l * l - tca * tca
        if d2 > r * r:
            return False
        thc = (r*r - d2)**0.5
        t0 = tca - thc
        t1 = tca + thc
        if t0 < 0:
            t0 = t1
        if t0 < 0:
            return False
        return True


def castRay(posFrom, direction, sphere):
    if not sphere.intersects(posFrom, direction):
        return vec3(0.2, 0.7, 0.8)
    return vec3(0.4, 0.4, 0.3)


img = Image.new('RGB', (WIDTH, HEIGHT), color='white')
pixels = img.load()

sphere = Sphere(vec3(-3, 0, -10), 2)

for i in range(WIDTH):
    for j in range(HEIGHT):
        x = (2.0 * (i + 0.5) / WIDTH - 1) * math.tan(FOV / 2.0) * WIDTH / HEIGHT
        y = -(2.0 * (j + 0.5) / HEIGHT - 1) * math.tan(FOV / 2.0)
        direction = vec3(x, y, -1).normalize()
        res = castRay(CAMERA_POS, direction, sphere)
        pixels[i, j] = (int(res[0] * 255), int(res[1] * 255), int(res[2] * 255))

img.save('result.png')

