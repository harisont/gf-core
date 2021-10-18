#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <structmember.h>

#include <pgf/pgf.h>
#include "./expr.h"
#include "./ffi.h"
#include "./transactions.h"

// ----------------------------------------------------------------------------
// PGF: the grammar object

static void
PGF_dealloc(PGFObject *self)
{
    if (self->db != NULL && self->revision != 0)
        pgf_free_revision(self->db, self->revision);
    Py_TYPE(self)->tp_free(self);
}

static PyObject *
PGF_writeToFile(PGFObject *self, PyObject *args)
{
    const char *fpath;
    if (!PyArg_ParseTuple(args, "s", &fpath))
        return NULL;

    PgfExn err;
    pgf_write_pgf(fpath, self->db, self->revision, &err);
    if (handleError(err) != PGF_EXN_NONE) {
        return NULL;
    }

    Py_RETURN_NONE;
}

typedef struct {
    PgfItor fn;
    PGFObject *grammar;
    void *collection;
} PyPGFClosure;

// static void
// pgf_collect_langs_seq(GuMapItor* fn, const void* key, void* value, GuExn* err)
// {
//     PgfConcr* concr = *((PgfConcr**) value);
//     PyPGFClosure* clo = (PyPGFClosure*) fn;
//
//     gu_buf_push((GuBuf*) clo->collection, PgfConcr*, concr);
// }

// static PyObject*
// PGF_str(PGFObject *self)
// {
    // GuPool* tmp_pool = gu_local_pool();
    //
    // GuExn* err = gu_exn(tmp_pool);
    // GuStringBuf* sbuf = gu_new_string_buf(tmp_pool);
    // GuOut* out = gu_string_buf_out(sbuf);
    //
    // GuBuf* languages = gu_new_buf(PgfConcr*, tmp_pool);
    //
    // PyPGFClosure clo = { { pgf_collect_langs_seq }, self, languages };
    // pgf_iter_languages(self->pgf, &clo.fn, err);
    //
    // pgf_print(self->pgf, gu_buf_length(languages),
    //                      gu_buf_data(languages),
    //                      out, err);
    //
    // PyObject* pystr = PyString_FromStringAndSize(gu_string_buf_data(sbuf),
    //                                              gu_string_buf_length(sbuf));
    //
    // gu_pool_free(tmp_pool);
    // return pystr;
//     return NULL;
// }

static PyObject *
PGF_getAbstractName(PGFObject *self, void *closure)
{
    PgfExn err;
    PgfText *txt = pgf_abstract_name(self->db, self->revision, &err);

    if (handleError(err) != PGF_EXN_NONE) {
        return NULL;
    }

    PyObject *name = PyUnicode_FromStringAndSize(txt->text, txt->size);
    free(txt);
    return name;
}

static void
_collect_cats(PgfItor *fn, PgfText *key, void *value, PgfExn *err)
{
    PgfText *name = key;
    PyPGFClosure *clo = (PyPGFClosure*) fn;

    PyObject *py_name = PyUnicode_FromStringAndSize(name->text, name->size);
    if (py_name == NULL) {
        err->type = PGF_EXN_OTHER_ERROR;
        return;
    }

    if (PyList_Append((PyObject*) clo->collection, py_name) != 0) {
        err->type = PGF_EXN_OTHER_ERROR;
        Py_DECREF(py_name);
    }
}

static PyObject *
PGF_getCategories(PGFObject *self, void *closure)
{
    PyObject *categories = PyList_New(0);
    if (categories == NULL)
        return NULL;

    PgfExn err;
    PyPGFClosure clo = { { _collect_cats }, self, categories };
    pgf_iter_categories(self->db, self->revision, &clo.fn, &err);
    if (handleError(err) != PGF_EXN_NONE) {
        Py_DECREF(categories);
        return NULL;
    }

    return categories;
}

