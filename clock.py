import numpy as np
from PIL import Image, ImageFont, ImageDraw
from datetime import datetime, timedelta
import serial
import serial.tools.list_ports
import time
import colorsys, random

serlist = serial.tools.list_ports.comports()

ser = serial.Serial()
ser.baudrate = 115200
ser.timeout = 0.5
ser.port = serlist[0][0]
print('Connecting to ' + ser.port)
ser.open()

black = (0, 0, 0)
white = (255, 255, 255)

lasttime = ''
fluc = 0
while True:
	output = Image.new("RGB", (32, 16), black)
	draw = ImageDraw.Draw(output)
	font = ImageFont.truetype('hd44780.ttf', 7)

	timereal = datetime.now()
	datetext = timereal.strftime("%m %d")

	if random.randrange(0, 10) == 0:
		fluc = fluc + [-1, 1][random.randrange(0, 2)] #random error to clock
		print('Time error: ' + str(fluc) + 'sec')
	timedisp = timereal + timedelta(seconds=fluc)
	timetext = timedisp.strftime("%H%M%S")

	timecolor = [(), (), ()]
	timecolor[0] = colorsys.hsv_to_rgb(timedisp.hour/24, 0.6, 1)
	timecolor[1] = colorsys.hsv_to_rgb((timedisp.minute/60+0.0)%1, 0.6, 1)
	timecolor[2] = colorsys.hsv_to_rgb(timedisp.second/60, 0.6, 1)

	timecolor = list(map(lambda y: tuple(map(lambda x: int(x*256), y)), timecolor))

	datecolor = colorsys.hsv_to_rgb(1, 0, 1)
	datecolor = tuple(map(lambda x: int(x*256), datecolor))

	datecolor_hyphen = colorsys.hsv_to_rgb(timedisp.second%10/10, 0.4, 1)
	datecolor_hyphen = tuple(map(lambda x: int(x*256), datecolor_hyphen))


	draw.text((3, 0), datetext, font=font, fill=datecolor)
	draw.text((14, 0), '-', font=font, fill=datecolor_hyphen)
	for k, i in enumerate(timetext):
		draw.text((k*5+(k//2), 9), i, font=font, fill=timecolor[k//2])
	#draw.text((9, 9), ":", font=font, fill=white

	if timedisp.second % 2 == 0:
		#draw.point((10, 10), fill=white)
		draw.point((10, 11), fill=white)
		#draw.point((10, 13), fill=white)
		draw.point((10, 14), fill=white)
		#draw.point((21, 10), fill=white)
		draw.point((21, 11), fill=white)
		#draw.point((21, 13), fill=white)
		draw.point((21, 14), fill=white)


	serdata = bytearray(1539)
	serdata[0] = 66
	serdata[1] = 77
	seq = 2
	imdata = np.array(output)
	for i in imdata:
		for j in i:
			for pixel in j:
				serdata[seq] = pixel
				seq += 1

	for i in range(4):
		try:
			ser.write(serdata)
			time.sleep(0.001)
		except:
			pass
	lasttime = datetime.now().strftime("%S")
	while datetime.now().strftime("%S") == lasttime:
		time.sleep(0.1)
