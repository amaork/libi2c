#include <Python.h>
#include "i2c.h"


#define _VERSION_ "0.1"
#define _NAME_ "pylibi2c"
PyDoc_STRVAR(I2CDevice_name, "I2CDevice");
PyDoc_STRVAR(pylibi2c_doc, "Linux userspace i2c library.\n");


/* PyArg_ParseTuple format */
#if PY_MAJOR_VERSION >= 3
static const char *format = "Iy#I";
#else
static const char *format = "Iw#I";
#endif


/* I2C r/w operate  */
enum I2CRW{READ, WRITE, IOCTL_READ, IOCTL_WRITE};


PyDoc_STRVAR(I2CDeviceObject_type_doc, "I2CDevice(bus, address, tenbit=0, iaddr_bytes=1, delay=5, flags=0) -> I2C Device object.\n");
typedef struct {
    PyObject_HEAD;
    I2CDevice dev;
} I2CDeviceObject;


static PyObject *I2CDevice_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {

    I2CDeviceObject *self;
    if ((self = (I2CDeviceObject *)type->tp_alloc(type, 0)) == NULL) {

        return NULL;
    }

    memset(&self->dev, 0, sizeof(self->dev));
    self->dev.bus = -1;

    /* 7 bits */
    self->dev.tenbit = 0;

    /* 5ms */
    self->dev.delay = 5;

    /* 1 byte internal address */
    self->dev.iaddr_bytes = 1;

    Py_INCREF(self);
    return (PyObject *)self;
}


PyDoc_STRVAR(I2CDevice_close_doc, "close()\n\nClose i2c device.\n");
static PyObject *I2CDevice_close(I2CDeviceObject *self) {

    /* Close i2c bus */
    if (self->dev.bus >= 0) {

        i2c_close(self->dev.bus);
    }

    Py_INCREF(Py_None);
    return Py_None;
}


static void I2CDevice_free(I2CDeviceObject *self) {

    PyObject *ref = I2CDevice_close(self);
    Py_XDECREF(ref);

    Py_TYPE(self)->tp_free((PyObject *)self);
}


/* I2CDevice(bus, addr, tenbit=0, iaddr_bytes=1, delay=5, flags=0) */
static int I2CDevice_init(I2CDeviceObject *self, PyObject *args, PyObject *kwds) {

	char *bus_name = NULL;
    static char *kwlist[] = {"bus", "addr", "tenbit", "iaddr_bytes", "delay", "flags", NULL};

    /* Bus name and device address is required */
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "sH|BBBH:__init__", kwlist,
                &bus_name, &self->dev.addr, &self->dev.tenbit, &self->dev.iaddr_bytes, &self->dev.delay, &self->dev.flags)) {

        return -1;
	}

    /* Open i2c bus */
	if ((self->dev.bus = i2c_open(bus_name)) == -1) {
        PyErr_SetFromErrno(PyExc_IOError);
        return -1;
    }

    return 0;
}


static PyObject *I2CDevice_enter(PyObject *self, PyObject *args) {

    if (!PyArg_ParseTuple(args, ""))
        return NULL;

    Py_INCREF(self);
    return self;
}


static PyObject *I2CDevice_exit(I2CDeviceObject *self, PyObject *args) {

    PyObject *exc_type = 0;
    PyObject *exc_value = 0;
    PyObject *traceback = 0;

    if (!PyArg_UnpackTuple(args, "__exit__", 3, 3, &exc_type, &exc_value, &traceback)) {

        return 0;
    }

    /* Close i2c bus */
    I2CDevice_close(self);
    Py_RETURN_FALSE;
}


