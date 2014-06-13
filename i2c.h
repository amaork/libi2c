#ifndef _I2C_INTERFACE_H_
#define _I2C_INTERFACE_H_


/* Macro */
#define	LOAD_OPER	0		/* 	Read operation load data form i2c to memory */
#define STORE_OPER	1		/* 	Write operation store data from memory to i2c  */

/* Data structure */
typedef struct{
	unsigned int	oper;				/*	I2C operate, read or write */
	unsigned char	*buf;				/*	I2c read or write data buffer */
	unsigned short	flags;				/*	Kernel layer i2c operate flags */
	unsigned int	len;				/*	I2C read or write len	*/
	unsigned char	dev_addr;			/*	I2C slave addresss	*/
	unsigned int	int_addr;			/*	I2C device internal address	*/	
	unsigned int	addr_bit;			/*	I2C device internal address lengeh */			
}I2C_MSG, *P_I2C_MSG;

/* Function declaration */
int i2c_oper(P_I2C_MSG msg);
int i2c_ioctl_oper(P_I2C_MSG msg);
int i2c_init(const char *dev_name);

#endif

