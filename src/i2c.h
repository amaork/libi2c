#ifndef _LIB_I2C_H_
#define _LIB_I2C_H_

#ifdef  __cplusplus
extern "C" {
#endif

#include <linux/i2c.h>
#include <linux/i2c-dev.h>

/* I2c device */
typedef struct i2c_device {
	int bus;			/* I2C Bus fd, return from i2c_open */
	unsigned short addr;		/* I2C device(slave) address */
	unsigned char tenbit;		/* I2C is 10 bit device address */
	unsigned char delay;		/* I2C internal operation delay, unit millisecond */
	unsigned short flags;		/* I2C i2c_ioctl_read/write flags */
	unsigned short iaddr_bytes;	/* I2C device internal address bytes, such as: 24C04 1 byte, 24C64 2 bytes */
}I2CDevice;

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

/* I2C read / write handle function */
typedef ssize_t (*I2C_READ_HANDLE)(const I2CDevice *dev, unsigned int iaddr, void *buf, size_t len);
typedef ssize_t (*I2C_WRITE_HANDLE)(const I2CDevice *dev, unsigned int iaddr, const void *buf, size_t len);

#ifdef  __cplusplus
}
#endif

#endif

