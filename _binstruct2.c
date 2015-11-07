/* ------------------------------------------------------------------------------
   binstruct module implementation

   Thomas Bonim (thomas.bonim@googlemail.com)
   This code is in the public domain
------------------------------------------------------------------------------- */
#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <stddef.h>
#include <string.h>

#ifdef _MSC_VER
#include <stdlib.h>
typedef signed __int8  int8_t;
typedef signed __int16 int16_t;
typedef signed __int32 int32_t;
typedef signed __int64 int64_t;
typedef unsigned __int8  uint8_t;
typedef unsigned __int16 uint16_t;
typedef unsigned __int32 uint32_t;
typedef unsigned __int64 uint64_t;
#define U64(u) (u##ui64)
#define byte_swap_u16   _byteswap_ushort
#define byte_swap_u32   _byteswap_ulong
#define byte_swap_u64   _byteswap_uint64
#define inline __inline
#else
#include <stdint.h>
#define byte_swap_u16   __builtin_bswap16
#define byte_swap_u32   __builtin_bswap32
#define byte_swap_u64   __builtin_bswap64
#define U64(u) (u##ULL)
#endif

#define UNUSED(obj) (void)(obj)

typedef struct {
    PyObject_HEAD
    PyObject   *o_data;
    Py_ssize_t  s_len;
    Py_ssize_t  s_pos;
    PyObject   *weakrefs;
} BinstructObject;


/* local exception */
static PyObject *BinstructError;

/* common code */
static void range_error(Py_ssize_t width)
{
    PyErr_Format(BinstructError, "unpack requires data of length %zd", width);
}

static inline int range_check(BinstructObject *self, Py_ssize_t width)
{
    if ((self->s_len - self->s_pos) >= width)
        return 1;
    else {
        range_error(width);
        return 0;
    }
}

static inline int range_check_and_copy(BinstructObject *self, void *dest, Py_ssize_t width)
{
    if ((self->s_len - self->s_pos) >= width) {
        memcpy(dest, PyString_AS_STRING(self->o_data) + self->s_pos, width);
        self->s_pos += width;
        return 1;
    }
    else {
        range_error(width);
        return 0;
    }
}

static inline PyObject *to_unsigned(unsigned long value)
{
    if (value > LONG_MAX)
        return PyLong_FromUnsignedLong(value);
    else
        return PyInt_FromLong(value);
}

static inline PyObject *to_signed(unsigned long value, unsigned width)
{
    if (value & (1UL << (width * 8 - 1)))
        return PyInt_FromLong(value | ((-1UL) << (width * 8)));
    else
        return PyInt_FromLong(value);
}

static inline PyObject *unpack_block(BinstructObject *self, Py_ssize_t len)
{
    const char *p;

    if (!range_check(self, len))
        return NULL;

    p = PyString_AS_STRING(self->o_data) + self->s_pos;
    self->s_pos += len;
    return PyString_FromStringAndSize(p, len);
}

static inline int unpack_uleb128(BinstructObject *self, uint64_t *value)
{
    Py_ssize_t pos;
    unsigned shift = 0;
    const char *p;
    uint8_t b;

    pos = self->s_pos;
    p = PyString_AS_STRING(self->o_data);
    while (self->s_pos < self->s_len) {
        b = (uint8_t)p[self->s_pos++];
        *value |= (b & 0x7f) << shift;
        if (b >= 128)
            shift += 7;
        else
            return 1;
    }

    self->s_pos = pos;
    PyErr_SetString(BinstructError, "unpack requires more data for LEB128");
    return 0;
}

/* default methods */
static PyObject *s_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    PyObject *self;
    BinstructObject *obj;

    UNUSED(args);
    UNUSED(kwds);
    self = type->tp_alloc(type, 0);
    if (self != NULL) {
        obj = (BinstructObject*)self;
        Py_INCREF(Py_None);
        obj->o_data = Py_None;
        obj->s_len = -1;
        obj->s_pos = -1;
    }

    return self;
}

static int s_init(BinstructObject *self, PyObject *args, PyObject *kwds)
{
    PyObject *o_data;
    static char *kwlist[] = {"data", NULL};

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "S:Binstruct", kwlist, &o_data))
        return -1;

    Py_INCREF(o_data);
    Py_CLEAR(self->o_data);
    self->o_data = o_data;
    self->s_len = PyString_Size(o_data);
    self->s_pos = 0;

    return 0;
}

