from PIL import Image
import json

filenames = ["json/non_enemy.json"]
for filename in filenames:
    result = Image.new('RGB', (1024,1024), color=(255,0,255,255))
    with open(filename, "r") as read_file:
        data = json.load(read_file)
        for sprite in data['sprites']:
            fname = "nonenemy/" + sprite['fileName'].upper().replace("PNG","png")
            x = int(sprite['x'])
            y = int(sprite['y'])
            width = float(sprite['width'])
            height = float(sprite['height'])
            image_n = Image.open(fname)
            result.paste(im=image_n, box=(x,y))
    result.save("sheets/" + filename[5:-5] + ".png")
