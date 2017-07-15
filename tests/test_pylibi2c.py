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
        bus = pylibi2c.open(1)
        self.assertGreaterEqual(bus, 0)
        self.i2c_size = 256
        self.device = {'bus': bus, 'addr': 0x56}

    def test_read(self):
        buf = ctypes.create_string_buffer(self.i2c_size)
        self.assertEqual(pylibi2c.read(self.device, 0, buf, self.i2c_size), self.i2c_size)
        self.assertEqual(pylibi2c.read(self.device, 0, buf, 100), 100)
        self.assertEqual(pylibi2c.read(self.device, 0, buf, self.i2c_size + 1), -1)
        self.assertEqual(pylibi2c.read(self.device, 13, buf, 13), 13)
        self.assertEqual(pylibi2c.read(self.device, 13, buf, 1), 1)

    def test_write(self):
        w_buf = ctypes.create_string_buffer(self.i2c_size)
        for i in range(self.i2c_size):
            w_buf[i] = six.int2byte(i & 0xff)

        self.assertEqual(pylibi2c.write(self.device, 13, w_buf, 13), 13)
        self.assertEqual(pylibi2c.write(self.device, 1, w_buf, 1), 1)

        self.assertEqual(pylibi2c.write(self.device, 0, w_buf, self.i2c_size), self.i2c_size)
        self.assertEqual(pylibi2c.write(self.device, 0, w_buf, self.i2c_size + 1), -1)

        r_buf = ctypes.create_string_buffer(self.i2c_size)
        self.assertEqual(pylibi2c.read(self.device, 0, r_buf, self.i2c_size), self.i2c_size)
        self.assertEqual(self.buf_equal_test(r_buf, w_buf), True)

    def test_ioctl_read(self):
        buf = ctypes.create_string_buffer(self.i2c_size)
        self.assertEqual(pylibi2c.ioctl_read(self.device, 0, buf, self.i2c_size), self.i2c_size)
        self.assertEqual(pylibi2c.ioctl_read(self.device, 0, buf, 100), 100)
        self.assertEqual(pylibi2c.ioctl_read(self.device, 0, buf, self.i2c_size + 1), -1)
        self.assertEqual(pylibi2c.ioctl_read(self.device, 13, buf, 13), 13)
        self.assertEqual(pylibi2c.ioctl_read(self.device, 13, buf, 1), 1)

    def test_ioctl_write(self):
        w_buf = ctypes.create_string_buffer(self.i2c_size)
        for i in range(self.i2c_size):
            w_buf[i] = six.int2byte(random.randint(0, 255))

        self.assertEqual(pylibi2c.ioctl_write(self.device, 13, w_buf, 13), 13)
        self.assertEqual(pylibi2c.ioctl_write(self.device, 1, w_buf, 1), 1)

        self.assertEqual(pylibi2c.ioctl_write(self.device, 0, w_buf, self.i2c_size), self.i2c_size)
        self.assertEqual(pylibi2c.ioctl_write(self.device, 0, w_buf, self.i2c_size + 1), -1)

        r_buf = ctypes.create_string_buffer(self.i2c_size)
        self.assertEqual(pylibi2c.read(self.device, 0, r_buf, self.i2c_size), self.i2c_size)
        self.assertEqual(self.buf_equal_test(r_buf, w_buf), True)


if __name__ == '__main__':
    unittest.main()
