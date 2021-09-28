#define PY_SSIZE_T_CLEAN
#include "hail.h"
#include <Python.h>

typedef struct _HailTypeObject {
	PyObject_HEAD
	struct hail_type *type;
} HailTypeObject;

/* basic hail type methods */
static PyObject *HailType_str(HailTypeObject *self);
static PyObject *HailType_repr(HailTypeObject *self);
static PyObject *HailType_richcmp(HailTypeObject *self, PyObject *other, int op);
static Py_hash_t HailType_hash(HailTypeObject *self);

static PyTypeObject HailType = {
	.tp_name = "_hail.HailType",
	.tp_basicsize = sizeof(HailTypeObject),
	.tp_itemsize = 0, /* itemsize */
	.tp_repr = (reprfunc)HailType_repr,
	.tp_hash = (hashfunc)HailType_hash,
	.tp_str = (reprfunc)HailType_str,
	.tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
	.tp_doc = "Hail type superclass",
	.tp_richcompare = (richcmpfunc)HailType_richcmp,
	.tp_new = PyType_GenericNew,
	.ob_base = PyVarObject_HEAD_INIT(NULL, 0)
};

static PyObject *TVoid_new(PyTypeObject *subtype, PyObject *args, PyObject *kwds);
static PyTypeObject TVoid = {
	.tp_name = "_hail._tvoid",
	.tp_flags = Py_TPFLAGS_DEFAULT,
	.tp_doc = "Hail type for boolean (true or false) values",
	.tp_base = &HailType,
	.tp_new = TVoid_new,
	.ob_base = PyVarObject_HEAD_INIT(NULL, 0)
};

static PyObject *TBool_new(PyTypeObject *subtype, PyObject *args, PyObject *kwds);
static PyTypeObject TBool = {
	.tp_name = "_hail._tbool",
	.tp_flags = Py_TPFLAGS_DEFAULT,
	.tp_doc = "Hail type for boolean (true or false) values",
	.tp_base = &HailType,
	.tp_new = TBool_new,
	.ob_base = PyVarObject_HEAD_INIT(NULL, 0)
};

static PyObject *TInt32_new(PyTypeObject *subtype, PyObject *args, PyObject *kwds);
static PyTypeObject TInt32 = {
	.tp_name = "_hail._tint32",
	.tp_flags = Py_TPFLAGS_DEFAULT,
	.tp_doc = "Hail type for 32 bit signed integers",
	.tp_base = &HailType,
	.tp_new = TInt32_new,
	.ob_base = PyVarObject_HEAD_INIT(NULL, 0)
};

static PyObject *TInt64_new(PyTypeObject *subtype, PyObject *args, PyObject *kwds);
static PyTypeObject TInt64 = {
	.tp_name = "_hail._tint64",
	.tp_flags = Py_TPFLAGS_DEFAULT,
	.tp_doc = "Hail type for 64 bit signed integers",
	.tp_base = &HailType,
	.tp_new = TInt64_new,
	.ob_base = PyVarObject_HEAD_INIT(NULL, 0)
};

static PyObject *TFloat32_new(PyTypeObject *subtype, PyObject *args, PyObject *kwds);
static PyTypeObject TFloat32 = {
	.tp_name = "_hail._tfloat32",
	.tp_flags = Py_TPFLAGS_DEFAULT,
	.tp_doc = "Hail type for IEEE-754 32 bit floating point numbers",
	.tp_base = &HailType,
	.tp_new = TFloat32_new,
	.ob_base = PyVarObject_HEAD_INIT(NULL, 0)
};

static PyObject *TFloat64_new(PyTypeObject *subtype, PyObject *args, PyObject *kwds);
static PyTypeObject TFloat64 = {
	.tp_name = "_hail._tfloat64",
	.tp_flags = Py_TPFLAGS_DEFAULT,
	.tp_doc = "Hail type for IEEE-754 64 bit floating point numbers",
	.tp_base = &HailType,
	.tp_new = TFloat64_new,
	.ob_base = PyVarObject_HEAD_INIT(NULL, 0)
};