static PyObject *
PGF_categoryContext(PGFObject *self, PyObject *args)
{
    const char *s;
    Py_ssize_t size;
    if (!PyArg_ParseTuple(args, "s#", &s, &size))
        return NULL;

    PgfText *catname = CString_AsPgfText(s, size);

    PgfExn err;
    size_t n_hypos;
    PgfTypeHypo *hypos = pgf_category_context(self->db, self->revision, catname, &n_hypos, &unmarshaller, &err);
    FreePgfText(catname);
    if (handleError(err) != PGF_EXN_NONE) {
        return NULL;
    }

    if (hypos == NULL) {
        Py_RETURN_NONE;
    }

    PyObject *contexts = PyTuple_FromHypos(hypos, n_hypos);

    for (size_t i = 0; i < n_hypos; i++) {
        free(hypos[i].cid);
        Py_DECREF((PyObject *)hypos[i].type);
    }
    free(hypos);

    return contexts;
}

static TypeObject *
PGF_getStartCat(PGFObject *self, void *closure)
{
    PgfExn err;
    PgfType type = pgf_start_cat(self->db, self->revision, &unmarshaller, &err);

    if (type == 0) {
        PyErr_SetString(PGFError, "start category cannot be found");
        return NULL;
    }
    else if (handleError(err) != PGF_EXN_NONE) {
        return NULL;
    }

    return (TypeObject *)type;
}

static void
_collect_funs(PgfItor *fn, PgfText *key, void *value, PgfExn *err)
{
    PgfText *name = key;
    PyPGFClosure *clo = (PyPGFClosure*) fn;

    PyObject *py_name = PyUnicode_FromStringAndSize(name->text, name->size);
    if (py_name == NULL) {
        err->type = PGF_EXN_OTHER_ERROR;
        return;
    }

    if (PyList_Append((PyObject*) clo->collection, py_name) != 0) {
        err->type = PGF_EXN_OTHER_ERROR;
        Py_DECREF(py_name);
    }
}

static PyObject *
PGF_getFunctions(PGFObject *self, void *closure)
{
    PyObject *functions = PyList_New(0);
    if (functions == NULL)
        return NULL;

    PgfExn err;
    PyPGFClosure clo = { { _collect_funs }, self, functions };
    pgf_iter_functions(self->db, self->revision, &clo.fn, &err);
    if (handleError(err) != PGF_EXN_NONE) {
        Py_DECREF(functions);
        return NULL;
    }

    return functions;
}

static PyObject *
PGF_functionsByCat(PGFObject *self, PyObject *args)
{
    const char *s;
    Py_ssize_t size;
    if (!PyArg_ParseTuple(args, "s#", &s, &size))
        return NULL;

    PgfText *catname = CString_AsPgfText(s, size);

    PyObject *functions = PyList_New(0);
    if (functions == NULL) {
        FreePgfText(catname);
        return NULL;
    }

    PgfExn err;
    PyPGFClosure clo = { { _collect_funs }, self, functions };
    pgf_iter_functions_by_cat(self->db, self->revision, catname, &clo.fn, &err);
    FreePgfText(catname);
    if (handleError(err) != PGF_EXN_NONE) {
        Py_DECREF(functions);
        return NULL;
    }

    return functions;
}

static TypeObject *
PGF_functionType(PGFObject *self, PyObject *args)
{
    const char *s;
    Py_ssize_t size;
    if (!PyArg_ParseTuple(args, "s#", &s, &size))
        return NULL;

    PgfText *funname = CString_AsPgfText(s, size);

    PgfExn err;
    PgfType type = pgf_function_type(self->db, self->revision, funname, &unmarshaller, &err);
    FreePgfText(funname);
    if (type == 0) {
        PyErr_Format(PyExc_KeyError, "function '%s' is not defined", s);
        return NULL;
    }
    else if (handleError(err) != PGF_EXN_NONE) {
        return NULL;
    }

    return (TypeObject *)type;
}

