#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "i2c.h"

void print_i2c_data(const unsigned char *data, size_t len)
{
    int i = 0;

    for (i = 0; i < len; i++) {

        if (i % 16 == 0) {

            fprintf(stdout, "\n");
        }

        fprintf(stdout, "%02x ", data[i]);
    }

    fprintf(stdout, "\n");
}


int main(int argc, char **argv)
{
    char i2c_dev_desc[128];
    I2C_READ_HANDLE i2c_read_handle = i2c_read;
    I2C_WRITE_HANDLE i2c_write_handle = i2c_write;
    unsigned int addr = 0, iaddr_bytes = 0, page_bytes = 0, bus_num = -1;

    if (argc < 5) {

        fprintf(stdout, "Usage:%s <bus_num> <dev_addr> <iaddr_bytes> <page_bytes> [ioctl]\n"
                "Such as:\n"
                "\t24c02 i2c_test 1 0x50 1 8\n"
                "\t24c04 i2c_test 1 0x50 1 16\n"
                "\t24c64 i2c_test 1 0x50 2 32\n"
                "\t24c64 i2c_test 1 0x50 2 ioctl\n", argv[0]);
        exit(0);
    }

    /* Get i2c bus number */
    if (sscanf(argv[1], "%u", &bus_num) != 1) {

        fprintf(stderr, "Can't parse i2c 'bus_num' [%s]\n", argv[1]);
        exit(-1);
    }

    /* Get i2c device address */
    if (sscanf(argv[2], "0x%x", &addr) != 1) {

        fprintf(stderr, "Can't parse i2c 'dev_addr' [%s]\n", argv[2]);
        exit(-1);
    }

    /* Get i2c internal address bytes */
    if (sscanf(argv[3], "%u", &iaddr_bytes) != 1) {

        fprintf(stderr, "Can't parse i2c 'iaddr_bytes' [%s]\n", argv[3]);
        exit(-2);
    }

    /* Get i2c page bytes number */
    if (sscanf(argv[4], "%u", &page_bytes) != 1) {

        fprintf(stderr, "Can't parse i2c 'page_bytes' [%s]\n", argv[4]);
        exit(-2);
    }


    /* If specify ioctl using ioctl r/w i2c */
    if (argc == 6 && (memcmp(argv[5], "ioctl", strlen("ioctl")) == 0)) {

        i2c_read_handle = i2c_ioctl_read;
        i2c_write_handle = i2c_ioctl_write;
        fprintf(stdout, "Using i2c_ioctl_oper r/w data\n");
    }
    else {

        fprintf(stdout, "Using i2c_oper r/w data\n");
    }

    /* Open i2c bus */
    int bus;
    char bus_name[32];
    memset(bus_name, 0, sizeof(bus_name));

    if (snprintf(bus_name, sizeof(bus_name), "/dev/i2c-%u", bus_num) < 0) {

        fprintf(stderr, "Format i2c bus name error!\n");
        exit(-3);
    }

    if ((bus = i2c_open(bus_name)) == -1) {

        fprintf(stderr, "Open i2c bus:%s error!\n", bus_name);
        exit(-3);
    }

    /* Init i2c device */
    I2CDevice device;
    memset(&device, 0, sizeof(device));
    i2c_init_device(&device);

    device.bus = bus;
    device.addr = addr & 0x3ff;
    device.page_bytes = page_bytes;
    device.iaddr_bytes = iaddr_bytes;

    /* Print i2c device description */
    fprintf(stdout, "%s\n", i2c_get_device_desc(&device, i2c_dev_desc, sizeof(i2c_dev_desc)));

    size_t i;
    unsigned char buf[256];
    size_t buf_size = sizeof(buf);
    memset(buf, 0, buf_size);

    /* I/O r/w 0x00 - 0xff */
    if (i2c_read_handle == i2c_read) {

        for (i = 0; i < buf_size; i++) {

            buf[i] = i;
        }
    }
    /* ioctl r/w 0xff - 0x0 */
    else {

        for (i = 0; i < buf_size; i++) {

            buf[i] = 0xff - i;
        }
    }

    /* Print before write */
    fprintf(stdout, "Write data:\n");
    print_i2c_data(buf, buf_size);

    if (i2c_write_handle(&device, 0x0, buf, buf_size) != buf_size) {

        fprintf(stderr, "Write i2c error!\n");
        exit(-4);
    }

    fprintf(stdout, "\nWrite success, prepare read....\n");

    /* Read */
    usleep(100000);
    memset(buf, 0, buf_size);

    if (i2c_read_handle(&device, 0x0, buf, buf_size) != buf_size) {

        fprintf(stderr, "Read i2c error!\n");
        exit(-5);
    }

    /* Print read result */
    fprintf(stdout, "Read data:\n");
    print_i2c_data(buf, buf_size);

    i2c_close(bus);
    return 0;
}