static PyObject *TStr_new(PyTypeObject *subtype, PyObject *args, PyObject *kwds);
static PyTypeObject TStr = {
	.tp_name = "_hail._tstr",
	.tp_flags = Py_TPFLAGS_DEFAULT,
	.tp_doc = "Hail type for strings",
	.tp_base = &HailType,
	.tp_new = TStr_new,
	.ob_base = PyVarObject_HEAD_INIT(NULL, 0)
};

static PyObject *TCall_new(PyTypeObject *subtype, PyObject *args, PyObject *kwds);
static PyTypeObject TCall = {
	.tp_name = "_hail._tcall",
	.tp_flags = Py_TPFLAGS_DEFAULT,
	.tp_doc = "Hail type for genotypes",
	.tp_base = &HailType,
	.tp_new = TCall_new,
	.ob_base = PyVarObject_HEAD_INIT(NULL, 0)
};

static void dealloc_complex_type(HailTypeObject *self);

static PyObject *ArrayTypes; /* an arena for array types */
static PyObject *TArray_new(PyTypeObject *subtype, PyObject *args, PyObject *kwds);
static PyObject *TArray_get_element_type(HailTypeObject *self, void *closure);
static PyObject *TArray_str(HailTypeObject *self);
static PyGetSetDef TArray_getset[] = {
	{"element_type", (getter) TArray_get_element_type, NULL, "element type", NULL},
	{NULL, NULL, NULL, NULL, NULL}  /* Sentinel */
};
static PyTypeObject TArray = {
	.tp_name = "_hail.tarray",
	.tp_flags = Py_TPFLAGS_DEFAULT,
	.tp_doc = "Hail type for arrays",
	.tp_getset = TArray_getset,
	.tp_base = &HailType,
	.tp_new = TArray_new,
	.tp_str = (reprfunc)TArray_str,
	.tp_dealloc = (destructor)dealloc_complex_type,
	.ob_base = PyVarObject_HEAD_INIT(NULL, 0)
};

static PyObject *TupleTypes; /* an arena for tuple types */
static PyObject *ttuple_to_python(struct hail_ttuple *tuple);
static PyObject *TTuple_new(PyTypeObject *subtype, PyObject *args, PyObject *kwds);
static Py_ssize_t TTuple_len(HailTypeObject *self);
static PyObject *TTuple_str(HailTypeObject *self);
static PyObject *TTuple_getitem(HailTypeObject *self, Py_ssize_t index);
static PySequenceMethods TTuple_SeqMethods = {
	.sq_length = (lenfunc)TTuple_len,
	.sq_item = (ssizeargfunc)TTuple_getitem,
};

// FIXME possibly add a faster iterator
static PyTypeObject TTuple = {
	.tp_name = "_hail.ttuple",
	.tp_as_sequence = &TTuple_SeqMethods,
	.tp_flags = Py_TPFLAGS_DEFAULT,
	.tp_doc = "Hail type for tuples",
	.tp_base = &HailType,
	.tp_new = TTuple_new,
	.tp_str = (reprfunc)TTuple_str,
	.tp_dealloc = (destructor)dealloc_complex_type,
	.ob_base = PyVarObject_HEAD_INIT(NULL, 0)
};

static void
_hail_m_free(void *data)
{
	Py_XDECREF(TupleTypes);
	Py_XDECREF(ArrayTypes);
}

PyDoc_STRVAR(_hail_module_doc, "hail native methods/interface");
static struct PyModuleDef hail_module = {
	PyModuleDef_HEAD_INIT,
	.m_doc = _hail_module_doc,
	.m_name="_hail",
	.m_size=-1,
	.m_free=_hail_m_free
};