static PyObject *
PGF_functionIsConstructor(PGFObject *self, PyObject *args)
{
    const char *s;
    Py_ssize_t size;
    if (!PyArg_ParseTuple(args, "s#", &s, &size))
        return NULL;

    PgfText *funname = CString_AsPgfText(s, size);

    PgfExn err;
    int isCon = pgf_function_is_constructor(self->db, self->revision, funname, &err);
    FreePgfText(funname);
    if (handleError(err) != PGF_EXN_NONE) {
        return NULL;
    }

    return PyBool_FromLong(isCon);
}

static PyObject *
PGF_categoryProbability(PGFObject *self, PyObject *args)
{
    const char *s;
    Py_ssize_t size;
    if (!PyArg_ParseTuple(args, "s#", &s, &size))
        return NULL;

    PgfText *catname = CString_AsPgfText(s, size);

    PgfExn err;
    prob_t prob = pgf_category_prob(self->db, self->revision, catname, &err);
    FreePgfText(catname);
    if (handleError(err) != PGF_EXN_NONE) {
        return NULL;
    }

    double dprob = (double) prob;
    if (dprob == INFINITY) {
        PyErr_Format(PyExc_KeyError, "category '%s' is not defined", s);
        return NULL;
    }
    return PyFloat_FromDouble(dprob);
}

static PyObject *
PGF_functionProbability(PGFObject *self, PyObject *args)
{
    const char *s;
    Py_ssize_t size;
    if (!PyArg_ParseTuple(args, "s#", &s, &size))
        return NULL;

    PgfText *funname = CString_AsPgfText(s, size);

    PgfExn err;
    prob_t prob = pgf_function_prob(self->db, self->revision, funname, &err);
    FreePgfText(funname);
    if (handleError(err) != PGF_EXN_NONE) {
        return NULL;
    }

    double dprob = (double) prob;
    if (dprob == INFINITY) {
        PyErr_Format(PyExc_KeyError, "function '%s' is not defined", s);
        return NULL;
    }
    return PyFloat_FromDouble(dprob);
}

static PyObject *
PGF_exprProbability(PGFObject *self, PyObject *args)
{
    ExprObject *expr;
    if (!PyArg_ParseTuple(args, "O!", &pgf_ExprType, &expr))
        return NULL;

    PgfExn err;
    prob_t prob = pgf_expr_prob(self->db, self->revision, (PgfExpr) expr, &marshaller, &err);
    if (handleError(err) != PGF_EXN_NONE) {
        return NULL;
    }

    double dprob = (double) prob;
    return PyFloat_FromDouble(dprob);
}

static PyGetSetDef PGF_getseters[] = {
    {"abstractName",
     (getter)PGF_getAbstractName, NULL,
     "the abstract syntax name",
     NULL},
    {"categories",
     (getter)PGF_getCategories, NULL,
     "a list containing all categories in the grammar",
     NULL},
    {"startCat",
     (getter)PGF_getStartCat, NULL,
     "the start category for the grammar",
     NULL},
    {"functions",
     (getter)PGF_getFunctions, NULL,
     "a list containing all functions in the grammar",
     NULL},
    {NULL}  /* Sentinel */
};

static PyMemberDef PGF_members[] = {
    {NULL}  /* Sentinel */
};