static PyObject *s_unpack_u8(BinstructObject *self, PyObject *args)
{
    const char *p;

    UNUSED(args);
    if (!range_check(self, 1))
        return NULL;

    p = PyString_AS_STRING(self->o_data);
    return PyInt_FromLong((uint8_t)p[self->s_pos++]);
}

static PyObject *s_unpack_s8(BinstructObject *self, PyObject *args)
{
    const char *p;

    UNUSED(args);
    if (!range_check(self, 1))
        return NULL;

    p = PyString_AS_STRING(self->o_data);
    return PyInt_FromLong((int8_t)p[self->s_pos++]);
}

static PyObject *s_unpack_bool(BinstructObject *self, PyObject *args)
{
    const char *p;

    UNUSED(args);
    if (!range_check(self, 1))
        return NULL;

    p = PyString_AS_STRING(self->o_data);
    return PyBool_FromLong(p[self->s_pos++]);
}

static PyObject *s_unpack_ube16(BinstructObject *self, PyObject *args)
{
    uint16_t value;

    UNUSED(args);
    if (!range_check_and_copy(self, &value, sizeof(value)))
        return NULL;
#if PY_LITTLE_ENDIAN
    return PyInt_FromLong(value);
#else
    return PyInt_FromLong(byte_swap_u16(value));
#endif
}

static PyObject *s_unpack_sbe16(BinstructObject *self, PyObject *args)
{
    uint16_t value;

    UNUSED(args);
    if (!range_check_and_copy(self, &value, sizeof(value)))
        return NULL;
#if PY_LITTLE_ENDIAN
    return to_signed(value, sizeof(value));
#else
    return to_signed(byte_swap_u16(value), sizeof(value));
#endif
}

static PyObject *s_unpack_ube32(BinstructObject *self, PyObject *args)
{
    uint32_t value;

    UNUSED(args);
    if (!range_check_and_copy(self, &value, sizeof(value)))
        return NULL;
#if PY_LITTLE_ENDIAN
    return to_unsigned(value);
#else
    return to_unsigned(byte_swap_u32(value));
#endif
}

static PyObject *s_unpack_sbe32(BinstructObject *self, PyObject *args)
{
    uint32_t value;

    UNUSED(args);
    if (!range_check_and_copy(self, &value, sizeof(value)))
        return NULL;
#if PY_LITTLE_ENDIAN
    return to_signed(value, sizeof(value));
#else
    return to_signed(byte_swap_u32(value), sizeof(value));
#endif
}

static PyObject *s_unpack_ube64(BinstructObject *self, PyObject *args)
{
    uint64_t value;

    UNUSED(args);
    if (!range_check_and_copy(self, &value, sizeof(value)))
        return NULL;
#if PY_LITTLE_ENDIAN
    return PyLong_FromUnsignedLongLong(value);
#else
    return PyLong_FromUnsignedLongLong(byte_swap_u64(value));
#endif
}

static PyObject *s_unpack_sbe64(BinstructObject *self, PyObject *args)
{
    uint64_t value;

    UNUSED(args);
    if (!range_check_and_copy(self, &value, sizeof(value)))
        return NULL;
#if PY_LITTLE_ENDIAN
    return PyLong_FromLongLong(value);
#else
    return PyLong_FromLongLong(byte_swap_u64(value));
#endif
}

static PyObject *s_unpack_ule16(BinstructObject *self, PyObject *args)
{
    uint16_t value;

    UNUSED(args);
    if (!range_check_and_copy(self, &value, sizeof(value)))
        return NULL;
#if PY_LITTLE_ENDIAN
    return PyInt_FromLong(byte_swap_u16(value));
#else
    return PyInt_FromLong(value);
#endif
}

static PyObject *s_unpack_sle16(BinstructObject *self, PyObject *args)
{
    uint16_t value;

    UNUSED(args);
    if (!range_check_and_copy(self, &value, sizeof(value)))
        return NULL;
#if PY_LITTLE_ENDIAN
    return to_signed(byte_swap_u16(value), sizeof(value));
#else
    return to_signed(value, sizeof(value));
#endif
}

static PyObject *s_unpack_ule32(BinstructObject *self, PyObject *args)
{
    uint32_t value;

    UNUSED(args);
    if (!range_check_and_copy(self, &value, sizeof(value)))
        return NULL;
#if PY_LITTLE_ENDIAN
    return to_unsigned(byte_swap_u32(value));
#else
    return to_unsigned(value);
#endif
}

static PyObject *s_unpack_sle32(BinstructObject *self, PyObject *args)
{
    uint32_t value;

    UNUSED(args);
    if (!range_check_and_copy(self, &value, sizeof(value)))
        return NULL;
#if PY_LITTLE_ENDIAN
    return to_signed(byte_swap_u32(value), sizeof(value));
#else
    return to_signed(value, sizeof(value));
#endif
}

