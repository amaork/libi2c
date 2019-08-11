#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "i2c.h"

int main(int argc, char **argv)
{
    int fd;
    I2CDevice device;
    const char *data = "9876543";

    /* First open i2c bus */
    if ((fd = i2c_open("/dev/i2c-0")) == -1) {

        perror("Open i2c bus error");
        return -1;
    }

    /* Fill i2c device struct */
    device.bus = fd;
    device.addr = 0x12;
    device.tenbit = 0;
    device.delay = 10;
    device.flags = 0;
    device.page_bytes = 8;
    device.iaddr_bytes = 0; /* Set this to zero, and using i2c_ioctl_xxxx API will ignore chip internal address */

    /* Write data to i2c */
    if (i2c_ioctl_write(&device, 0x0, data, strlen(data)) != strlen(data)) {

        /* Error process */
    }

    i2c_close(fd);
    return 0;

}

