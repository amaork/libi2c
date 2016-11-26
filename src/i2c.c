/*********************************************************(**************************************************************
**	FileName:i2c_lib.c
**	Date: Mar 17 2010 Ver 0.1
**	Description: I2C E2PROM temperature sensor read write operation 
************************************************************************************************************************/

/************************I2C Function Call relationship**************************************
	App			xxxx_xxxx()				such as	:	vcom_adj() / user_settings_oper()
	Mid			xxxx_oper()				such as : 	xxxx_vcom_oper() / e2prom_oper()
	I2C			i2c_oper()				such as : 	i2c_oper() / i2c_ioctl_oper()
	I2C			i2c_read/write()		such as :	i2c_read/ i2c_write()      
	Sys(Driver)	read/write()			such as	:	read/write() ioctl()
********************************************************************************************/
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include "i2c.h"

#define INT_DELAY_MAX 50

/* Internal address max length */
#define INT_ADDR_MAX_BYTES 3

/* File I/O write max length */
#define WRITE_MAX_BYTES 4

/* Ioctl write max length */
#define IOCTL_MAX_BYTES 16

static int i2c_file;
static unsigned int int_delay;


//////////////////////以下函数只在本文件内调用，给以上函数提供支持，上层应用程序不用调用//////////////////////////////////
static void i2c_delay(unsigned int delay);
static void i2c_int_addr_conv(unsigned int int_addr, unsigned int iaddr_bytes, unsigned char *addr);

static int i2c_select(unsigned int dev_addr);
static int i2c_read(unsigned int int_addr, unsigned int iaddr_bytes, unsigned char *buf, size_t count);
static int i2c_write(unsigned int int_addr, unsigned int iaddr_bytes, const unsigned char *buf, size_t count);
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/*
**	@brief		:	open i2c device name
**	@bus_name	:	i2c bus name such as: /dev/i2c-1
**	@delay		:	internal delay unit microsecond
**	@return		:	failed return -1, success return 0
*/
int i2c_init(const char *bus_name, unsigned int delay)
{
	/* Open i2c-bus devcice */
	if ((i2c_file = open(bus_name, O_RDWR)) == -1) {

		perror("Open i2c device error:");
		return -1;
	}

	int_delay = delay;
	if (int_delay > INT_DELAY_MAX) {

		int_delay = INT_DELAY_MAX;
	}

	return 0;
}


/*	
**	I2C bus top layer interface to operation different i2c device 	
**	This function will call XXX:read/write system call and be relate 
**	i2c-dev.c i2cdev_read/write to operation i2c device it must 
**	had to notice i2c bus ack signal 
*/
int i2c_oper(P_I2C_MSG msg)
{
	int ret;	

	/*	Set to select different i2c devie */
	if ((ret = i2c_select(msg->dev_addr)) == 0) {

		/*	According to user specify operation to call basic i2c function to read or write data */
		switch (msg->oper) {

			case I2C_LOAD	: ret = i2c_read(msg->int_addr, msg->iaddr_bytes, msg->buf, msg->len);break;
			case I2C_STORE	: ret = i2c_write(msg->int_addr, msg->iaddr_bytes, msg->buf, msg->len);break;
		}
	}

	return ret;
}

/* 	
**	I2C bus top layer interface to operation different i2c devide 
**	This function will call XXX:ioctl system call and will be related 
**	i2c-dev.c i2cdev_ioctl to operation i2c device.
**	1. it can choice ignore or not ignore i2c bus ack signal 
**	2. it can choice ignore or not ignore i2c internal address 
**	
**	I2C_MSG struct need fill this members:
**		oper		:	load oper or store oper, load 0, store 1 
**		dev_addr	:	i2c device address 
**		int_addr	:	i2c internal address
**		iaddr_bytes	:	i2c internal address length, 1 2 or 3 or zero(ignor internal address)	
**		flags		:	operaction flags if it set flag XXX:I2C_M_IGNORE_NAK i2c will ingor ack signal 
**		buf			:	if it was load oper, it using cache load data
**						if it was store oper, it using cache send data 
**		len			:	load or srore data size(bytes), it must less than buf size  
*/