static PyObject *s_unpack_ule64(BinstructObject *self, PyObject *args)
{
    uint64_t value;

    UNUSED(args);
    if (!range_check_and_copy(self, &value, sizeof(value)))
        return NULL;
#if PY_LITTLE_ENDIAN
    return PyLong_FromUnsignedLongLong(byte_swap_u64(value));
#else
    return PyLong_FromUnsignedLongLong(value);
#endif
}

static PyObject *s_unpack_sle64(BinstructObject *self, PyObject *args)
{
    uint64_t value;

    UNUSED(args);
    if (!range_check_and_copy(self, &value, sizeof(value)))
        return NULL;
#if PY_LITTLE_ENDIAN
    return PyLong_FromLongLong(byte_swap_u64(value));
#else
    return PyLong_FromLongLong(value);
#endif
}

static PyObject *s_unpack_uleb128(BinstructObject *self, PyObject *args)
{
    uint64_t value = 0;

    UNUSED(args);
    if (!unpack_uleb128(self, &value))
        return NULL;

    if (value > LONG_MAX)
        return PyLong_FromUnsignedLongLong(value);
    else
        return PyInt_FromLong((long)value);
}

static PyObject *s_unpack_sleb128(BinstructObject *self, PyObject *args)
{
    uint64_t value = 0;
    unsigned shift = 0;
    const char *p;
    uint8_t b;

    UNUSED(args);
    p = PyString_AS_STRING(self->o_data);
    while (self->s_pos < self->s_len) {
        b = (uint8_t)p[self->s_pos++];
        value |= (b & 0x7f) << shift;
        shift += 7;
        if (b < 64)
            if (value > LONG_MAX)
                return PyLong_FromUnsignedLongLong(value);
            else
                return PyInt_FromLong((long)value);

        else if (b < 128) {
            value |= -U64(1) << shift;
            if ((int64_t)value < LONG_MIN)
                return PyLong_FromLongLong((int64_t)value);
            else
                return PyInt_FromLong((long)value);
        }
    }

    PyErr_SetString(BinstructError, "unpack requires more data for LEB128");
    return NULL;
}

static PyObject *s_unpack_string(BinstructObject *self, PyObject *args)
{
    Py_ssize_t len;
    const char *p;

    UNUSED(args);
    len = self->s_len - self->s_pos;
    if (!len) {
        PyErr_SetString(BinstructError, "unpack requires data for string");
        return NULL;
    }

    p = PyString_AS_STRING(self->o_data) + self->s_pos;
    len = (Py_ssize_t)strnlen(p, len);
    self->s_pos += len + 1;
    return PyString_FromStringAndSize(p, len);
}

static PyObject *s_unpack_block(BinstructObject *self, PyObject *args)
{
    const char *p;
    Py_ssize_t len;

    if (!PyArg_ParseTuple(args, "n:unpack_block", &len))
        return NULL;

    if (len < 0) {
        PyErr_SetString(BinstructError, "block length must not be negative");
        return NULL;
    }

    if (!range_check(self, len))
        return NULL;

    p = PyString_AS_STRING(self->o_data) + self->s_pos;
    self->s_pos += len;
    return PyString_FromStringAndSize(p, len);
}

static PyObject *s_unpack_block_u8(BinstructObject *self, PyObject *args)
{
    const char *p;

    UNUSED(args);
    if (!range_check(self, 1))
        return NULL;

    p = PyString_AS_STRING(self->o_data);
    return unpack_block(self, (uint8_t)p[self->s_pos++]);
}

static PyObject *s_unpack_block_be16(BinstructObject *self, PyObject *args)
{
    uint16_t len;

    UNUSED(args);
    if (!range_check_and_copy(self, &len, sizeof(len)))
        return NULL;
#if PY_LITTLE_ENDIAN
    return unpack_block(self, len);
#else
    return unpack_block(self, byte_swap_u16(len));
#endif
}

static PyObject *s_unpack_block_be32(BinstructObject *self, PyObject *args)
{
    uint32_t len;

    UNUSED(args);
    if (!range_check_and_copy(self, &len, sizeof(len)))
        return NULL;
#if PY_LITTLE_ENDIAN
    return unpack_block(self, len);
#else
    return unpack_block(self, byte_swap_u32(len));
#endif
}