static PyMethodDef PGF_methods[] = {
    {"writeToFile", (PyCFunction)PGF_writeToFile, METH_VARARGS,
     "Writes PGF to file"},

    {"categoryContext", (PyCFunction)PGF_categoryContext, METH_VARARGS,
     "Returns the context for a given category"
    },
    {"functionsByCat", (PyCFunction)PGF_functionsByCat, METH_VARARGS,
     "Returns the list of functions for a given category"
    },
    {"functionType", (PyCFunction)PGF_functionType, METH_VARARGS,
     "Returns the type of a function"
    },
    {"functionIsConstructor", (PyCFunction)PGF_functionIsConstructor, METH_VARARGS,
     "Checks whether a function is a constructor"
    },
    {"categoryProbability", (PyCFunction)PGF_categoryProbability, METH_VARARGS,
     "Returns the probability of a category"
    },
    {"functionProbability", (PyCFunction)PGF_functionProbability, METH_VARARGS,
     "Returns the probability of a function"
    },
    {"exprProbability", (PyCFunction)PGF_exprProbability, METH_VARARGS,
     "Returns the probability of an expression"
    },

    {"checkoutBranch", (PyCFunction)PGF_checkoutBranch, METH_VARARGS,
     "Switch to a branch"
    },
    {"newTransaction", (PyCFunction)PGF_newTransaction, METH_VARARGS,
     "Create new transaction"
    },

    {"getGlobalFlag", (PyCFunction)PGF_getGlobalFlag, METH_VARARGS,
     "Get the value of a global flag"
    },
    {"getAbstractFlag", (PyCFunction)PGF_getAbstractFlag, METH_VARARGS,
     "Get the value of an abstract flag"
    },

    {NULL}  /* Sentinel */
};

static PyTypeObject pgf_PGFType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    //0,                         /*ob_size*/
    "pgf.PGF",                 /*tp_name*/
    sizeof(PGFObject),         /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    (destructor)PGF_dealloc,   /*tp_dealloc*/
    0,                         /*tp_print*/
    0,                         /*tp_getattr*/
    0,                         /*tp_setattr*/
    0,                         /*tp_compare*/
    0,                         /*tp_repr*/
    0,                         /*tp_as_number*/
    0,                         /*tp_as_sequence*/
    0,                         /*tp_as_mapping*/
    0,                         /*tp_hash */
    0,                         /*tp_call*/
    0, // (reprfunc) PGF_str,        /*tp_str*/
    0,                         /*tp_getattro*/
    0,                         /*tp_setattro*/
    0,                         /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/
    "PGF object",              /*tp_doc*/
    0,                           /*tp_traverse */
    0,                           /*tp_clear */
    0,                           /*tp_richcompare */
    0,                           /*tp_weaklistoffset */
    0,                           /*tp_iter */
    0,                           /*tp_iternext */
    PGF_methods,               /*tp_methods */
    PGF_members,               /*tp_members */
    PGF_getseters,             /*tp_getset */
    0,                         /*tp_base */
    0,                         /*tp_dict */
    0,                         /*tp_descr_get */
    0,                         /*tp_descr_set */
    0,                         /*tp_dictoffset */
    0,                         /*tp_init */
    0,                         /*tp_alloc */
    0,                         /*tp_new */
};

// ----------------------------------------------------------------------------
// pgf: the module

static PGFObject *
pgf_readPGF(PyObject *self, PyObject *args)
{
    const char *fpath;
    if (!PyArg_ParseTuple(args, "s", &fpath))
        return NULL;

    PGFObject *py_pgf = (PGFObject *)pgf_PGFType.tp_alloc(&pgf_PGFType, 0);

    PgfExn err;
    py_pgf->db = pgf_read_pgf(fpath, &py_pgf->revision, &err);
    if (handleError(err) != PGF_EXN_NONE) {
        Py_DECREF(py_pgf);
        return NULL;
    }

    return py_pgf;
}

static PGFObject *
pgf_bootNGF(PyObject *self, PyObject *args)
{
    const char *fpath; // pgf
    const char *npath; // ngf
    if (!PyArg_ParseTuple(args, "ss", &fpath, &npath))
        return NULL;

    PGFObject *py_pgf = (PGFObject *)pgf_PGFType.tp_alloc(&pgf_PGFType, 0);

    PgfExn err;
    py_pgf->db = pgf_boot_ngf(fpath, npath, &py_pgf->revision, &err);
    if (handleError(err) != PGF_EXN_NONE) {
        Py_DECREF(py_pgf);
        return NULL;
    }

    return py_pgf;
}