// public object singletons
static PyObject *tvoid, *tbool, *tint32, *tint64, *tfloat32, *tfloat64, *tstr, *tcall;

PyMODINIT_FUNC
PyInit__hail(void)
{
	PyObject *mod = PyModule_Create(&hail_module);
	if (mod == NULL) {
		goto error;
	}

	if (PyType_Ready(&HailType) < 0
		|| PyType_Ready(&TVoid) < 0
		|| PyType_Ready(&TBool) < 0
		|| PyType_Ready(&TInt32) < 0
		|| PyType_Ready(&TInt64) < 0
		|| PyType_Ready(&TFloat32) < 0
		|| PyType_Ready(&TFloat64) < 0
		|| PyType_Ready(&TStr) < 0
		|| PyType_Ready(&TCall) < 0
		|| PyType_Ready(&TArray) < 0
		|| PyType_Ready(&TTuple) < 0)
	{
		goto error;
	}

	tvoid = TVoid.tp_new(&TVoid, NULL, NULL);
	tbool = TBool.tp_new(&TBool, NULL, NULL);
	tint32 = TInt32.tp_new(&TInt32, NULL, NULL);
	tint64 = TInt64.tp_new(&TInt64, NULL, NULL);
	tfloat32 = TFloat32.tp_new(&TFloat32, NULL, NULL);
	tfloat64 = TFloat64.tp_new(&TFloat64, NULL, NULL);
	tstr = TStr.tp_new(&TStr, NULL, NULL);
	tcall = TCall.tp_new(&TCall, NULL, NULL);
	if (!tvoid || !tbool || !tint32 || !tint64 || !tfloat32 || !tfloat64 || !tstr || !tcall) {
		goto error;
	}

	Py_INCREF(tvoid);
	if (PyModule_AddObject(mod, "tvoid", tvoid) < 0) {
		Py_DECREF(tvoid);
		goto error;
	}

	Py_INCREF(tbool);
	if (PyModule_AddObject(mod, "tbool", tbool) < 0) {
		Py_DECREF(tbool);
		goto error;
	}

	Py_INCREF(tint32);
	if (PyModule_AddObject(mod, "tint32", tint32) < 0) {
		Py_DECREF(tint32);
		goto error;
	}

	Py_INCREF(tint64);
	if (PyModule_AddObject(mod, "tint64", tint64) < 0) {
		Py_DECREF(tint64);
		goto error;
	}

	Py_INCREF(tfloat32);
	if (PyModule_AddObject(mod, "tfloat32", tfloat32) < 0) {
		Py_DECREF(tfloat32);
		goto error;
	}

	Py_INCREF(tfloat64);
	if (PyModule_AddObject(mod, "tfloat64", tfloat64) < 0) {
		Py_DECREF(tfloat64);
		goto error;
	}

	Py_INCREF(tstr);
	if (PyModule_AddObject(mod, "tstr", tstr) < 0) {
		Py_DECREF(tstr);
		goto error;
	}

	Py_INCREF(tcall);
	if (PyModule_AddObject(mod, "tcall", tcall) < 0) {
		Py_DECREF(tcall);
		goto error;
	}

	Py_INCREF(&TArray);
	if (PyModule_AddObject(mod, "tarray", (PyObject *)&TArray) < 0) {
		Py_DECREF(&TArray);
		goto error;
	}

	Py_INCREF(&TTuple);
	if (PyModule_AddObject(mod, "ttuple", (PyObject *)&TTuple) < 0) {
		Py_DECREF(&TTuple);
		goto error;
	}

	ArrayTypes = PyDict_New();
	if (ArrayTypes == NULL) goto error;

	TupleTypes = PyDict_New();
	if (TupleTypes == NULL) goto error;

	return mod;
error:
	Py_XDECREF(tvoid);
	Py_XDECREF(tbool);
	Py_XDECREF(tint32);
	Py_XDECREF(tint64);
	Py_XDECREF(tfloat32);
	Py_XDECREF(tfloat64);
	Py_XDECREF(tstr);
	Py_XDECREF(tcall);
	Py_XDECREF(ArrayTypes);
	Py_XDECREF(TupleTypes);
	Py_XDECREF(mod);
	return NULL;
}