static PyObject *s_unpack_block_be64(BinstructObject *self, PyObject *args)
{
    uint64_t len;

    UNUSED(args);
    if (!range_check_and_copy(self, &len, sizeof(len)))
        return NULL;
#if PY_LITTLE_ENDIAN
    return unpack_block(self, (Py_ssize_t)len);
#else
    return unpack_block(self, (Py_ssize_t)byte_swap_u64(len));
#endif
}

static PyObject *s_unpack_block_le16(BinstructObject *self, PyObject *args)
{
    uint16_t len;

    UNUSED(args);
    if (!range_check_and_copy(self, &len, sizeof(len)))
        return NULL;
#if PY_LITTLE_ENDIAN
    return unpack_block(self, byte_swap_u16(len));
#else
    return unpack_block(self, len);
#endif
}

static PyObject *s_unpack_block_le32(BinstructObject *self, PyObject *args)
{
    uint32_t len;

    UNUSED(args);
    if (!range_check_and_copy(self, &len, sizeof(len)))
        return NULL;
#if PY_LITTLE_ENDIAN
    return unpack_block(self, byte_swap_u32(len));
#else
    return unpack_block(self, len);
#endif
}

static PyObject *s_unpack_block_le64(BinstructObject *self, PyObject *args)
{
    uint64_t len;

    UNUSED(args);
    if (!range_check_and_copy(self, &len, sizeof(len)))
        return NULL;
#if PY_LITTLE_ENDIAN
    return unpack_block(self, (Py_ssize_t)byte_swap_u64(len));
#else
    return unpack_block(self, (Py_ssize_t)len);
#endif
}

static PyObject *s_unpack_block_uleb128(BinstructObject *self, PyObject *args)
{
    uint64_t len = 0;

    UNUSED(args);
    if (!unpack_uleb128(self, &len))
        return NULL;

    return unpack_block(self, (Py_ssize_t)len);
}

/* methods */
static struct PyMethodDef s_methods[] = {
    {"unpack_u8", (PyCFunction)s_unpack_u8, METH_NOARGS, NULL},
    {"unpack_s8", (PyCFunction)s_unpack_s8, METH_NOARGS, NULL},
    {"unpack_bool", (PyCFunction)s_unpack_bool, METH_NOARGS, NULL},
    {"unpack_ube16", (PyCFunction)s_unpack_ube16, METH_NOARGS, NULL},
    {"unpack_sbe16", (PyCFunction)s_unpack_sbe16, METH_NOARGS, NULL},
    {"unpack_ube32", (PyCFunction)s_unpack_ube32, METH_NOARGS, NULL},
    {"unpack_sbe32", (PyCFunction)s_unpack_sbe32, METH_NOARGS, NULL},
    {"unpack_ube64", (PyCFunction)s_unpack_ube64, METH_NOARGS, NULL},
    {"unpack_sbe64", (PyCFunction)s_unpack_sbe64, METH_NOARGS, NULL},
    {"unpack_ule16", (PyCFunction)s_unpack_ule16, METH_NOARGS, NULL},
    {"unpack_sle16", (PyCFunction)s_unpack_sle16, METH_NOARGS, NULL},
    {"unpack_ule32", (PyCFunction)s_unpack_ule32, METH_NOARGS, NULL},
    {"unpack_sle32", (PyCFunction)s_unpack_sle32, METH_NOARGS, NULL},
    {"unpack_ule64", (PyCFunction)s_unpack_ule64, METH_NOARGS, NULL},
    {"unpack_sle64", (PyCFunction)s_unpack_sle64, METH_NOARGS, NULL},
    {"unpack_uleb128", (PyCFunction)s_unpack_uleb128, METH_NOARGS, NULL},
    {"unpack_sleb128", (PyCFunction)s_unpack_sleb128, METH_NOARGS, NULL},
    {"unpack_string", (PyCFunction)s_unpack_string, METH_NOARGS, NULL},
    {"unpack_block", (PyCFunction)s_unpack_block, METH_VARARGS, NULL},
    {"unpack_block_u8", (PyCFunction)s_unpack_block_u8, METH_NOARGS, NULL},
    {"unpack_block_be16", (PyCFunction)s_unpack_block_be16, METH_NOARGS, NULL},
    {"unpack_block_be32", (PyCFunction)s_unpack_block_be32, METH_NOARGS, NULL},
    {"unpack_block_be64", (PyCFunction)s_unpack_block_be64, METH_NOARGS, NULL},
    {"unpack_block_le16", (PyCFunction)s_unpack_block_le16, METH_NOARGS, NULL},
    {"unpack_block_le32", (PyCFunction)s_unpack_block_le32, METH_NOARGS, NULL},
    {"unpack_block_le64", (PyCFunction)s_unpack_block_le64, METH_NOARGS, NULL},
    {"unpack_block_uleb128", (PyCFunction)s_unpack_block_uleb128, METH_NOARGS, NULL},
    {NULL, NULL}
};

