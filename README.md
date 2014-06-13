libi2c
=======

## Description

Linux userspace i2c operating library


## Feature

- Support single, double, three byte i2c internal address

- Provide read/write/ioctl system call to operating i2c device

- Support 4 byte aligned write, read/write length are unlimited

- Using ioctl system call operate i2c can ignore i2c device ack signal and internal address

## Interface

	/* Initialize i2c bus */
	int i2c_init(const char *dev_name);

	/* Using read/write system call operate i2c device */
	int i2c_oper(P_I2C_MSG msg);

	/* Using ioctl system call operate i2c device */
	int i2c_ioctl_oper(P_I2C_MSG msg);
 	

## Data structure
	
	typedef struct{
        unsigned int    oper;                           /*      I2C operate, read or write */
        unsigned char   *buf;                           /*      I2c read or write data buffer */
        unsigned short  flags;                          /*      Kernel layer i2c operate flags */
        unsigned int    len;                            /*      I2C read or write len   */
        unsigned char   dev_addr;                       /*      I2C slave addresss      */
        unsigned int    int_addr;                       /*      I2C device internal address     */
        unsigned int    addr_bit;                       /*      I2C device internal address lengeh */
	}I2C_MSG, *P_I2C_MSG;

## Usage

1. First call `i2c_init` initialize i2c device

		if (i2c_init("/dev/i2c-0") == -1){

			/* Error process */
		}

2. Second fill `I2C_MSG` struct, prepare read or write 

		I2C_MSG msg
		unsigned char buffer[128];
	
		bzero(&msg, sizeof(msg));
		bzero(buffer, sizeof(buffer));

		msg.oper	=	OPER_LOAD;	/*	Read */
		msg.buf		=	buffer;		/*	Read buffer */
		msg.len		=	32;			/*	Read 32 byte form i2c deivce to buffer */
		msg.dev_addr= 	0x7A;		/*	I2C device address */
		msg.int_addr=	0x10;		/*	Device internal address */
		msg.addr_bit=	1;			/*	Device internal address is 1 byte */

3. Call `i2c_oper` or `i2c_ioctl_oper` read or write i2c device

		int ret;

		if ((ret = i2c_oper(&msg)) != msg.len){

			/* Error process */
			if (ret == -1){

			}
			/* Read data less than msg.len */
			else{

				/* Do something */
			}
		}

		
## Notice

1. If i2c device do not have internal address, please use `i2c_ioctl_oper` function for read/write 

2. If want ignore i2c device ack signal, please  use `i2c_ioctl_oper` function, set I2C_MSG.falgs as  `I2C_M_IGNORE_ACK`