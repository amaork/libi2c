#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "i2c.h"

void print_i2c_msg(const P_I2C_MSG msg)
{
	int i = 0;

	for (i = 0; i < msg->len; i++){

		if (i % 16 == 0){

			fprintf(stdout, "\n");
		}

		fprintf(stdout, "%02x ", msg->buf[i]);
	}

	fprintf(stdout, "\n");
}


int main(int argc, char** argv)
{
	I2C_MSG msg;
	I2C_OPER_HANDLE i2c_oper_handle = i2c_oper;
	
	int i;
	unsigned int addr;
	unsigned char buf[256];
	memset(&msg, 0, sizeof(msg));
	memset(buf, 0, sizeof(buf));

	if (argc < 4) {

		fprintf(stdout, "Usage:%s <bus> <dev_addr> <iaddr_bytes> [ioctl]\n"
						"Such as:\n"
						"\t24c04 i2c_test /dev/i2c-1 0x50 1\n"
						"\t24c64 i2c_test /dev/i2c-1 0x50 2\n"
						"\t24c64 i2c_test /dec/i2c-1 0x50 2 ioctl\n", argv[0]);
		exit(0);
	}

	/* If specify ioctl using ioctl r/w i2c */
	if (argc == 5 && (memcmp(argv[4], "ioctl", strlen("ioctl")) == 0)) {

		i2c_oper_handle = i2c_ioctl_oper;
		fprintf(stdout, "Using i2c_ioctl_oper r/w data\n");
	}
	else {

		i2c_oper_handle = i2c_oper;
		fprintf(stdout, "Using i2c_oper r/w data\n");
	}

	/* Get i2c device address */	
	if (sscanf(argv[2], "0x%x", &addr) != 1) {

		fprintf(stderr, "Can't parse i2c device address[%s]\n", argv[2]);
		exit(-1);
	}

	if (addr < 0x3 || addr > 0x77) {

		fprintf(stderr, "Invalid i2c device address:0x[%x], should be: 0x3 - 0x77\n", addr);
		exit(-1);
	}

	/* Get i2c internal address bytes */
	if (sscanf(argv[3], "%d", &msg.iaddr_bytes) != 1) {

		fprintf(stderr, "Can't parse i2c internal address bytes[%s]\n", argv[3]);
		exit(-2);
	}

	if (msg.iaddr_bytes <= 0 || msg.iaddr_bytes >= 4) {
		
		fprintf(stderr, "Ivalid i2c internal address bytes[%d], should be: 1 - 3\n", msg.iaddr_bytes);
		exit(-2);
	}

	/* Init i2c bus */	
	if (i2c_init(argv[1], 5) != 0) {

		fprintf(stderr, "Init i2c bus:%s error!\b", argv[1]);
		exit(-3);
	}

	/* Init I2C_MSG */
	msg.buf = buf;
	msg.len = sizeof(buf);
	msg.int_addr = 0x0;
	msg.dev_addr = addr & 0xff;

	/* Write 0 - 255 */
	msg.oper = I2C_STORE;

	/* I/O r/w 0x00 - 0xff */
	if (i2c_oper_handle == i2c_oper) {

		for (i = 0; i < msg.len; i++) {

			msg.buf[i] = i;
		}
	}
	/* ioctl r/w 0xff - 0x0 */
	else {

		for (i = 0; i < msg.len; i++) {

			msg.buf[i] = 0xff - i;
		}
	}

	/* Print before write */
	fprintf(stdout, "Write data:\n");
	print_i2c_msg(&msg);

	if (i2c_oper_handle(&msg) != msg.len) {

		fprintf(stderr, "Write i2c error!\n");
		exit(-4);
	}	

	fprintf(stdout, "\nWrite success, prepare read....\n");

	/* Read */
	usleep(100000);
	msg.oper = I2C_LOAD;
	memset(buf, 0, sizeof(buf));
	
	if (i2c_oper_handle(&msg) != msg.len) {

		fprintf(stderr, "Read i2c error!\n");
		exit(-5);
	}

	/* Print read result */
	fprintf(stdout, "Read data:\n");
	print_i2c_msg(&msg);

	return 0;
}