static PyObject *
HailType_str(HailTypeObject *self)
{
	if (self->type == NULL) {
		Py_RETURN_NONE;
	}

	switch (self->type->tag) {
	case HAIL_TY_VOID:
		return PyUnicode_FromString("void");
	case HAIL_TY_BOOL:
		return PyUnicode_FromString("bool");
	case HAIL_TY_INT32:
		return PyUnicode_FromString("int32");
	case HAIL_TY_INT64:
		return PyUnicode_FromString("int64");
	case HAIL_TY_FLOAT32:
		return PyUnicode_FromString("float32");
	case HAIL_TY_FLOAT64:
		return PyUnicode_FromString("float64");
	case HAIL_TY_STR:
		return PyUnicode_FromString("str");
	case HAIL_TY_CALL:
		return PyUnicode_FromString("call");
	default:
		return PyErr_Format(PyExc_TypeError, "HailType.__str__ called with %s",
				Py_TYPE(self)->tp_name);
	}

}

static PyObject *
HailType_repr(HailTypeObject *self)
{
	PyObject *str = PyObject_Str((PyObject *)self);

	if (!PyUnicode_Check(str)) {
		// this is None (or NULL), return it
		return str;
	}

	// constants, intermidiate, and return strings
	PyObject *substr = NULL, *repstr = NULL, *tmpstr = NULL, *retstr = NULL;
	substr = PyUnicode_FromString("'");
	if (substr == NULL) goto hailtype_repr_out;
	repstr = PyUnicode_FromString("\\'");
	if (repstr == NULL) goto hailtype_repr_out;
	tmpstr = PyUnicode_Replace(str, substr, repstr, -1);
	if (tmpstr == NULL) goto hailtype_repr_out;
	retstr = PyUnicode_FromFormat("dtype('%U')", tmpstr);

hailtype_repr_out:
	Py_DECREF(str);
	Py_XDECREF(substr);
	Py_XDECREF(repstr);
	Py_XDECREF(tmpstr);
	return retstr;
}

static Py_hash_t
HailType_hash(HailTypeObject *self)
{
	/* bottom 3 or 4 bits are likely to be 0; rotate y by 4 to avoid
	   excessive hash collisions for dicts and sets */
	uintptr_t y = (size_t)self->type;
	y = (y >> 4) | (y << (8 * SIZEOF_VOID_P - 4));
	Py_hash_t hash = y;
	return hash == -1 ? -2 : hash;
}

static PyObject *
HailType_richcmp(HailTypeObject *self, PyObject *other, int op)
{
	if (op != Py_EQ && op != Py_NE) {
		Py_RETURN_NOTIMPLEMENTED;
	}
	switch (PyObject_IsInstance(other, (PyObject*)&HailType)) {
	case -1: return NULL;
	case 1: break;
	default: Py_RETURN_FALSE;
	}
	Py_RETURN_RICHCOMPARE(self->type, ((HailTypeObject*)other)->type, op);
}

static PyObject *
HailType_New(PyTypeObject *type, struct hail_type *hail_type)
{
	HailTypeObject *self = (HailTypeObject *)type->tp_alloc(type, 0);
	if (self == NULL) return PyErr_NoMemory();
	self->type = hail_type;
	return (PyObject *)self;
}

