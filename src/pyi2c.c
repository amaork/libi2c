#include <Python.h>
#include "i2c.h"


/* PyArg_ParseTuple format */
#if PY_MAJOR_VERSION > 2
static const char *format = "O!Iy#I";
#else
static const char *format = "O!Iw#I";
#endif


/* I2C r/w operate  */
enum I2CRW{READ, WRITE, IOCTL_READ, IOCTL_WRITE};


/* For test */
static PyObject* add(PyObject *self, PyObject *args)
{
	int x = 0;
	int y = 0;

	if (!PyArg_ParseTuple(args, "ii", &x, &y)) {

		return NULL;
	}

	return Py_BuildValue("i", x + y);
}


/* int bus_init(unsignd char bus_num) */
static PyObject* bus_open(PyObject *self, PyObject *args)
{
	int result = -1;
	unsigned char bus_num = 0;

	if (!PyArg_ParseTuple(args, "B", &bus_num)) {

		fprintf(stderr, "Get arguments error!\n");
		result = -1;
		goto out;
	}

	result = i2c_open(bus_num);

out:
	return Py_BuildValue("i", result);
}


/* void bus_close(int bus_fd) */
static PyObject* bus_close(PyObject *self, PyObject *args)
{
	int bus_fd = 0;

	if (!PyArg_ParseTuple(args, "i", &bus_fd)) {

		fprintf(stderr, "Get arguments error!\n");
	}
	else {

		i2c_close(bus_fd);
	}

	return Py_BuildValue("");
}


/* Convert python dict to stuct i2c_device, success retrun 0, failed -1 */
static int dict_to_i2c_device(PyObject *dict, I2CDevice *device)
{
	PyObject *value = NULL;
	struct dict_item {
		const char *key;
		unsigned int required;
		unsigned int defualt;
		char type;
		void *data;
	};

	if (!dict || !device) {

		fprintf(stderr, "Args is NULL!\n");
		return -1;
	}

	struct dict_item args_items[] = {
		{ "bus", 1, 0, 'I', &device->bus },
		{ "addr", 1, 0, 'S', &device->addr },
		{ "tenbit", 0, 0, 'C', &device->tenbit },
		{ "delay", 0, 5, 'C', &device->delay },
		{ "flags", 0, 0, 'S', &device->flags },
		{ "iaddr_bytes", 0, 1, 'C', &device->iaddr_bytes },
		{ NULL, 0, 0, 0, NULL },
	};

	if (!PyDict_Check(dict)) {

		fprintf(stderr, "Device is not a dict!\n");
		return -1;
	}

	long data = 0;
	struct dict_item *item = args_items;
	for (; item->key; item++) {

		if (!(value = PyDict_GetItemString(dict, item->key)) && item->required) {

			fprintf(stderr, "I2C %s is required!\n", item->key);
			return -1;
		}

		/* If value do not set, using defualt value */
		data = value ? PyLong_AsLong(value) : item->defualt;

		/* Save data to I2CDevice */
		switch (item->type) {

			case 'I' : *(int*)item->data = data;break;
			case 'C' : *(unsigned char*)item->data = data;break;
			case 'S' : *(unsigned short*)item->data = data;break;
			default	 : fprintf(stderr, "Parse %s error!\n", item->key);return -1;
		}
	}

	return 0;
}


/* i2c r/w operate */
static int i2c_rw(PyObject *args, enum I2CRW rw)
{
	unsigned int len = 0;
	unsigned int iaddr = 0;
	unsigned char *buf = NULL;

	Py_ssize_t size = 0;
	PyObject *dict = NULL;

	I2CDevice device;
	memset(&device, 0, sizeof(device));

	I2C_READ_HANDLE read_handle = NULL;
	I2C_WRITE_HANDLE write_handle = NULL;

	/* Get args from python */
	if (!PyArg_ParseTuple(args, format, &PyDict_Type, &dict, &iaddr, &buf, &size, &len)) {

		fprintf(stderr, "Get arguments error!\n");
		return -1;
	}

	/* Convert python dict to I2CDevice */
	if (dict_to_i2c_device(dict, &device) == -1) {

		return -1;
	}

	/* Check buf size */
	if (size < len) {

		fprintf(stderr, "Buffer size error: too small!\n");
		return -1;
	}

	/* Get r/w handle */
	switch (rw) {

		case	READ		:	read_handle = i2c_read;break;
		case	WRITE		:	write_handle = i2c_write;break;
		case	IOCTL_READ	:	read_handle = i2c_ioctl_read;break;
		case	IOCTL_WRITE	:	write_handle = i2c_ioctl_write;break;
	}

	/* Read or write i2c device */
	return read_handle ? read_handle(&device, iaddr, buf, len) : write_handle(&device, iaddr, buf, len);
}


/* file read */
static PyObject* f_read(PyObject *self, PyObject *args)
{
	return Py_BuildValue("i", i2c_rw(args, READ));
}


/* file write */
static PyObject* f_write(PyObject *self, PyObject *args)
{
	return Py_BuildValue("i", i2c_rw(args, WRITE));
}


/* ioctl read */
static PyObject* ioctl_read(PyObject *self, PyObject *args)
{
	return Py_BuildValue("i", i2c_rw(args, IOCTL_READ));
}


/* ioctl write */
static PyObject* ioctl_write(PyObject *self, PyObject *args)
{
	return Py_BuildValue("i", i2c_rw(args, IOCTL_WRITE));
}


/* Module name and doc string */
static const char module_name[] = "pylibi2c";
static const char module_docstring[] = "Linux userspace i2c library";


/* pylibi2c module methods */
static PyMethodDef pylibi2c_methods[] = {

	{ "add", (PyCFunction)add, METH_VARARGS, "int add(x, y) -> x + y" },

	{ "open", (PyCFunction)bus_open, METH_VARARGS, "int bus_open(unsigned char bus_num) -> bus fd or -1" },
	{ "close", (PyCFunction)bus_close, METH_VARARGS, "void bus_close(int bus_fd)"},

	/* file r/w */
	{ "read", (PyCFunction)f_read, METH_VARARGS, "int read(device_dict, iaddr, buf, len) -> read length or -1" },
	{ "write", (PyCFunction)f_write, METH_VARARGS, "int write(device_dict, iaddr, buf, len) -> write lenght or -1" },

	/* ioctl r/w */
	{ "ioctl_read", (PyCFunction)ioctl_read, METH_VARARGS, "int ioctl_read(device_dict, iaddr, buf, len) -> read length or -1" },
	{ "ioctl_write", (PyCFunction)ioctl_write, METH_VARARGS, "int ioctl_write(device_dict, iaddr, buf, len) -> write lenght or -1" },

	{ NULL, NULL, 0, NULL },
};


#if PY_MAJOR_VERSION > 2
static struct PyModuleDef pylibi2cmodule = {
	PyModuleDef_HEAD_INIT,
       	module_name,		/* Module name */
	module_docstring,	/* Module pylibi2cMethods */
       	-1,			/* size of per-interpreter state of the module, size of per-interpreter state of the module,*/
	pylibi2c_methods,
};
#endif


#if PY_MAJOR_VERSION > 2
PyMODINIT_FUNC PyInit_pylibi2c(void)
#else
PyMODINIT_FUNC initpylibi2c(void)
#endif
{
#if PY_MAJOR_VERSION > 2
	return PyModule_Create(&pylibi2cmodule);
#else
	Py_InitModule3(module_name, pylibi2c_methods, module_docstring);
#endif
}

