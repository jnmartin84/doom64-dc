from PIL import Image
import json

filenames = ["json/spectre.json", "json/barniteshot.json", "json/fattrect3.json", "json/playtrooposs.json", "json/skulbosshead.json", "json/fattrect1.json", "json/non_enemy.json", "json/remcybr.json", "json/fattrect2.json", "json/painbsp.json", "json/sargfirstcybr.json"]
for filename in filenames:
#    print(filename[5:-5])
    result = Image.new('RGB', (1024,1024), color=(255,0,255,255))
    with open(filename, "r") as read_file:
        data = json.load(read_file)
        for sprite in data['sprites']:
            if "non_enemy" in filename:
                fname = "doom64nonenemy/" + sprite['fileName'].upper().replace("PNG","png")
            else:
                fname = "doom64monster/" + sprite['fileName'].upper().replace("PNG","png")
#            print(fname)
            x = int(sprite['x'])
            y = int(sprite['y'])
            width = float(sprite['width'])
            height = float(sprite['height'])
            image_n = Image.open(fname)
            result.paste(im=image_n, box=(x,y))
    if "non_enemy" in filename:
        result.save("sheets/unc/" + filename[5:-5] + ".png")
    else:
        result.save("sheets/vq/" + filename[5:-5] + ".png")   