/* i2c r/w operate */
static int i2c_rw(I2CDeviceObject *self, PyObject *args, enum I2CRW rw) {

	I2CDevice device;
	Py_ssize_t size = 0;
	unsigned int len = 0;
	unsigned int iaddr = 0;
	unsigned char *buf = NULL;
	I2C_READ_HANDLE read_handle = NULL;
	I2C_WRITE_HANDLE write_handle = NULL;

	/* Get args from python */
	if (!PyArg_ParseTuple(args, format, &iaddr, &buf, &size, &len)) {

		fprintf(stderr, "Get arguments error!\n");
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

    /* Copy data to device */
    device = self->dev;

	/* Read or write i2c device */
	return read_handle ? read_handle(&device, iaddr, buf, len) : write_handle(&device, iaddr, buf, len);
}


/* file read */
PyDoc_STRVAR(I2CDevice_read_doc, "read(iaddr, buf, size)\n\nRead #size bytes data from device #iaddress to #buf.\n");
static PyObject* I2CDevice_read(I2CDeviceObject *self, PyObject *args) {

	return Py_BuildValue("i", i2c_rw(self, args, READ));
}


/* file write */
PyDoc_STRVAR(I2CDevice_write_doc, "write(iaddr, buf, size)\n\nWrite #size bytes data from #buf to device #iaddress.\n");
static PyObject* I2CDevice_write(I2CDeviceObject *self, PyObject *args) {

	return Py_BuildValue("i", i2c_rw(self, args, WRITE));
}


/* ioctl read */
PyDoc_STRVAR(I2CDevice_ioctl_read_doc, "ioctl_read(iaddr, buf, size)\n\nIoctl read #size bytes data from device #iaddress to #buf.\n");
static PyObject* I2CDevice_ioctl_read(I2CDeviceObject *self, PyObject *args) {

	return Py_BuildValue("i", i2c_rw(self, args, IOCTL_READ));
}


/* ioctl write */
PyDoc_STRVAR(I2CDevice_ioctl_write_doc, "ioctl_write(iaddr, buf, size)\n\nIoctl write #size bytes data from #buf to device #iaddress.\n");
static PyObject* I2CDevice_ioctl_write(I2CDeviceObject *self, PyObject *args) {

	return Py_BuildValue("i", i2c_rw(self, args, IOCTL_WRITE));
}


/* pylibi2c module methods */
static PyMethodDef I2CDevice_methods[] = {

    {"read", (PyCFunction)I2CDevice_read, METH_VARARGS, I2CDevice_read_doc},
    {"write", (PyCFunction)I2CDevice_write, METH_VARARGS, I2CDevice_write_doc},
    {"close", (PyCFunction)I2CDevice_close, METH_NOARGS, I2CDevice_close_doc},
    {"ioctl_read", (PyCFunction)I2CDevice_ioctl_read, METH_VARARGS, I2CDevice_ioctl_read_doc},
    {"ioctl_write", (PyCFunction)I2CDevice_ioctl_write, METH_VARARGS, I2CDevice_ioctl_write_doc},
    {"__enter__", (PyCFunction)I2CDevice_enter, METH_NOARGS, NULL},
    {"__exit__", (PyCFunction)I2CDevice_exit, METH_NOARGS, NULL},
	{NULL},
};


static PyTypeObject I2CDeviceObjectType = {
#if PY_MAJOR_VERSION >= 3
    PyVarObject_HEAD_INIT(NULL, 0)
#else
    PyObject_HEAD_INIT(NULL)
    0,				            /* ob_size */
#endif
    I2CDevice_name,		        /* tp_name */
    sizeof(I2CDeviceObject),	/* tp_basicsize */
    0,			        	    /* tp_itemsize */
    (destructor)I2CDevice_free,/* tp_dealloc */
    0,				            /* tp_print */
    0,				            /* tp_getattr */
    0,				            /* tp_setattr */
    0,				            /* tp_compare */
    0,				            /* tp_repr */
    0,				            /* tp_as_number */
    0,				            /* tp_as_sequence */
    0,				            /* tp_as_mapping */
    0,				            /* tp_hash */
    0,				            /* tp_call */
    0,				            /* tp_str */
    0,				            /* tp_getattro */
    0,				            /* tp_setattro */
    0,				            /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /* tp_flags */
    I2CDeviceObject_type_doc,	/* tp_doc */
    0,				            /* tp_traverse */
    0,				            /* tp_clear */
    0,				            /* tp_richcompare */
    0,				            /* tp_weaklistoffset */
    0,				            /* tp_iter */
    0,				            /* tp_iternext */
    I2CDevice_methods,		    /* tp_methods */
    0,				            /* tp_members */
    0,		                    /* tp_getset */
    0,				            /* tp_base */
    0,				            /* tp_dict */
    0,				            /* tp_descr_get */
    0,				            /* tp_descr_set */
    0,				            /* tp_dictoffset */
    (initproc)I2CDevice_init,	/* tp_init */
    0,				            /* tp_alloc */
    I2CDevice_new,		        /* tp_new */
};


static PyMethodDef pylibi2c_methods[] = {

    {NULL}
};


#if PY_MAJOR_VERSION >= 3
static struct PyModuleDef pylibi2cmodule = {
	PyModuleDef_HEAD_INIT,
    _NAME_,         /* Module name */
	pylibi2c_doc,	/* Module pylibi2cMethods */
    -1,			    /* size of per-interpreter state of the module, size of per-interpreter state of the module,*/
	pylibi2c_methods,
};
#endif


#if PY_MAJOR_VERSION >= 3
PyMODINIT_FUNC PyInit_pylibi2c(void)
#else
PyMODINIT_FUNC initpylibi2c(void)
#endif
{

    PyObject *module;

    if (PyType_Ready(&I2CDeviceObjectType) < 0) {
#if PY_MAJOR_VERSION >= 3
        return NULL;
#else
        return;
#endif
    }

#if PY_MAJOR_VERSION >= 3
    module = PyModule_Create(&pylibi2cmodule);
    PyObject *version = PyUnicode_FromString(_VERSION_);
#else
	module = Py_InitModule3(_NAME_, pylibi2c_methods, pylibi2c_doc);
    PyObject *version = PyString_FromString(_VERSION_);
#endif

    /* Set module version */
    PyObject *dict = PyModule_GetDict(module);
    PyDict_SetItemString(dict, "__version__", version);
    Py_DECREF(version);

    /* Register I2CDeviceObject */
    Py_INCREF(&I2CDeviceObjectType);
    PyModule_AddObject(module, I2CDevice_name, (PyObject *)&I2CDeviceObjectType);

#if PY_MAJOR_VERSION >= 3
    return module;
#endif
}

