/*********************************************************(**************************************************************
**	FileName:i2c_lib.c
**	Date: Mar 17 2010 Ver 0.1
**	Description: I2C E2PROM temperature sensor read write operation 
************************************************************************************************************************/

/************************I2C Function Call relationship****************************
	App			xxxx_xxxx()				such as	:	vcom_adj() user_settings_oper()
	Mid			xxxx_oper()				such as : 	xxxx_vcom_oper() e2prom_oper()
	I2C			i2c_oper()				such as : 	i2c_oper() or i2c_ioctl_oper()
	I2C			i2c_read/write()		such as :	i2c_read/write()      
	Sys(Driver)	read/write()			such as	:	read/write()  ioctl()
***********************************************************************************/

#include "i2c.h"
#include <stdio.h>
#include <unistd.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>	/* I2C */

static int i2c_file;
static unsigned int i2c_dev_addr;


//////////////////////以下函数只在本文件内调用，给以上函数提供支持，上层应用程序不用调用//////////////////////////////////
static void i2c_delay(unsigned int delay);
static int i2c_select(unsigned int dev_addr);
static void i2c_int_addr_conv(unsigned int int_addr, unsigned int addr_bit, unsigned char *addr);
static int i2c_read(unsigned int int_addr, unsigned int addr_bit, unsigned char *buf, size_t count);
static int i2c_write(unsigned int int_addr, unsigned int addr_bit, const unsigned char *buf, size_t count);
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
**	@brief		:	open i2c device name
**	@dev_name	:	devoce name
**	@return		:	failed return -1
*/
int i2c_init(const char *dev_name)
{
	/* Open i2c-bus devcice */
	if ((i2c_file = open(dev_name, O_RDWR)) == -1){

		perror("Open i2c device error:");
		return -1;
	}

	return i2c_file;
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
	if ((ret = i2c_select(msg->dev_addr)) == 0){

		/*	According to user specify operation to call basic i2c function to read or write data */
		switch (msg->oper)	{

			case	LOAD_OPER	:	ret = i2c_read	(msg->int_addr, msg->addr_bit, msg->buf, msg->len);break;
			case	STORE_OPER	:	ret = i2c_write (msg->int_addr, msg->addr_bit, msg->buf, msg->len);break;
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
**		oper	:	load oper or store oper, load 0, store 1 
**		dev_addr:	i2c device address 
**		int_addr:	i2c internal address
**		addr_bit:	i2c internal address length, 1 2 or 3 or zero(ignor internal address)	
**		flags	:	operaction flags if it set flag XXX:I2C_M_IGNORE_NAK i2c will ingor ack signal 
**		buf		:	if it was load oper, it using cache load data
**					if it was store oper, it using cache send data 
**		len		:	load or srore data size(bytes), it must less than buf size  
*/

int i2c_ioctl_oper(P_I2C_MSG msg)
{
	int ret;
	unsigned char int_addr[32];
	struct i2c_msg ioctl_msg[2];
	struct i2c_rdwr_ioctl_data ioctl_data;

	bzero(int_addr, sizeof(int_addr));
	bzero(ioctl_msg, sizeof(ioctl_msg));
	bzero(&ioctl_data,sizeof(struct i2c_rdwr_ioctl_data));		

	/*	Do this just in case that then did not have a internal addr */
	if (msg->addr_bit){

		/* Conversion i2c internal address if it have the internal addr */
		i2c_int_addr_conv(msg->int_addr, msg->addr_bit, int_addr);	
	}

	/* Read Oper 
	*********************************************************************/	
	if (msg->oper == LOAD_OPER){

		/*	Target have internal address */
		if (msg->addr_bit){

			/*	First message is write internal address */
			ioctl_msg[0].len  	= 	msg->addr_bit;	
			ioctl_msg[0].addr 	= 	msg->dev_addr;
			ioctl_msg[0].buf  	= 	int_addr;
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
		else{
			
			/*	Second message is read data */	
			ioctl_msg[0].len	= 	msg->len;
			ioctl_msg[0].addr	= 	msg->dev_addr;
			ioctl_msg[0].buf	=	msg->buf;	
			ioctl_msg[0].flags	|=	I2C_M_RD;

			/* Package to i2c message to operation i2c device */
			ioctl_data.nmsgs	=	1;
			ioctl_data.msgs		=	ioctl_msg;
		}
	}
	/* Write oper 
	************************************************************************/
	else{

		/* Connnect data operation after device internal address */
		bcopy(msg->buf, int_addr + msg->addr_bit, msg->len);	
	
		/* Fill kernel ioctl i2c_msg */
		ioctl_msg[0].len	=	msg->addr_bit + msg->len;
		ioctl_msg[0].addr	=	msg->dev_addr;	
		ioctl_msg[0].buf	=	int_addr;
		ioctl_msg[0].flags	=	msg->flags;

		/* Package to i2c message to operation i2c device */
		ioctl_data.nmsgs	=	1;
		ioctl_data.msgs		=	ioctl_msg;	
	}

	/* Select i2c device */

	/* Using ioctl interface operation i2c device */		
	if((ret = ioctl(i2c_file, I2C_RDWR, (unsigned long)&ioctl_data)) > 0){

		return msg->len;

	}else{

		return ret;
	}
}

/******************************************************************************************************
	@brief		:	read #count bytes data from #int_addr to #buf
	@int_addr	:	i2c_device internal address will read data from this address , no address set zero
	@addr_bit	:	i2c devide internal address length 
	@buf		:	i2c data will read to here
	@count		:	how many data to read, count must less than or equal to buf size 
	@return 	: 	success return read data  length , failed -1
******************************************************************************************************/
static int i2c_read(unsigned int int_addr, unsigned int addr_bit, unsigned char *buf, size_t count)
{	
	size_t cnt;
	unsigned char addr[3];
	
	/* Calculate internal address save them to addr */
	i2c_int_addr_conv(int_addr, addr_bit, addr);
	
	/* Write internal address to devide  */
	if (write(i2c_file, addr, addr_bit) != addr_bit){

		perror("Write i2c internal address error:");
		return -1;
	}	

	/* Sleep a while */
	i2c_delay(10);

	/* Read count bytes data from in_taddr specify address */
	if ((cnt = read(i2c_file, buf, count)) < 0){

		perror("Read i2c data error:");	
		return -1;
	}

	/* Success return read counter */
	return cnt;
}



/******************************************************************************************************
	@brief		:	write buf data to i2c device 
	@int_addr	: 	i2c_device internal address, no address set zero
	@addr_bit	:	i2c devide internal address length 
	@buf		:	data will write to i2c device 
	@count		:	buf data length without '/0'
	@return 	: 	success return write data length , failed -1
******************************************************************************************************/
static int i2c_write(unsigned int int_addr, unsigned int addr_bit, const unsigned char *buf, size_t count)
{
	size_t cnt = 0;
	unsigned char tmp_buf[7], addr[3];
	
	i2c_delay(10);
		
	/* Once only can write less than 4 byte */
	for (; count >= 4; count -= 4)	{
		/* Calculate internal and save them to addr array */
		i2c_int_addr_conv(int_addr, addr_bit, addr);
		
		/* Clear write buffer */
		bzero(tmp_buf,  sizeof(tmp_buf));
	
		/* Copy i2c internal address to write buffer */
		bcopy(addr, tmp_buf, addr_bit);

		/* Copy data to tmp_buf */
		bcopy(buf, tmp_buf + addr_bit, 4);
		
		/* Write to buf content to i2c device length is address length and write buffer length */
		if (write(i2c_file, tmp_buf, addr_bit + 4) != addr_bit + 4){

			perror("I2C write error:");
			return -1;
		}
	
		/* XXX: Must have a little time delay */
		i2c_delay(1);
	
		buf  += 4;		/* to next 4 byte */
		cnt  += 4;		/* success write count */
		int_addr += 4;	/* increase internal address to new position */
	}

	/* Data length is less than 4bytes */
	if (count){	

		/* Internal address conversion */
		i2c_int_addr_conv(int_addr, addr_bit, addr);

		/* Clear write buffer */
		bzero(tmp_buf,  sizeof(tmp_buf));
	
		/* Copy i2c internal address to write buffer */
		bcopy(addr, tmp_buf, addr_bit);

		/* Copy write content to tmp_buf */
		bcopy(buf, tmp_buf + addr_bit, count);
	
		i2c_delay(10);	

		/* Write to buf content to i2c device length is address length and write buffer length */
		if (write(i2c_file, tmp_buf, addr_bit + count) != addr_bit + count){

			perror("I2C write error:");
			return -1;
		}
		else{

			/* Return success write byte size */
			return cnt + count;
		}		
	}
	/* Write data length just mutiple by 4 */
	else{

		/* Return success write byte size */
		return cnt;
	}
}


/******************************************************************************************************
	@brief		:	i2c internal address convert
	@int_addr	:	i2c device internal address	
	@addr_bit	:	i2c device internal address length	
	@addr		:	arrary to save after convert address 
******************************************************************************************************/
static void i2c_int_addr_conv(unsigned int int_addr, unsigned int addr_bit, unsigned char *addr)
{
	bzero(addr, 3);
	
	/* Analytic internal address to save them to a addr arry */

	/* One byte internal address */
	if(int_addr <= 0xFF){	

		addr[addr_bit - 1]	= int_addr & 0xFF;
	}
	/* Two byte internal address */
	else if (int_addr > 0xFF && int_addr <= 0xFFFF) {			
		addr[addr_bit - 2]	= ((int_addr>>8) & 0xff);			
		addr[addr_bit - 1]	= (int_addr & 0xFF);
	}
	/* Three byte internal address */
	else if (int_addr > 0xFFFF && int_addr <= 0xFFFFFF) {		
		addr[addr_bit - 3]	= (int_addr 	& 0xFFFFFF) >> 16;
		addr[addr_bit - 2]	= ((int_addr & 0xFFFFFF) >> 8) & 0xFF;
		addr[addr_bit - 1]	= (int_addr  & 0xFFFFFF) & 0xFF;	
	}
}

/******************************************************************************************************
	@brief		:	Select i2c address @i2c bus
	@i2c_addr	:	i2c device address
	@return		:	success return 0, failed return -1
******************************************************************************************************/
static int i2c_select(unsigned int i2c_addr)
{
	/* Set i2c address is 7bits */
	if (ioctl(i2c_file, I2C_TENBIT, 0)){

		perror("Set i2c 7bits address failed:");
		return -1;
	}	

	/* Set i2c device as slave ans set it address */
	if (ioctl(i2c_file, I2C_SLAVE, i2c_addr)){

		perror("Set i2c address failed:");
		return -1;
	}

	/* Save i2c device address */
	i2c_dev_addr = i2c_addr;

	return 0;
}

/******************************************************************************************************
	@brief	:	i2c delay 
	@delay	:	how many times delay 
******************************************************************************************************/
static void i2c_delay(unsigned int delay)
{
	usleep(delay);
}


