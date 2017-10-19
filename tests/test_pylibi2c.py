import random
import unittest
import pylibi2c


class Pylibi2cTest(unittest.TestCase):
    def setUp(self):
        self.i2c_size = 256
        # 24C04 E2PROM test
        self.i2c = pylibi2c.I2CDevice(bus="/dev/i2c-1", addr=0x56, page_bytes=16)

    def test_init(self):
        with self.assertRaises(TypeError):
            pylibi2c.I2CDevice()

        with self.assertRaises(TypeError):
            pylibi2c.I2CDevice(1, 2)

        with self.assertRaises(TypeError):
            pylibi2c.I2CDevice("1", "2")

        with self.assertRaises(TypeError):
            pylibi2c.I2CDevice("/dev/i2c-1")

        with self.assertRaises(IOError):
            pylibi2c.I2CDevice("/dev/i2c-100", 0x56)

    def test_getattr(self):
        i2c = pylibi2c.I2CDevice("/dev/i2c-1", 0x56)

        with self.assertRaises(AttributeError):
            i2c.bus

        with self.assertRaises(AttributeError):
            i2c.addr

    def test_setattr(self):
        i2c = pylibi2c.I2CDevice("/dev/i2c-1", 0x56)

        with self.assertRaises(AttributeError):
            i2c.bus = ""

        with self.assertRaises(AttributeError):
            i2c.addr = ""

    def test_flags(self):
        i2c = pylibi2c.I2CDevice("/dev/i2c-1", 0x56)
        self.assertEqual(i2c.flags, 0)

        i2c = pylibi2c.I2CDevice("/dev/i2c-1", 0x56, flags=1)
        self.assertEqual(i2c.flags, 1)

        with self.assertRaises(TypeError):
            i2c.flags = "100"

        with self.assertRaises(TypeError):
            i2c.flags = 0.1

        with self.assertRaises(ValueError):
            i2c.flags = -1

        i2c.flags = 0
        self.assertEqual(i2c.flags, 0)

        i2c.flags = pylibi2c.I2C_M_NOSTART
        self.assertEqual(i2c.flags, pylibi2c.I2C_M_NOSTART)

        i2c.flags = pylibi2c.I2C_M_IGNORE_NAK
        self.assertEqual(i2c.flags, pylibi2c.I2C_M_IGNORE_NAK)

    def test_delay(self):
        i2c = pylibi2c.I2CDevice("/dev/i2c-1", 0x56)
        self.assertEqual(i2c.delay, 1)

        i2c = pylibi2c.I2CDevice("/dev/i2c-1", 0x56, delay=0)
        self.assertEqual(i2c.delay, 0)

        with self.assertRaises(TypeError):
            i2c.delay = "100"

        with self.assertRaises(TypeError):
            i2c.delay = 0.1

        with self.assertRaises(ValueError):
            i2c.delay = -1

        with self.assertRaises(ValueError):
            i2c.delay = 101

        i2c.delay = 10
        self.assertEqual(i2c.delay, 10)

        i2c.delay = 100
        self.assertEqual(i2c.delay, 100)

    def test_tenbit(self):
        i2c = pylibi2c.I2CDevice("/dev/i2c-1", 0x56)
        self.assertEqual(i2c.tenbit, False)

        i2c = pylibi2c.I2CDevice("/dev/i2c-1", 0x56, tenbit=1)
        self.assertEqual(i2c.tenbit, True)

        with self.assertRaises(TypeError):
            i2c.tenbit = 0

        with self.assertRaises(TypeError):
            i2c.tenbit = 100

        with self.assertRaises(TypeError):
            i2c.tenbit = "True"

        i2c.tenbit = False
        self.assertEqual(i2c.tenbit, False)

        i2c.tenbit = True
        self.assertEqual(i2c.tenbit, True)

    def test_page_bytes(self):
        i2c = pylibi2c.I2CDevice("/dev/i2c-1", 0x56)
        self.assertEqual(i2c.page_bytes, 8)

        i2c = pylibi2c.I2CDevice("/dev/i2c-1", 0x56, page_bytes=16)
        self.assertEqual(i2c.page_bytes, 16)

        with self.assertRaises(TypeError):
            i2c.page_bytes = "1"

        with self.assertRaises(ValueError):
            i2c.page_bytes = -1

        with self.assertRaises(ValueError):
            i2c.page_bytes = 0

        with self.assertRaises(ValueError):
            i2c.page_bytes = 4

        with self.assertRaises(ValueError):
            i2c.page_bytes = 10

        with self.assertRaises(ValueError):
            i2c.page_bytes = 2048

        i2c.page_bytes = 32
        self.assertEqual(i2c.page_bytes, 32)

        i2c.page_bytes = 64
        self.assertEqual(i2c.page_bytes, 64)

    def test_iaddr_bytes(self):
        i2c = pylibi2c.I2CDevice("/dev/i2c-1", 0x56)
        self.assertEqual(i2c.iaddr_bytes, 1)

        i2c = pylibi2c.I2CDevice("/dev/i2c-1", 0x56, iaddr_bytes=2)
        self.assertEqual(i2c.iaddr_bytes, 2)

        with self.assertRaises(TypeError):
            i2c.iaddr_bytes = "1"

        with self.assertRaises(ValueError):
            i2c.iaddr_bytes = -1

        with self.assertRaises(ValueError):
            i2c.iaddr_bytes = 5

        i2c.iaddr_bytes = 0
        self.assertEqual(i2c.iaddr_bytes, 0)

        i2c.iaddr_bytes = 1
        self.assertEqual(i2c.iaddr_bytes, 1)

        i2c.iaddr_bytes = 3
        self.assertEqual(i2c.iaddr_bytes, 3)

    def test_read(self):
        self.assertEqual(len(self.i2c.read(0, self.i2c_size)), self.i2c_size)
        self.assertEqual(len(self.i2c.read(0, 100)), 100)
        self.assertEqual(len(self.i2c.read(13, 13)), 13)
        self.assertEqual(len(self.i2c.read(13, 1)), 1)

    def test_write(self):

        # 0 - 0xff
        w_buf = bytearray(range(self.i2c_size))
        self.assertEqual(self.i2c.write(0, bytes(w_buf)), self.i2c_size)
        r_buf = self.i2c.read(0, self.i2c_size)
        self.assertEqual(len(r_buf), self.i2c_size)
        self.assertSequenceEqual(w_buf, r_buf)

        # Random data
        w_buf = bytearray(self.i2c_size)
        for i in range(self.i2c_size):
            w_buf[i] = random.randint(0, 255)
        self.assertEqual(self.i2c.write(0, bytes(w_buf)), self.i2c_size)
        r_buf = self.i2c.read(0, self.i2c_size)
        self.assertEqual(len(r_buf), self.i2c_size)
        self.assertSequenceEqual(w_buf, r_buf)

        # Not aligned write
        data = "a212131edada123qdadaeqeqdsadskfljfjfj"
        for addr in range(1, 200, 2):
            self.assertEqual(self.i2c.write(addr, data), len(data))
            self.assertEqual(self.i2c.read(addr, len(data)).decode("ascii"), data)

    def test_ioctl_read(self):
        self.assertEqual(len(self.i2c.ioctl_read(0, self.i2c_size)), self.i2c_size)
        self.assertEqual(len(self.i2c.ioctl_read(0, 100)), 100)
        self.assertEqual(len(self.i2c.ioctl_read(13, 13)), 13)
        self.assertEqual(len(self.i2c.ioctl_read(13, 1)), 1)

    def test_ioctl_ioctl_write(self):
        # 0 - 0xff
        w_buf = bytearray(range(self.i2c_size))
        self.assertEqual(self.i2c.ioctl_write(0, bytes(w_buf)), self.i2c_size)
        r_buf = self.i2c.ioctl_read(0, self.i2c_size)
        self.assertEqual(len(r_buf), self.i2c_size)
        self.assertSequenceEqual(w_buf, r_buf)

        # Random data
        w_buf = bytearray(self.i2c_size)
        for i in range(self.i2c_size):
            w_buf[i] = random.randint(0, 255)
        self.assertEqual(self.i2c.ioctl_write(0, bytes(w_buf)), self.i2c_size)
        r_buf = self.i2c.ioctl_read(0, self.i2c_size)
        self.assertEqual(len(r_buf), self.i2c_size)
        self.assertSequenceEqual(w_buf, r_buf)

        # Not aligned write
        data = "a212131edada123qdadaeqeqdsadskfljfjfj"
        for addr in range(1, 200, 2):
            self.assertEqual(self.i2c.ioctl_write(addr, data), len(data))
            self.assertEqual(self.i2c.ioctl_read(addr, len(data)).decode("ascii"), data)


if __name__ == '__main__':
    unittest.main()