static PGFObject *
pgf_readNGF(PyObject *self, PyObject *args)
{
    const char *fpath;
    if (!PyArg_ParseTuple(args, "s", &fpath))
        return NULL;

    PGFObject *py_pgf = (PGFObject *)pgf_PGFType.tp_alloc(&pgf_PGFType, 0);

    PgfExn err;
    py_pgf->db = pgf_read_ngf(fpath, &py_pgf->revision, &err);
    if (handleError(err) != PGF_EXN_NONE) {
        Py_DECREF(py_pgf);
        return NULL;
    }

    return py_pgf;
}

static PGFObject *
pgf_newNGF(PyObject *self, PyObject *args)
{
    const char *s;
    Py_ssize_t size;
    const char *fpath = NULL;
    if (!PyArg_ParseTuple(args, "s#|s", &s, &size, &fpath))
        return NULL;

    PgfText *absname = CString_AsPgfText(s, size);

    PGFObject *py_pgf = (PGFObject *)pgf_PGFType.tp_alloc(&pgf_PGFType, 0);

    PgfExn err;
    py_pgf->db = pgf_new_ngf(absname, fpath, &py_pgf->revision, &err);
    FreePgfText(absname);
    if (handleError(err) != PGF_EXN_NONE) {
        Py_DECREF(py_pgf);
        return NULL;
    }

    return py_pgf;
}

static ExprObject *
pgf_readExpr(PyObject *self, PyObject *args)
{
    const char *s;
    Py_ssize_t size;
    if (!PyArg_ParseTuple(args, "s#", &s, &size))
        return NULL;

    PgfText *input = CString_AsPgfText(s, size);

    PgfExpr expr = pgf_read_expr(input, &unmarshaller);
    FreePgfText(input);
    if (expr == 0) {
        PyErr_SetString(PGFError, "expression cannot be parsed");
        return NULL;
    }

    return (ExprObject *)expr;
}

static PyObject *
pgf_showExpr(PyObject *self, PyObject *args)
{
    PyObject *pylist;
    ExprObject *expr;
    if (!PyArg_ParseTuple(args, "O!O!", &PyList_Type, &pylist, &pgf_ExprType, &expr))
        return NULL;

    PgfPrintContext *ctxt = PyList_AsPgfPrintContext(pylist);
    PgfText *s = pgf_print_expr((PgfExpr) expr, ctxt, 0, &marshaller);
    FreePgfPrintContext(ctxt);
    PyObject *str = PyUnicode_FromStringAndSize(s->text, s->size);
    FreePgfText(s);
    return str;
}

static TypeObject *
pgf_readType(PyObject *self, PyObject *args)
{
    const char *s;
    Py_ssize_t size;
    if (!PyArg_ParseTuple(args, "s#", &s, &size))
        return NULL;

    PgfText *input = CString_AsPgfText(s, size);

    PgfType type = pgf_read_type(input, &unmarshaller);
    FreePgfText(input);
    if (type == 0) {
        PyErr_SetString(PGFError, "type cannot be parsed");
        return NULL;
    }

    return (TypeObject *)type;
}

static PyObject *
pgf_showType(PyObject *self, PyObject *args)
{
    PyObject *pylist;
    TypeObject *type;
    if (!PyArg_ParseTuple(args, "O!O!", &PyList_Type, &pylist, &pgf_TypeType, &type))
        return NULL;

    PgfPrintContext *ctxt = PyList_AsPgfPrintContext(pylist);
    PgfText *s = pgf_print_type((PgfType) type, ctxt, 0, &marshaller);
    FreePgfPrintContext(ctxt);
    PyObject *str = PyUnicode_FromStringAndSize(s->text, s->size);
    FreePgfText(s);
    return str;
}

