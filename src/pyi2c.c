#include <Python.h>
#include "i2c.h"

/* For test */
static PyObject* add(PyObject* self, PyObject* args)
{
	int x = 0;
	int y = 0;

	if (!PyArg_ParseTuple(args, "ii", &x, &y)) {

		return NULL;
	}

	return Py_BuildValue("i", x + y);
}


/* int init(const char* bus_name, unsigned int delay) */
static PyObject* init(PyObject* self, PyObject* args)
{
	int result = -1;
	char *bus_name = NULL;
	unsigned int msec_delay = 0;

	if (!PyArg_ParseTuple(args, "t#I", &bus_name, &msec_delay)) {

		result = -1;
		fprintf(stderr, "Get arguments error!\n");
		goto out;
	}

	result = i2c_init(bus_name, msec_delay);

out:
	return Py_BuildValue("i", result);
}


/* file read */
static PyObject* f_read(PyObject* self, PyObject* args)
{
	I2C_MSG msg;
	int result = -1;
	memset(&msg, 0, sizeof(msg));

	if (!PyArg_ParseTuple(args, "bIIwI", &msg.dev_addr, &msg.int_addr, &msg.iaddr_bytes, &msg.buf, &msg.len)) {

		result = -1;
		goto out;
	}

	msg.oper = I2C_LOAD;
	result = i2c_oper(&msg);

out:
	return Py_BuildValue("i", result);
}


/* file write */
static PyObject* f_write(PyObject* self, PyObject* args)
{
	I2C_MSG msg;
	int result = -1;
	memset(&msg, 0, sizeof(msg));

	if (!PyArg_ParseTuple(args, "bIIwI", &msg.dev_addr, &msg.int_addr, &msg.iaddr_bytes, &msg.buf, &msg.len)) {

		result = -1;
		goto out;
	}

	msg.oper = I2C_STORE;
	result = i2c_oper(&msg);

out:
	return Py_BuildValue("i", result);
}


/* ioctl read */
static PyObject* ioctl_read(PyObject* self, PyObject* args)
{
	I2C_MSG msg;
	int result = -1;
	memset(&msg, 0, sizeof(msg));

	if (!PyArg_ParseTuple(args, "bIIwIh", &msg.dev_addr, &msg.int_addr, &msg.iaddr_bytes, &msg.buf, &msg.len, &msg.flags)) {

		result = -1;
		goto out;
	}

	msg.oper = I2C_LOAD;
	result = i2c_oper(&msg);

out:
	return Py_BuildValue("i", result);
}


/* ioctl write */
static PyObject* ioctl_write(PyObject* self, PyObject* args)
{
	I2C_MSG msg;
	int result = -1;
	memset(&msg, 0, sizeof(msg));

	if (!PyArg_ParseTuple(args, "bIIwIh", &msg.dev_addr, &msg.int_addr, &msg.iaddr_bytes, &msg.buf, &msg.len, &msg.flags)) {

		result = -1;
		goto out;
	}

	msg.oper = I2C_STORE;
	result = i2c_oper(&msg);

out:
	return Py_BuildValue("i", result);
}


/* pylibi2c module methods */
static PyMethodDef pylibi2cMethods[] = {

	{ "add", (PyCFunction)add, METH_VARARGS, "int add(x, y) -> x + y" },

	{ "open", (PyCFunction)init, METH_VARARGS, "int init(const char* bus_name, unsigned init delay)" },

	/* i2c_oper r/w */
	{ "read", (PyCFunction)f_read, METH_VARARGS, "int read(dev_addr, int_addr, iaddr_bytes, buf, len)" },
	{ "write", (PyCFunction)f_write, METH_VARARGS, "int write(dev_addr, int_addr, iaddr_bytes, buf, len)" },

	/* i2c_ioctl_oper r/w */
	{ "ioctl_read", (PyCFunction)ioctl_read, METH_VARARGS, "int ioctl_read(dev_addr, int_addr, iaddr_bytes, buf, len, flags)" },
	{ "ioctl_write", (PyCFunction)ioctl_write, METH_VARARGS, "int ioctl_write(dev_addr, int_addr, iaddr_bytes, buf, len, flags)" },


	{ NULL, NULL, 0, NULL },
};


PyMODINIT_FUNC initpylibi2c(void)
{
	Py_InitModule("pylibi2c", pylibi2cMethods);
}