/* properties */
static void s_dealloc(BinstructObject *obj)
{
    if (obj->weakrefs)
        PyObject_ClearWeakRefs((PyObject*)obj);

    Py_XDECREF(obj->o_data);
    Py_TYPE(obj)->tp_free((PyObject*)obj);
}

static PyObject *s_get_data(BinstructObject *self, void *closure)
{
    UNUSED(closure);
    Py_INCREF(self->o_data);
    return self->o_data;
}

static PyObject *s_get_pos(BinstructObject *self, void *closure)
{
    UNUSED(closure);
    return PyInt_FromSsize_t(self->s_pos);
}

static int s_set_pos(BinstructObject *self, PyObject *value, void *closure)
{
    Py_ssize_t pos;

    UNUSED(closure);
    if (value == NULL) {
        PyErr_SetString(PyExc_TypeError, "cannot delete the pos attribute");
        return -1;
    }

    if (PyInt_Check(value))
        pos = PyInt_AsSsize_t(value);
    else if (PyLong_Check(value))
        pos = PyLong_AsSsize_t(value);
    else {
        PyErr_SetString(PyExc_TypeError, "pos requires integer type");
        return -1;
    }

    if (pos < 0 || pos > self->s_len) {
        PyErr_SetString(BinstructError, "pos out of range");
        return -1;
    }

    self->s_pos = pos;
    return 0;

}

static PyGetSetDef s_getset[] = {
    {"data", (getter)s_get_data, (setter)NULL, "data string", NULL},
    {"pos", (getter)s_get_pos, (setter)s_set_pos, "byte position", NULL},
    {NULL}
};

/* typedef */
static PyTypeObject BinstructType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "Binstruct",
    sizeof(BinstructObject),
    0,
    (destructor)s_dealloc,                      /* tp_dealloc */
    0,                                          /* tp_print */
    0,                                          /* tp_getattr */
    0,                                          /* tp_setattr */
    0,                                          /* tp_compare */
    0,                                          /* tp_repr */
    0,                                          /* tp_as_number */
    0,                                          /* tp_as_sequence */
    0,                                          /* tp_as_mapping */
    0,                                          /* tp_hash */
    0,                                          /* tp_call */
    0,                                          /* tp_str */
    PyObject_GenericGetAttr,                    /* tp_getattro */
    PyObject_GenericSetAttr,                    /* tp_setattro */
    0,                                          /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE | Py_TPFLAGS_HAVE_WEAKREFS,/* tp_flags */
    0, /*s__doc__,*/                            /* tp_doc */
    0,                                          /* tp_traverse */
    0,                                          /* tp_clear */
    0,                                          /* tp_richcompare */
    offsetof(BinstructObject, weakrefs),        /* tp_weaklistoffset */
    0,                                          /* tp_iter */
    0,                                          /* tp_iternext */
    s_methods,                                  /* tp_methods */
    NULL,                                       /* tp_members */
    s_getset,                                   /* tp_getset */
    0,                                          /* tp_base */
    0,                                          /* tp_dict */
    0,                                          /* tp_descr_get */
    0,                                          /* tp_descr_set */
    0,                                          /* tp_dictoffset */
    (initproc)s_init,                           /* tp_init */
    PyType_GenericAlloc,                        /* tp_alloc */
    s_new,                                      /* tp_new */
    PyObject_Del,                               /* tp_free */
};

/* module definition */
static struct PyMethodDef module_functions[] = {
    {NULL, NULL}
};

/* module initialization */
PyMODINIT_FUNC init_binstruct2(void)
{
    PyObject *module;

    module = Py_InitModule3("_binstruct2", module_functions, NULL);
    if (!module)
        return;

    Py_TYPE(&BinstructType) = &PyType_Type;
    if (PyType_Ready(&BinstructType) < 0)
        return;

    if (!BinstructError) {
        BinstructError = PyErr_NewException("binstruct.error", NULL, NULL);
        if (!BinstructError)
            return;
    }

    Py_INCREF(BinstructError);
    PyModule_AddObject(module, "error", BinstructError);
    Py_INCREF((PyObject*)&BinstructType);
    PyModule_AddObject(module, "Binstruct", (PyObject*)&BinstructType);
}