int i2c_ioctl_oper(P_I2C_MSG msg)
{
	struct i2c_msg ioctl_msg[2];
	struct i2c_rdwr_ioctl_data ioctl_data;
	unsigned char tmp_buf[INT_ADDR_MAX_BYTES + IOCTL_MAX_BYTES];

	memset(tmp_buf, 0, sizeof(tmp_buf));
	memset(ioctl_msg, 0, sizeof(ioctl_msg));
	memset(&ioctl_data, 0, sizeof(ioctl_data));

	/* Read */
	if (msg->oper == I2C_LOAD) {

		/*	Target have internal address */
		if (msg->iaddr_bytes) {

			i2c_int_addr_conv(msg->int_addr, msg->iaddr_bytes, tmp_buf);

			/*	First message is write internal address */
			ioctl_msg[0].len  	= 	msg->iaddr_bytes;	
			ioctl_msg[0].addr 	= 	msg->dev_addr;
			ioctl_msg[0].buf  	= 	tmp_buf;
			ioctl_msg[0].flags	= 	msg->flags;

			/*	Second message is read data */	
			ioctl_msg[1].len	= 	msg->len;
			ioctl_msg[1].addr	= 	msg->dev_addr;
			ioctl_msg[1].buf	=	msg->buf;	
			ioctl_msg[1].flags	|=	I2C_M_RD;

			/* Package to i2c message to operation i2c device */
			ioctl_data.nmsgs	=	2;
			ioctl_data.msgs		=	ioctl_msg;
		}
		/* Target did not have internal address */
		else {
			
			/*	Second message is read data */	
			ioctl_msg[0].len	= 	msg->len;
			ioctl_msg[0].addr	= 	msg->dev_addr;
			ioctl_msg[0].buf	=	msg->buf;	
			ioctl_msg[0].flags	|=	I2C_M_RD;

			/* Package to i2c message to operation i2c device */
			ioctl_data.nmsgs	=	1;
			ioctl_data.msgs		=	ioctl_msg;
		}

		/* Using ioctl interface operation i2c device */
		if (ioctl(i2c_file, I2C_RDWR, (unsigned long)&ioctl_data) == -1) {

			perror("Ioctl read i2c error:");
			return -1;
		}

		return msg->len;
	}
	/* Write */
	else if (msg->oper == I2C_STORE) {

		size_t size = 0, cnt = 0;
		size_t remain = msg->len;
		unsigned int int_addr = msg->int_addr;

		while (remain > 0) {

			size = remain >= IOCTL_MAX_BYTES ? IOCTL_MAX_BYTES : remain;

			/* Convert i2c internal address */
			memset(tmp_buf, 0, sizeof(tmp_buf));
			i2c_int_addr_conv(int_addr, msg->iaddr_bytes, tmp_buf);

			/* Connect write data after device internal address */
			memcpy(tmp_buf + msg->iaddr_bytes, msg->buf + cnt, size);

			/* Fill kernel ioctl i2c_msg */
			memset(ioctl_msg, 0, sizeof(ioctl_msg));
			memset(&ioctl_data, 0, sizeof(ioctl_data));

			ioctl_msg[0].len	=	msg->iaddr_bytes + size;
			ioctl_msg[0].addr	=	msg->dev_addr;
			ioctl_msg[0].buf	=	tmp_buf;
			ioctl_msg[0].flags	=	msg->flags;

			ioctl_data.nmsgs	=	1;
			ioctl_data.msgs		=	ioctl_msg;

			if (ioctl(i2c_file, I2C_RDWR, (unsigned long)&ioctl_data) == -1){

				perror("Ioctl write i2c error:");
				return -1;
			}

			/* XXX: Must have a little time delay */
			i2c_delay(int_delay);

			cnt += size;
			remain -= size;
			int_addr += size;
		}

		return cnt;

	} /* end of I2C_STORE */

	return -1;
}


