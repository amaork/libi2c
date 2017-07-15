libi2c
=======

Linux userspace i2c library.


## Features

- Support C/C++ and Python.

- Support Python 2+, Python3+

- Support multiple bus and devices.

- Support 7-bit and 10-bit i2c slave address.

- Support 1 - 4 byte internal address, auto convert.

- Provide read/write/ioctl functions to operate i2c device.

- Support 4/16 byte aligned write, read/write length are unlimited.

- Using ioctl functions operate i2c can ignore i2c device ack signal and internal address.

## Installation

	sudo python setup.py install

	or

	sudo make install

	or

	sudo make install PYTHON=python3.5

## Interface

	i2c_ioctl_write (once max 16 bytes) are more efficient than i2c_write (once max 4 bytes).

	/* Close i2c bus */
	void i2c_close(int bus);

	/* Open i2c bus, return i2c bus fd */
	int i2c_open(unsigned char bus_num);

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
		unsigned char delay;		/* I2C internal operation delay, unit microsecond */
		unsigned short flags;		/* I2C i2c_ioctl_read/write flags */
		unsigned short iaddr_bytes;	/* I2C device internal address bytes, such as: 24C04 1 byte, 24C64 2 bytes */
	}I2CDevice;

**Python device dict:**

	required keys: bus, addr.
	optional keys: tenbit(defult 0, 7-bit), delay(defualt 5ms), flags(defualt 0), iaddr_bytes(defualt 1 byte internal address).


## C/C++ Usage

**1. First call `i2c_open` open i2c bus.**

		int bus;

		/* Open i2c bus 0, /dev/i2c-0 */
		if ((bus = i2c_open(0)) == -1) {

			/* Error process */
		}

**2. Second fill `I2CDevice` struct, prepare read or write.**

		I2CDevice device;
		memset(&device, 0, sizeof(device));

		device.bus	= bus;	/* Bus 0 */
		device.addr = 0x50;	/* Slave address is 0x50, 7-bit */
		device.iaddr_bytes = 1;	/* Device internal address is 1 byte */

**3. Call `i2c_read/write` or `i2c_ioctl_read/write` read or write i2c device.**

		unsigned char buffer[256];
		ssize_t size = sizeof(buffer);
		memset(buffer, 0, sizeof(buffer));

		/* From i2c 0x0 address read 256 bytes data to buffer */
		if (i2c_read(&device, 0x0, buffer, size)) != size) {

			/* Error process */
		}

**4. Close i2c bus `i2c_close(bus)`.**

		i2c_close(bus);

## Python Usage

	import ctypes
	import pylibi2c

	# Open i2c bus 0.
	bus = pylibi2c.open(0)
	if bus == -1:
		# Error process
		pass

	# Fill I2CDevice dict.
	device = {"bus": bus, "addr": 0x50}

	# From i2c 0x0 address read 256 bytes data to buf.
	buf = ctypes.create_string_buffer(256)
	size = pylibi2c.ioctl_read(device, 0x0, buf, 256)


## Notice

1. If i2c device do not have internal address, please use `i2c_ioctl_read/write` function for read/write.

2. If want ignore i2c device ack signal, please use `i2c_ioctl_read/write` function, set I2CDevice.falgs as  `I2C_M_IGNORE_ACK`.