static PyObject *
pgf_mkHypo(PyObject *self, PyObject *args)
{
    PyObject *type;
    if (!PyArg_ParseTuple(args, "O!", &pgf_TypeType, &type))
        return NULL;

    // HypoObject *hypo = PyObject_New(HypoObject, &pgf_HypoType);
    // hypo->bind_type = Py_True; // explicit
    // hypo->cid = PyUnicode_FromStringAndSize("_", 1);
    // hypo->type = type;
    // Py_INCREF(hypo->bind_type);
    // Py_INCREF(hypo->cid);
    // Py_INCREF(hypo->type);
    // return hypo;

    PyObject *tup = PyTuple_New(3);
    PyTuple_SetItem(tup, 0, Py_True); // explicit
    PyTuple_SetItem(tup, 1, PyUnicode_FromStringAndSize("_", 1));
    PyTuple_SetItem(tup, 2, type);
    Py_INCREF(Py_True);
    Py_INCREF(type);
    return tup;
}

static PyObject *
pgf_mkDepHypo(PyObject *self, PyObject *args)
{
    PyObject *var;
    PyObject *type;
    if (!PyArg_ParseTuple(args, "UO!", &var, &pgf_TypeType, &type))
        return NULL;

    // HypoObject *hypo = PyObject_New(HypoObject, &pgf_HypoType);
    // hypo->bind_type = Py_True; // explicit
    // hypo->cid = var;
    // hypo->type = type;
    // Py_INCREF(hypo->bind_type);
    // Py_INCREF(hypo->cid);
    // Py_INCREF(hypo->type);
    // return hypo;

    PyObject *tup = PyTuple_New(3);
    PyTuple_SetItem(tup, 0, Py_True); // explicit
    PyTuple_SetItem(tup, 1, var);
    PyTuple_SetItem(tup, 2, type);
    Py_INCREF(Py_True);
    Py_INCREF(var);
    Py_INCREF(type);
    return tup;
}

static PyObject *
pgf_mkImplHypo(PyObject *self, PyObject *args)
{
    PyObject *var;
    PyObject *type;
    if (!PyArg_ParseTuple(args, "UO!", &var, &pgf_TypeType, &type))
        return NULL;

    // HypoObject *hypo = PyObject_New(HypoObject, &pgf_HypoType);
    // hypo->bind_type = Py_False; // implicit
    // hypo->cid = var;
    // hypo->type = type;
    // Py_INCREF(hypo->bind_type);
    // Py_INCREF(hypo->cid);
    // Py_INCREF(hypo->type);
    // return hypo;

    PyObject *tup = PyTuple_New(3);
    PyTuple_SetItem(tup, 0, Py_False); // implicit
    PyTuple_SetItem(tup, 1, var);
    PyTuple_SetItem(tup, 2, type);
    Py_INCREF(Py_True);
    Py_INCREF(var);
    Py_INCREF(type);
    return tup;
}

static PyMethodDef module_methods[] = {
    {"readPGF",  (void*)pgf_readPGF,  METH_VARARGS,
     "Reads a PGF file into memory"},
    {"bootNGF",  (void*)pgf_bootNGF,  METH_VARARGS,
     "Reads a PGF file into memory and stores the unpacked data in an NGF file"},
    {"readNGF",  (void*)pgf_readNGF,  METH_VARARGS,
     "Reads an NGF file into memory"},
    {"newNGF",   (void*)pgf_newNGF,  METH_VARARGS,
     "Creates a new NGF file with the given name"},

    {"readExpr", (void*)pgf_readExpr, METH_VARARGS,
     "Parses a string as an abstract tree"},
    {"showExpr", (void*)pgf_showExpr, METH_VARARGS,
     "Renders an expression as a string"},
    {"readType", (void*)pgf_readType, METH_VARARGS,
     "Parses a string as an abstract type"},
    {"showType", (void*)pgf_showType, METH_VARARGS,
     "Renders a type as a string"},

    {"mkHypo", (void*)pgf_mkHypo, METH_VARARGS,
     "Creates hypothesis for non-dependent type i.e. A"},
    {"mkDepHypo", (void*)pgf_mkDepHypo, METH_VARARGS,
     "Creates hypothesis for dependent type i.e. (x : A)"},
    {"mkImplHypo", (void*)pgf_mkImplHypo, METH_VARARGS,
     "Creates hypothesis for dependent type with implicit argument i.e. ({x} : A)"},
    {NULL, NULL, 0, NULL}        /* Sentinel */
};