#define RETURN_TVOID do { Py_INCREF(tvoid); return tvoid; } while (0)
#define RETURN_TBOOL do { Py_INCREF(tbool); return tbool; } while (0)
#define RETURN_TINT32 do { Py_INCREF(tint32); return tint32; } while (0)
#define RETURN_TINT64 do { Py_INCREF(tint64); return tint64; } while (0)
#define RETURN_TFLOAT32 do { Py_INCREF(tfloat32); return tfloat32; } while (0)
#define RETURN_TFLOAT64 do { Py_INCREF(tfloat64); return tfloat64; } while (0)
#define RETURN_TSTR do { Py_INCREF(tstr); return tstr; } while (0)
#define RETURN_TCALL do { Py_INCREF(tcall); return tcall; } while (0)

static PyObject *
HailType_from_Type(struct hail_type *type)
{
	switch (type->tag) {
	case HAIL_TY_VOID:
		RETURN_TVOID;
	case HAIL_TY_BOOL:
		RETURN_TBOOL;
	case HAIL_TY_INT32:
		RETURN_TINT32;
	case HAIL_TY_INT64:
		RETURN_TINT64;
	case HAIL_TY_FLOAT32:
		RETURN_TFLOAT32;
	case HAIL_TY_FLOAT64:
		RETURN_TFLOAT64;
	case HAIL_TY_STR:
		RETURN_TSTR;
	case HAIL_TY_CALL:
		RETURN_TCALL;
	case HAIL_TY_ARRAY: {
		struct hail_type *elt_ty = ((struct hail_tarray *)type)->element_ty;
		PyObject* ty = HailType_from_Type(elt_ty);
		if (ty == NULL) return NULL;
		PyObject* args = Py_BuildValue("(O)", ty);
		Py_DECREF(ty);
		if (args == NULL) return NULL;
		PyObject* obj = TArray.tp_new(&TArray, args, NULL);
		Py_DECREF(args);
		return obj;
	}
	case HAIL_TY_TUPLE: {
		PyObject *args = ttuple_to_python((struct hail_ttuple *)type);
		PyObject *obj = TTuple.tp_new(&TTuple, args, NULL);
		Py_DECREF(args);
		return obj;
	}
	}

	abort();
}

static void
dealloc_complex_type(HailTypeObject *self)
{
	PyTypeObject *type = Py_TYPE(self);
	PyMem_Free(self->type);
	type->tp_free(self);
}


static PyObject *
TVoid_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
	return HailType_New(type, hail_tvoid);
}

static PyObject *
TBool_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
	return HailType_New(type, hail_tbool);
}

static PyObject *
TInt32_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
	return HailType_New(type, hail_tint32);
}

static PyObject *
TInt64_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
	return HailType_New(type, hail_tint64);
}

static PyObject *
TFloat32_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
	return HailType_New(type, hail_tfloat32);
}

static PyObject *
TFloat64_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
	return HailType_New(type, hail_tfloat64);
}

static PyObject *
TStr_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
	return HailType_New(type, hail_tstr);
}

static PyObject *
TCall_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
	return HailType_New(type, hail_tcall);
}

static PyObject *
TArray_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
	const char *kw_names[] = {"element_type", NULL};
	HailTypeObject *py_element_type;
	if (!PyArg_ParseTupleAndKeywords(args, kwds, "O!:__new__", (char **)kw_names,
&HailType, &py_element_type)) {
		return NULL;
	}

	PyObject *memo = PyDict_GetItem(ArrayTypes, (PyObject *)py_element_type);
	if (memo != NULL) {
		Py_INCREF(memo); /* take ownership */
		return memo;
	}

	struct hail_tarray *tarray = PyMem_Malloc(sizeof(struct hail_tarray));
	if (tarray == NULL)
		return PyErr_NoMemory();

	tarray->ty.tag = HAIL_TY_ARRAY;
	tarray->element_ty = py_element_type->type;
	PyObject *self = HailType_New(type, &tarray->ty);
	if (self == NULL) {
		PyMem_Free(tarray);
		return self;
	}

	if (PyDict_SetItem(ArrayTypes, (PyObject *)py_element_type, self) < 0) {
		Py_XDECREF(self);
		return PyErr_Format(PyExc_SystemError, "internal error saving array type to arena");
	}

	return self;
}

