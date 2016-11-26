#ifndef _LIB_I2C_H_
#define _LIB_I2C_H_

#ifdef  __cplusplus
extern "C" {
#endif

#include <linux/i2c.h>
#include <linux/i2c-dev.h>

/* 
	I2C_LOAD: read operation, load data from i2c to memory 
	I2C_STORE: write operation, store data from memory to i2c
*/
enum I2C_OPER{I2C_LOAD, I2C_STORE};

/* Data structure */
typedef struct{
	unsigned int	oper;				/*	I2C operate, I2C_OPER */
	unsigned char	*buf;				/*	I2c read or write data buffer */
	unsigned int	len;				/*	I2C read or write length */
	unsigned short	flags;				/*	Kernel layer i2c operate flags */
	unsigned char	dev_addr;			/*	I2C slave addresss	*/
	unsigned int	int_addr;			/*	I2C device internal address	*/	
	unsigned int	iaddr_bytes;		/*	I2C device internal address length 1, 2, 3 bytes, 24C04 1, 24C64 2 etc */
}I2C_MSG, *P_I2C_MSG;

/* Function declaration */
int i2c_oper(P_I2C_MSG msg);
int i2c_ioctl_oper(P_I2C_MSG msg);
int i2c_init(const char *bus_name, unsigned int delay);

/* i2c operation handle pointer */
typedef int (*I2C_OPER_HANDLE)(P_I2C_MSG msg);


#ifdef  __cplusplus
}
#endif

#endif