#define MOD_ERROR_VAL NULL
#define MOD_SUCCESS_VAL(val) val
#define MOD_INIT(name) PyMODINIT_FUNC PyInit_##name(void)
#define MOD_DEF(ob, name, doc, methods) \
    static struct PyModuleDef moduledef = { \
        PyModuleDef_HEAD_INIT, name, doc, -1, methods, }; \
        ob = PyModule_Create(&moduledef);

#define TYPE_READY(type) \
    if (PyType_Ready(&type) < 0) \
        return MOD_ERROR_VAL;

#define ADD_TYPE(name, type) \
    Py_INCREF(&type); \
    if (PyModule_AddObject(m, name, (PyObject *)&type) < 0) { \
        Py_DECREF(&type); \
        Py_DECREF(m); \
        return NULL; \
    }

#define ADD_TYPE_DIRECT(name, type) \
    Py_INCREF(type); \
    if (PyModule_AddObject(m, name, (PyObject *)type) < 0) { \
        Py_DECREF(type); \
        Py_DECREF(m); \
        return NULL; \
    }

MOD_INIT(pgf)
{
    PyObject *m;

    TYPE_READY(pgf_PGFType);
    TYPE_READY(pgf_TransactionType);
    TYPE_READY(pgf_ExprType);
    TYPE_READY(pgf_ExprAbsType);
    TYPE_READY(pgf_ExprAppType);
    TYPE_READY(pgf_ExprLitType);
    TYPE_READY(pgf_ExprMetaType);
    TYPE_READY(pgf_ExprFunType);
    TYPE_READY(pgf_ExprVarType);
    TYPE_READY(pgf_ExprTypedType);
    TYPE_READY(pgf_ExprImplArgType);
    TYPE_READY(pgf_TypeType);
    // TYPE_READY(pgf_HypoType);

    MOD_DEF(m, "pgf", "The Runtime for Portable Grammar Format in Python", module_methods);
    if (m == NULL)
        return MOD_ERROR_VAL;

    PGFError = PyErr_NewException("pgf.PGFError", NULL, NULL);
    ADD_TYPE_DIRECT("PGFError", PGFError);

    ADD_TYPE("PGF", pgf_PGFType);
    ADD_TYPE("Transaction", pgf_TransactionType);
    ADD_TYPE("Expr", pgf_ExprType);
    ADD_TYPE("ExprAbs", pgf_ExprAbsType);
    ADD_TYPE("ExprApp", pgf_ExprAppType);
    ADD_TYPE("ExprLit", pgf_ExprLitType);
    ADD_TYPE("ExprMeta", pgf_ExprMetaType);
    ADD_TYPE("ExprFun", pgf_ExprFunType);
    ADD_TYPE("ExprVar", pgf_ExprVarType);
    ADD_TYPE("ExprTyped", pgf_ExprTypedType);
    ADD_TYPE("ExprImplArg", pgf_ExprImplArgType);
    ADD_TYPE("Type", pgf_TypeType);
    // ADD_TYPE("Hypo", pgf_HypoType);

    ADD_TYPE_DIRECT("BIND_TYPE_EXPLICIT", Py_True);
    ADD_TYPE_DIRECT("BIND_TYPE_IMPLICIT", Py_False);

    return MOD_SUCCESS_VAL(m);
}