static PyObject *
TArray_str(HailTypeObject *self)
{
	PyObject *ety = TArray_get_element_type(self, NULL);
	PyObject *str = PyUnicode_FromFormat("array<%S>", ety);
	Py_DECREF(ety);
	return str;
}

static PyObject *
TTuple_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
	Py_ssize_t n_args = PyTuple_Size(args);

	for (Py_ssize_t i = 0; i < n_args; i++) {
		PyObject *item = PyTuple_GET_ITEM(args, i);
    		switch (PyObject_IsInstance(item, (PyObject*)&HailType)) {
      		case -1:
      			return NULL;
      		case 0:
        		return PyErr_Format(PyExc_TypeError,
                     		"expected argument %d of __init__() to be %.200s, "
                     		"not %.200s", i + 1, HailType.tp_name,
                     		_PyType_Name(Py_TYPE(item)));
      		case 1:
      			break;
      		default:
      			abort();
    		}
	}

	PyObject *memo = PyDict_GetItem(TupleTypes, args);
	if (memo != NULL) {
		Py_INCREF(memo);
		return memo;
	}

	struct hail_ttuple *ttuple = PyMem_Malloc(sizeof(struct hail_ttuple)
		+ n_args * sizeof(struct hail_type *));
	if (ttuple == NULL)
		return PyErr_NoMemory();

	ttuple->ty.tag = HAIL_TY_TUPLE;
	ttuple->len = n_args;
	for (Py_ssize_t i = 0; i < n_args; i++) {
		HailTypeObject *item = (void *)PyTuple_GET_ITEM(args, i);
		ttuple->types[i] = item->type;
	}

	PyObject *self = HailType_New(type, &ttuple->ty);
	if (self == NULL) {
		PyMem_Free(ttuple);
		return self;
	}

	if (PyDict_SetItem(TupleTypes, args, self) < 0) {
		Py_XDECREF(self);
		return PyErr_Format(PyExc_SystemError, "internal error saving tuple type to arena");
	}

	return self;
}

static PyObject *
TArray_get_element_type(HailTypeObject *self, void *closure)
{
	return HailType_from_Type(((struct hail_tarray *)self->type)->element_ty);
}

static PyObject *
TTuple_str(HailTypeObject *self)
{
	PyObject *types = ttuple_to_python((void *)self->type);
	if (types == NULL) return NULL;
	Py_ssize_t n = PyTuple_Size(types);
	for (Py_ssize_t i = 0; i < n; i++) {
		PyTuple_SetItem(types, i, PyObject_Str(PyTuple_GET_ITEM(types, i)));
	}
	PyObject *sep = PyUnicode_FromString(", ");
	PyObject *joined = PyUnicode_Join(sep, types);
	PyObject *str = PyUnicode_FromFormat("tuple(%U)", joined);
	Py_DECREF(types);
	Py_XDECREF(sep);
	Py_XDECREF(joined);
	return str;
}

static Py_ssize_t
TTuple_len(HailTypeObject *self)
{
	return ((struct hail_ttuple *)self->type)->len;
}

static PyObject *
TTuple_getitem(HailTypeObject *self, Py_ssize_t index)
{
	struct hail_ttuple *tuple = (struct hail_ttuple *)self->type;
	if (index >= tuple->len) {
		return PyErr_Format(PyExc_IndexError, "index %zd out of range for length %d",
			index, tuple->len);
	}
	return HailType_from_Type(tuple->types[index]);
}

static PyObject *
ttuple_to_python(struct hail_ttuple *ttuple)
{
	PyObject *args = PyTuple_New(ttuple->len);
	if (args == NULL) return args;
	for (int i = 0; i < ttuple->len; i++) {
		PyTuple_SET_ITEM(args, i, HailType_from_Type(ttuple->types[i]));
	}
	return args;
}