/*
**	@brief		:	read #count bytes data from #int_addr to #buf
**	#int_addr	:	i2c_device internal address will read data from this address, no address set zero
**	#iaddr_bytes:	i2c devide internal address length 
**	#buf		:	i2c data will read to here
**	#len		:	how many data to read, lenght must less than or equal to buf size 
**	@return 	: 	success return read data length, failed -1
*/
static int i2c_read(unsigned int int_addr, unsigned int iaddr_bytes, unsigned char *buf, size_t len)
{	
	size_t cnt;
	unsigned char addr[INT_ADDR_MAX_BYTES];
	
	/* Convert i2c internal address */
	i2c_int_addr_conv(int_addr, iaddr_bytes, addr);
	
	/* Write internal address to devide  */
	if (write(i2c_file, addr, iaddr_bytes) != iaddr_bytes) {

		perror("Write i2c internal address error:");
		return -1;
	}	

	/* Sleep a while */
	i2c_delay(int_delay);

	/* Read count bytes data from int_addr specify address */
	if ((cnt = read(i2c_file, buf, len)) < 0) {

		perror("Read i2c data error:");	
		return -1;
	}

	/* Success return read counter */
	return cnt;
}


/*
**	@brief		:	write #buf data to i2c device 
**	@int_addr	: 	i2c_device internal address, no address set zero
**	@iaddr_bytes:	i2c devide internal address length 
**	@buf		:	data will write to i2c device 
**	@count		:	buf data length without '/0'
**	@return 	: 	success return write data length, failed -1
*/
static int i2c_write(unsigned int int_addr, unsigned int iaddr_bytes, const unsigned char *buf, size_t len)
{
	size_t remain = len;
	size_t cnt = 0, size = 0;
	unsigned char tmp_buf[WRITE_MAX_BYTES + INT_ADDR_MAX_BYTES];

	/* Once only can write less than 4 byte */
	while (remain > 0) {

		size = remain >= WRITE_MAX_BYTES ? WRITE_MAX_BYTES : remain;

		/* Convert i2c internal address */
		memset(tmp_buf, 0, sizeof(tmp_buf));
		i2c_int_addr_conv(int_addr, iaddr_bytes, tmp_buf);

		/* Copy data to tmp_buf */
		memcpy(tmp_buf + iaddr_bytes, buf + cnt, size);

		/* Write to buf content to i2c device length is address length and write buffer length */
		if (write(i2c_file, tmp_buf, iaddr_bytes + size) != iaddr_bytes + size) {

			perror("I2C write error:");
			return -1;
		}

		/* XXX: Must have a little time delay */
		i2c_delay(int_delay);

		/* Move to next #size bytes */
		cnt += size;
		remain -= size;
		int_addr += size;
	}

	return cnt;
}


/*
**	@brief		:	i2c internal address convert
**	#int_addr	:	i2c device internal address	
**	#iaddr_bytes:	i2c device internal address length	
**	#addr		:	arrary to save after convert address 
*/
static void i2c_int_addr_conv(unsigned int int_addr, unsigned int iaddr_bytes, unsigned char *addr)
{
	memset(addr, 0, iaddr_bytes);
	
	/* One byte internal address */
	if (int_addr <= 0xFF) {	

		addr[iaddr_bytes - 1] = int_addr & 0xFF;
	}
	/* Two byte internal address */
	else if (int_addr > 0xFF && int_addr <= 0xFFFF) {			

		addr[iaddr_bytes - 2] = ((int_addr >> 8) & 0xff);			
		addr[iaddr_bytes - 1] = (int_addr & 0xFF);
	}
	/* Three byte internal address */
	else if (int_addr > 0xFFFF && int_addr <= 0xFFFFFF) {		

		addr[iaddr_bytes - 3] = (int_addr & 0xFFFFFF) >> 16;
		addr[iaddr_bytes - 2] = ((int_addr & 0xFFFFFF) >> 8) & 0xFF;
		addr[iaddr_bytes - 1] = (int_addr & 0xFFFFFF) & 0xFF;	
	}
}


/*
**	@brief		:	Select i2c address @i2c bus
**	#dev_addr	:	i2c device address
**	#return		:	success return 0, failed return -1
*/
static int i2c_select(unsigned int dev_addr)
{
	/* Set i2c address is 7bits */
	if (ioctl(i2c_file, I2C_TENBIT, 0)) {

		perror("Set i2c 7bits address failed:");
		return -1;
	}	

	/* Set i2c device as slave ans set it address */
	if (ioctl(i2c_file, I2C_SLAVE, dev_addr)) {

		perror("Set i2c address failed:");
		return -1;
	}

	return 0;
}

/*
**	@brief	:	i2c delay 
**	#msec	:	microsecond to be delay
*/
static void i2c_delay(unsigned int msec)
{
	usleep(msec * 1000);
}


