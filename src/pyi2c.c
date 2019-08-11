#include <Python.h>
#include "i2c.h"


#define _VERSION_ "0.1"
#define _NAME_ "pylibi2c"
#define _I2CDEV_MAX_SIZE_ 4096
#define _I2CDEV_MAX_IADDR_BYTES_SIZE 4
#define _I2CDEV_MAX_PAGE_BYTES_SIZE 1024
PyDoc_STRVAR(I2CDevice_name, "I2CDevice");
PyDoc_STRVAR(pylibi2c_doc, "Linux userspace i2c library.\n");


/* Macros needed for Python 3 */
#ifndef PyInt_Check
#define PyInt_Check      PyLong_Check
#define PyInt_FromLong   PyLong_FromLong
#define PyInt_AsLong     PyLong_AsLong
#define PyInt_Type       PyLong_Type
#endif


PyDoc_STRVAR(I2CDeviceObject_type_doc, "I2CDevice(bus, address, tenbit=False, iaddr_bytes=1, page_bytes=8, delay=1, flags=0) -> I2CDevice object.\n");
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
    i2c_init_device(&self->dev);

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


/* I2CDevice(bus, addr, tenbit=0, iaddr_bytes=1, page_bytes=8, delay=1, flags=0) */
static int I2CDevice_init(I2CDeviceObject *self, PyObject *args, PyObject *kwds) {

    char *bus_name = NULL;
    static char *kwlist[] = {"bus", "addr", "tenbit", "iaddr_bytes", "page_bytes", "delay", "flags", NULL};

    /* Bus name and device address is required */
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "sH|BBHBH:__init__", kwlist,
                                     &bus_name, &self->dev.addr,
                                     &self->dev.tenbit, &self->dev.iaddr_bytes, &self->dev.page_bytes, &self->dev.delay, &self->dev.flags)) {

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


/* str */
static PyObject *I2CDevice_str(PyObject *object) {

    char desc[128];
    PyObject *dev_desc = NULL;
    I2CDeviceObject *self = (I2CDeviceObject *)object;
    i2c_get_device_desc(&self->dev, desc, sizeof(desc));

#if PY_MAJOR_VERSION >= 3
    dev_desc = PyUnicode_FromString(desc);
#else
    dev_desc = PyString_FromString(desc);
#endif

    Py_INCREF(dev_desc);
    return dev_desc;
}


/* i2c read device */
static PyObject *i2c_read_device(I2CDeviceObject *self, PyObject *args, int ioctl) {

    int result;
    unsigned int len = 0;
    unsigned int iaddr = 0;
    PyObject *bytearray = NULL;
    char buf[_I2CDEV_MAX_SIZE_];
    memset(buf, 0, sizeof(buf));
    I2C_READ_HANDLE read_handle = NULL;

    if (!PyArg_ParseTuple(args, "II:read", &iaddr, &len)) {

        return NULL;
    }

    len = len > sizeof(buf) ? sizeof(buf) : len;
    read_handle = ioctl ? i2c_ioctl_read : i2c_read;
    result = read_handle(&self->dev, iaddr, buf, len);

    if (result < 0) {
        PyErr_SetFromErrno(PyExc_IOError);
        return NULL;
    }

    if (result != len) {
        perror("short read");
        return NULL;
    }

    /* Copy data to bytearray and return */
    bytearray = PyByteArray_FromStringAndSize(buf, len);
    Py_INCREF(bytearray);
    return bytearray;
}


/* i2c write device */
static PyObject *i2c_write_device(I2CDeviceObject *self, PyObject *args, int ioctl) {

    char *buf = NULL;
    Py_ssize_t size = 0;
    unsigned int iaddr = 0;
    PyObject *result = NULL;
    I2C_WRITE_HANDLE write_handle = NULL;

    if (!PyArg_ParseTuple(args, "Is#::write", &iaddr, &buf, &size)) {
        return NULL;
    }

    write_handle = ioctl ? i2c_ioctl_write : i2c_write;
    result = Py_BuildValue("i", write_handle(&self->dev, iaddr, buf, size));

    Py_INCREF(result);
    return result;
}


/* file read */
PyDoc_STRVAR(I2CDevice_read_doc, "read(iaddr, buf, size)\n\nRead #size bytes data from device #iaddress to #buf.\n");
static PyObject *I2CDevice_read(I2CDeviceObject *self, PyObject *args) {

    return i2c_read_device(self, args, 0);
}


/* file write */
PyDoc_STRVAR(I2CDevice_write_doc, "write(iaddr, buf, size)\n\nWrite #size bytes data from #buf to device #iaddress.\n");
static PyObject *I2CDevice_write(I2CDeviceObject *self, PyObject *args) {

    return i2c_write_device(self, args, 0);
}


/* ioctl read */
PyDoc_STRVAR(I2CDevice_ioctl_read_doc, "ioctl_read(iaddr, buf, size)\n\nIoctl read #size bytes data from device #iaddress to #buf.\n");
static PyObject *I2CDevice_ioctl_read(I2CDeviceObject *self, PyObject *args) {

    return i2c_read_device(self, args, 1);
}


/* ioctl write */
PyDoc_STRVAR(I2CDevice_ioctl_write_doc, "ioctl_write(iaddr, buf, size)\n\nIoctl write #size bytes data from #buf to device #iaddress.\n");
static PyObject *I2CDevice_ioctl_write(I2CDeviceObject *self, PyObject *args) {

    return i2c_write_device(self, args, 1);
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


/* Check */
static int check_user_input(const char *name, PyObject *input, int min, int max) {

    int value;

    if (input == NULL) {
        PyErr_SetString(PyExc_TypeError, "Cannot delete the last attribute");
        return -1;
    }

    /* tenbit attribute, boolean type */
    if (memcmp(name, "tenbit", strlen(name)) == 0) {
        if (!PyBool_Check(input)) {
            PyErr_SetString(PyExc_TypeError, "The last attribute value must be boolean");
            return -1;
        }

        return 0;
    }

    /* Other attribute, integer type */
    if (!PyInt_Check(input)) {
        PyErr_SetString(PyExc_TypeError, "The last attribute value must be an integer");
        return -1;
    }

    /* Convert value to int */
    value = PyLong_AsLong(input);

    /* Check input value range */
    if (value < min || value > max) {
        PyErr_Format(PyExc_ValueError, "invalid ' %s'(%d - %d)", name, min, max);
        return -1;
    }

    return 0;
}

/* flags */
PyDoc_STRVAR(I2CDevice_flags_doc, "ioctl_read/write i2c_msg flags\n\n"
             "I_2C_M_NOSTART\n"
             "In a combined transaction, no 'S Addr Wr/Rd [A]' is generated at some point."
             "For example, setting I2C_M_NOSTART on the second partial message generates something like:\n\n"
             "\t\t\tS Addr Rd [A] [Data] NA Data [A] P\n\n"
             "If you set the I2C_M_NOSTART variable for the first partial message, we do not generate Addr,"
             "but we do generate the startbit S. This will probably confuse all other clients on your bus, so don't try this.\n\n"
             "I2C_M_IGNORE_NAK\n"
             "Normally message is interrupted immediately if there is [NA] from the"
             "client. Setting this flag treats any [NA] as [A], and all of message is sent.These messages may still fail to SCL lo->hi timeout.\n\n"
             "I2C_M_NO_RD_ACK\n"
             "In a read message, master A/NA bit is skipped.\n\n");
static PyObject *I2CDevice_get_flags(I2CDeviceObject *self, void *closure) {

    PyObject *result = Py_BuildValue("H", self->dev.flags);
    Py_INCREF(result);
    return result;
}

static int I2CDevice_set_flags(I2CDeviceObject *self, PyObject *value, void *closeure) {

    if (check_user_input("falgs", value, 0, I2C_M_NOSTART) != 0) {

        return -1;
    }

    self->dev.flags = PyLong_AsLong(value);
    return 0;
}


/* delay */
PyDoc_STRVAR(I2CDevice_delay_doc, "i2c internal operate delay, unit milliscond.\n\n");
static PyObject *I2CDevice_get_delay(I2CDeviceObject *self, void *closure) {

    PyObject *result = Py_BuildValue("b", self->dev.delay);
    Py_INCREF(result);
    return result;
}

static int I2CDevice_set_delay(I2CDeviceObject *self, PyObject *value, void *closeure) {

    if (check_user_input("delay", value, 0, 100) != 0) {

        return -1;
    }

    self->dev.delay = PyLong_AsLong(value);
    return 0;
}


/* tenbit */
PyDoc_STRVAR(I2CDevice_tenbit_doc, "True, Enable 10 bit addressing.\n\nFalse, 7 bit addressing(default).\n");
static PyObject *I2CDevice_get_tenbit(I2CDeviceObject *self, void *closure) {

    PyObject *result = self->dev.tenbit ? Py_True : Py_False;
    Py_INCREF(result);
    return result;
}

static int I2CDevice_set_tenbit(I2CDeviceObject *self, PyObject *value, void *closeure) {

    if (check_user_input("tenbit", value, 0, 1) != 0) {

        return -1;
    }

    self->dev.tenbit = PyLong_AsLong(value);
    return 0;
}


/* iaddr_bytes */
PyDoc_STRVAR(I2CDevice_page_bytes_doc, "i2c EEPROM max number of bytes per page (must be divisible by 8)\n\n"
             "8, 8 bytes per page, such as 24C01/24C02\n\n"
             "16, 16 bytes per page, such as 24C04/24C08/24C16\n\n"
             "32, 32 bytes per page, such as 24C32/24C64\n\n");
static PyObject *I2CDevice_get_page_bytes(I2CDeviceObject *self, void *closure) {

    PyObject *result = Py_BuildValue("I", self->dev.page_bytes);
    Py_INCREF(result);
    return result;
}

static int I2CDevice_set_page_bytes(I2CDeviceObject *self, PyObject *value, void *closeure) {

    int page_bytes = 0;

    if (check_user_input("page_bytes", value, 8, _I2CDEV_MAX_PAGE_BYTES_SIZE) != 0) {

        return -1;
    }

    page_bytes = PyLong_AsLong(value);

    /* Divisible by 8 */
    if (page_bytes % 8) {
        PyErr_SetString(PyExc_ValueError, "The 'page_bytes' must be divisible by 8");
        return -1;
    }

    self->dev.page_bytes = page_bytes;
    return 0;
}


/* iaddr_bytes */
PyDoc_STRVAR(I2CDevice_iaddr_bytes_doc, "I2C device internal(word) address bytes.\n\n"
             "0, special device, without internal address\n\n"
             "1, 1 byte internal address, such as 24C04\n\n"
             "2, 2 byte internal address, such as 24C64\n\n"
             "2, 3 byte internal address, such as 24C1024\n\n");
static PyObject *I2CDevice_get_iaddr_bytes(I2CDeviceObject *self, void *closure) {

    PyObject *result = Py_BuildValue("b", self->dev.iaddr_bytes);
    Py_INCREF(result);
    return result;
}

static int I2CDevice_set_iaddr_bytes(I2CDeviceObject *self, PyObject *value, void *closeure) {

    if (check_user_input("iaddr_bytes", value, 0, _I2CDEV_MAX_IADDR_BYTES_SIZE) != 0) {

        return -1;
    }

    self->dev.iaddr_bytes = PyLong_AsLong(value);
    return 0;
}


static PyGetSetDef I2CDevice_getseters[] = {

    {"flags", (getter)I2CDevice_get_flags, (setter)I2CDevice_set_flags, I2CDevice_flags_doc},
    {"delay", (getter)I2CDevice_get_delay, (setter)I2CDevice_set_delay, I2CDevice_delay_doc},
    {"tenbit", (getter)I2CDevice_get_tenbit, (setter)I2CDevice_set_tenbit, I2CDevice_tenbit_doc},
    {"page_bytes", (getter)I2CDevice_get_page_bytes, (setter)I2CDevice_set_page_bytes, I2CDevice_page_bytes_doc},
    {"iaddr_bytes", (getter)I2CDevice_get_iaddr_bytes, (setter)I2CDevice_set_iaddr_bytes, I2CDevice_iaddr_bytes_doc},
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
    I2CDevice_str,	            /* tp_str */
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
    I2CDevice_getseters,        /* tp_getset */
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


void define_constants(PyObject *module) {

    PyModule_AddObject(module, "I2C_M_NOSTART", Py_BuildValue("H", I2C_M_NOSTART));
    PyModule_AddObject(module, "I2C_M_NO_RD_ACK", Py_BuildValue("H", I2C_M_NO_RD_ACK));
    PyModule_AddObject(module, "I2C_M_IGNORE_NAK", Py_BuildValue("H", I2C_M_IGNORE_NAK));
}


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

    /* Constants */
    define_constants(module);

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

