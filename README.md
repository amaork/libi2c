libi2c
=======

Linux userspace i2c library.


## Features

- Support C/C++ and Python.

- Support Python2+, Python3+

- Support multiple bus and devices.

- Support 7-bit and 10-bit i2c slave address.

- Support 1 - 4 byte internal address, auto convert.

- Provide read/write/ioctl functions to operate i2c device.

- Support 8/16/32/64/128/256 bytes page aligned write, read/write length are unlimited.

- Using ioctl functions operate i2c can ignore i2c device ack signal and internal address.


## Installation

	sudo python setup.py install

	or

	sudo make install

	or

	sudo make install PYTHON=pythonX.X

## Interface

	i2c_ioctl_write (once max 16 bytes) are more efficient than i2c_write (once max 4 bytes).

	/* Close i2c bus */
	void i2c_close(int bus);

	/* Open i2c bus, return i2c bus fd */
	int i2c_open(const char *bus_name);

	/* I2C file I/O read, write */
	ssize_t i2c_read(const I2CDevice *device, unsigned int iaddr, void *buf, size_t len);
	ssize_t i2c_write(const I2CDevice *device, unsigned int iaddr, const void *buf, size_t len);

	/* I2c ioctl read, write can set i2c flags */
	ssize_t i2c_ioctl_read(const I2CDevice *device, unsigned int iaddr, void *buf, size_t len);
	ssize_t i2c_ioctl_write(const I2CDevice *device, unsigned int iaddr, const void *buf, size_t len);

## Data structure

**C/C++**

	typedef struct i2c_device {
		int bus;			/* I2C Bus fd, return from i2c_open */
		unsigned short addr;		/* I2C device(slave) address */
		unsigned char tenbit;		/* I2C is 10 bit device address */
		unsigned char delay;		/* I2C internal operate delay, unit millisecond */
		unsigned short flags;		/* I2C i2c_ioctl_read/write flags */
		unsigned int page_bytes;    	/* I2C max number of bytes per page, 1K/2K 8, 4K/8K/16K 16, 32K/64K 32 etc */
		unsigned int iaddr_bytes;	/* I2C device internal(word) address bytes, such as: 24C04 1 byte, 24C64 2 bytes */
	}I2CDevice;

**Python**

	I2CDevice object
	I2CDevice(bus, addr, tenbit=False, iaddr_bytes=1, page_bytes=8, delay=1, flags=0)
	tenbit, delay, flags, page_bytes, iaddr_bytes are attributes can setter/getter after init

	required args: bus, addr.
	optional args: tenbit(defult False, 7-bit), delay(defualt 1ms), flags(defualt 0), iaddr_bytes(defualt 1 byte internal address), page_bytes(default 8 bytes per page).


## C/C++ Usage

**1. First call `i2c_open` open i2c bus.**

	int bus;

	/* Open i2c bus /dev/i2c-0 */
	if ((bus = i2c_open("/dev/i2c-0")) == -1) {

		/* Error process */
	}

**2. Second fill `I2CDevice` struct, prepare read or write.**

	I2CDevice device;
	memset(&device, 0, sizeof(device));

	/* 24C04 */
	device.bus = bus;	/* Bus 0 */
	device.addr = 0x50;	/* Slave address is 0x50, 7-bit */
	device.iaddr_bytes = 1;	/* Device internal address is 1 byte */
	device.page_bytes = 16; /* Device are capable of 16 bytes per page */

**3. Call `i2c_read/write` or `i2c_ioctl_read/write` read or write i2c device.**

	unsigned char buffer[256];
	ssize_t size = sizeof(buffer);
	memset(buffer, 0, sizeof(buffer));

	/* From i2c 0x0 address read 256 bytes data to buffer */
	if ((i2c_read(&device, 0x0, buffer, size)) != size) {

		/* Error process */
	}

**4. Close i2c bus `i2c_close(bus)`.**

	i2c_close(bus);

## Python Usage

	import ctypes
	import pylibi2c

	# Open i2c device @/dev/i2c-0, addr 0x50.
	i2c = pylibi2c.I2CDevice('/dev/i2c-0', 0x50)

	# Open i2c device @/dev/i2c-0, addr 0x50, 16bits internal address
	i2c = pylibi2c.I2CDevice('/dev/i2c-0', 0x50, iaddr_bytes=2)

	# Set delay
	i2c.delay = 10

	# Set page_bytes
	i2c.page_bytes = 16

	# Set flags
	i2c.flags = pylibi2c.I2C_M_IGNORE_NAK

	# Python2
	buf = bytes(bytearray(256))

	# Python3
	buf = bytes(256)

	# Write data to i2c, buf must be read-only type
	size = i2c.write(0x0, buf)

	# From i2c 0x0(internal address) read 256 bytes data, using ioctl_read.
	data = i2c.ioctl_read(0x0, 256)

## Notice

1. If i2c device do not have internal address, please use `i2c_ioctl_read/write` function for read/write, set`'iaddr_bytes=0`.

2. If want ignore i2c device nak signal, please use `i2c_ioctl_read/write` function, set I2CDevice.falgs as  `I2C_M_IGNORE_NCK`.
