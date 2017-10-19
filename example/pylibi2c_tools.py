# -*- coding: utf-8 -*-
import argparse
import pylibi2c


if __name__ == '__main__':
    parser = argparse.ArgumentParser()

    # Required args
    parser.add_argument('-b', '--bus', help='i2c bus, such as /dev/i2c-1', type=str, required=True)
    parser.add_argument('-d', '--device', help='i2c device address, such as 0x56', type=str, required=True)

    # Device option args
    parser.add_argument('--iaddr', help='i2c internal address', type=str, default="0x0")
    parser.add_argument('--delay', help='i2c r/w delay, unit is msec', type=int, default=1)
    parser.add_argument('--iaddr_bytes', help='i2c internal address bytes', type=int, default=1)
    parser.add_argument('--page_bytes', help='i2c per page max number of bytes', type=int, default=8)

    # Read/write options
    parser.add_argument('--data', help='write data', type=str)
    parser.add_argument('--size', help='read data size', type=int)
    parser.add_argument('--ioctl', help='using ioctl r/w i2c', type=bool, default=False)
    args = vars(parser.parse_args())

    try:

        bus = args.get('bus')
        device = int(args.get('device'), 16)

        delay = args.get('delay')
        iaddr = int(args.get('iaddr'), 16)
        page_bytes = args.get('page_bytes')
        iaddr_bytes = args.get('iaddr_bytes')

        data = args.get('data')
        size = args.get('size')
        ioctl = args.get('ioctl')

        if data is None and size is None:
            raise RuntimeError("'data' or 'size' must specified one, 'data' for write, 'size' for read")

        # Create a i2c device
        i2c = pylibi2c.I2CDevice(bus=bus, addr=device, page_bytes=page_bytes, iaddr_bytes=iaddr_bytes, delay=delay)

        if data:
            write_handle = i2c.ioctl_write if ioctl else i2c.write
            ret = write_handle(iaddr, bytes(data.encode("ascii")))
            print("Write: '{0:s}' to address: 0x{1:x}".format(data, iaddr))
            print("Result:{}".format(ret))
        else:
            read_handle = i2c.ioctl_read if ioctl else i2c.read
            data = read_handle(iaddr, size)
            print("Read: {0:d} bytes data from address: 0x{1:x}".format(size, iaddr))
            print("Result:'{}'".format(data.decode("ascii")))
    except (TypeError, IOError, ValueError, RuntimeError) as err:
        print("I2C R/W error:{}".format(err))

