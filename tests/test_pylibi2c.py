import six
import random
import ctypes
import unittest
import pylibi2c


class Pylibi2cTest(unittest.TestCase):
    def buf_equal_test(self, buf1, buf2):
        if len(buf1) != len(buf2):
            return False

        for i in range(len(buf1)):
            if buf1[i] != buf2[i]:
                return False

        return True

    def setUp(self):
        self.i2c_size = 256
        self.i2c = pylibi2c.I2CDevice(bus="/dev/i2c-1", addr=0x56)

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

    def test_read(self):
        buf = ctypes.create_string_buffer(self.i2c_size)
        self.assertEqual(self.i2c.read(0, buf, self.i2c_size), self.i2c_size)
        self.assertEqual(self.i2c.read(0, buf, 100), 100)
        self.assertEqual(self.i2c.read(0, buf, self.i2c_size + 1), -1)
        self.assertEqual(self.i2c.read(13, buf, 13), 13)
        self.assertEqual(self.i2c.read(13, buf, 1), 1)

    def test_write(self):
        w_buf = ctypes.create_string_buffer(self.i2c_size)
        for i in range(self.i2c_size):
            w_buf[i] = six.int2byte(i & 0xff)

        self.assertEqual(self.i2c.write(13, w_buf, 13), 13)
        self.assertEqual(self.i2c.write(1, w_buf, 1), 1)

        self.assertEqual(self.i2c.write(0, w_buf, self.i2c_size), self.i2c_size)
        self.assertEqual(self.i2c.write(0, w_buf, self.i2c_size + 1), -1)

        r_buf = ctypes.create_string_buffer(self.i2c_size)
        self.assertEqual(self.i2c.read(0, r_buf, self.i2c_size), self.i2c_size)
        self.assertEqual(self.buf_equal_test(r_buf, w_buf), True)

    def test_ioctl_read(self):
        buf = ctypes.create_string_buffer(self.i2c_size)
        self.assertEqual(self.i2c.ioctl_read(0, buf, self.i2c_size), self.i2c_size)
        self.assertEqual(self.i2c.ioctl_read(0, buf, 100), 100)
        self.assertEqual(self.i2c.ioctl_read(0, buf, self.i2c_size + 1), -1)
        self.assertEqual(self.i2c.ioctl_read(13, buf, 13), 13)
        self.assertEqual(self.i2c.ioctl_read(13, buf, 1), 1)

    def test_ioctl_write(self):
        w_buf = ctypes.create_string_buffer(self.i2c_size)
        for i in range(self.i2c_size):
            w_buf[i] = six.int2byte(random.randint(0, 255))

        self.assertEqual(self.i2c.ioctl_write(13, w_buf, 13), 13)
        self.assertEqual(self.i2c.ioctl_write(1, w_buf, 1), 1)

        self.assertEqual(self.i2c.ioctl_write(0, w_buf, self.i2c_size), self.i2c_size)
        self.assertEqual(self.i2c.ioctl_write(0, w_buf, self.i2c_size + 1), -1)

        r_buf = ctypes.create_string_buffer(self.i2c_size)
        self.assertEqual(self.i2c.read(0, r_buf, self.i2c_size), self.i2c_size)
        self.assertEqual(self.buf_equal_test(r_buf, w_buf), True)


if __name__ == '__main__':
    unittest.main()
