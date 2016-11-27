# -*- coding: utf-8 -*-
import sys
import ctypes
import argparse

sys.path.append("..")
import pylibi2c


if __name__ == '__main__':
	parser = argparse.ArgumentParser()
	parser.add_argument('-b', '--bus', help='i2c bus, such as 1', type=int, default=1)
	parser.add_argument('-d', '--dev_addr', help='i2c device address', type=int, default=0x50)
	parser.add_argument('-i', '--int_addr', help='i2c internal address', type=int, default=0x0)
	parser.add_argument('-l', '--iaddr_bytes', help='i2c internal address bytes', type=int, default=1)
	parser.add_argument('-w', '--write', help='defualt is read, with -w read', type=bool, default=False)
	parser.add_argument('-s', '--size', help='read / write size', type=int, default=1)
	parser.add_argument('--data', help='write data start', type=int, default=0x0)
	parser.add_argument('--delay', help='i2c r/w delay, unit is msec', type=int, default=5)
	parser.add_argument('--ioctl', help='using ioctl r/w i2c', type=bool, default=False)
	args = vars(parser.parse_args())

	# Args parser
	bus = args.get('bus')
	size = args.get('size')
	data = args.get('data')
	ioctl = args.get('ioctl')
	write = args.get('write')
	delay = args.get('delay')
	dev_addr = args.get('dev_addr')
	int_addr = args.get('int_addr')
	iaddr_bytes = args.get('iaddr_bytes')

	# Open i2c bus
	if pylibi2c.open('/dev/i2c-{0:d}'.format(bus), delay) != 0:
		print "Open i2c bus:{0:d} error!".format(bus)
		sys.exit(-1)


	# Create read / write buffer
	buf = ctypes.create_string_buffer(size)

	# Ioctl r /w
	if ioctl:
		if write:
			for i in range(size):
				buf[i] = chr(data & 0xff)
				data += 1

			if pylibi2c.ioctl_write(dev_addr, int_addr, iaddr_bytes, buf, size, 0) != size:
				print "Write error!"
				sys.exit(-1)

		else:
			if pylibi2c.ioctl_read(dev_addr, int_addr, iaddr_bytes, buf, size, 0) != size:
				print "Read error!"
				sys.exit(-1)

	# File r / w
	else:
		if write:
			for i in range(size):
				buf[i] = chr(data & 0xff)
				data += 1

			if pylibi2c.write(dev_addr, int_addr, iaddr_bytes, buf, size) != size:
				print "Write error!"
				sys.exit(-1)

		else:
			if pylibi2c.read(dev_addr, int_addr, iaddr_bytes, buf, size) != size:
				print "Read error!"
				sys.exit(-1)

	# Print read / write data
	print "Write data:" if write else "Read data:"

	for i in range(size):
		if i % 16 == 0:
			print ""
		print "{0:02x} ".format(ord(buf[i])),

