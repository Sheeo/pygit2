/*
 * Copyright 2010-2015 The pygit2 contributors
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2,
 * as published by the Free Software Foundation.
 *
 * In addition to the permissions in the GNU General Public License,
 * the authors give you unlimited permission to link the compiled
 * version of this file into combinations with other programs,
 * and to distribute those combinations without any restriction
 * coming from the use of this file.  (The General Public License
 * restrictions do apply in other respects; for example, they cover
 * modification of the file, and distribution when not linked into
 * a combined executable.)
 *
 * This file is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include "error.h"
#include "types.h"
#include "utils.h"
#include "oid.h"
#include "signature.h"

int
Signature_init(Signature *self, PyObject *args, PyObject *kwds)
{
    char *keywords[] = {"name", "email", "time", "offset", "encoding", NULL};
    PyObject *py_name, *tname;
    char *email, *encoding = "utf-8";
    const char *name;
    long long time = -1;
    int offset = 0;
    int err;
    git_signature *signature;

    if (!PyArg_ParseTupleAndKeywords(
            args, kwds, "Os|Lis", keywords,
            &py_name, &email, &time, &offset, &encoding))
        return -1;

    name = py_str_borrow_c_str(&tname, py_name, encoding);
    if (name == NULL)
        return -1;

    if (time == -1) {
        err = git_signature_now(&signature, name, email);
    } else {
        err = git_signature_new(&signature, name, email, time, offset);
    }
    Py_DECREF(tname);

    if (err < 0) {
        Error_set(err);
        return -1;
    }

    self->obj = NULL;
    self->signature = signature;

    if (encoding) {
        self->encoding = strdup(encoding);
        if (self->encoding == NULL) {
            PyErr_NoMemory();
            return -1;
        }
    }

    return 0;
}

void
Signature_dealloc(Signature *self)
{
    /* self->obj is the owner of the git_signature, so we musn't free it */
    if (self->obj) {
        Py_CLEAR(self->obj);
    } else {
        git_signature_free((git_signature *) self->signature);
        free(self->encoding);
    }

    PyObject_Del(self);
}

PyDoc_STRVAR(Signature__pointer__doc__, "Get the signature's pointer. For internal use only.");
PyObject *
Signature__pointer__get__(Repository *self)
{
    /* Bytes means a raw buffer */
    return PyBytes_FromStringAndSize((char *) &self->repo, sizeof(git_repository *));
}

PyDoc_STRVAR(Signature__encoding__doc__, "Encoding.");

PyObject *
Signature__encoding__get__(Signature *self)
{
    const char *encoding;

    encoding = self->encoding;
    if (encoding == NULL)
        encoding = "utf-8";

    return to_encoding(encoding);
}


PyDoc_STRVAR(Signature_raw_name__doc__, "Name (bytes).");

PyObject *
Signature_raw_name__get__(Signature *self)
{
    return to_bytes(self->signature->name);
}


PyDoc_STRVAR(Signature_raw_email__doc__, "Email (bytes).");

PyObject *
Signature_raw_email__get__(Signature *self)
{
    return to_bytes(self->signature->email);
}


PyDoc_STRVAR(Signature_name__doc__, "Name.");

PyObject *
Signature_name__get__(Signature *self)
{
    return to_unicode(self->signature->name, self->encoding, "strict");
}


PyDoc_STRVAR(Signature_email__doc__, "Email address.");

PyObject *
Signature_email__get__(Signature *self)
{
    return to_unicode(self->signature->email, self->encoding, "strict");
}


PyDoc_STRVAR(Signature_time__doc__, "Unix time.");

PyObject *
Signature_time__get__(Signature *self)
{
    return PyLong_FromLongLong(self->signature->when.time);
}


PyDoc_STRVAR(Signature_offset__doc__, "Offset from UTC in minutes.");

PyObject *
Signature_offset__get__(Signature *self)
{
    return PyLong_FromLong(self->signature->when.offset);
}

PyGetSetDef Signature_getseters[] = {
    GETTER(Signature, _encoding),
    GETTER(Signature, raw_name),
    GETTER(Signature, raw_email),
    GETTER(Signature, name),
    GETTER(Signature, email),
    GETTER(Signature, time),
    GETTER(Signature, offset),
    GETTER(Signature, _pointer),
    {NULL}
};


PyDoc_STRVAR(Signature__doc__, "Signature.");

PyTypeObject SignatureType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "_pygit2.Signature",                       /* tp_name           */
    sizeof(Signature),                         /* tp_basicsize      */
    0,                                         /* tp_itemsize       */
    (destructor)Signature_dealloc,             /* tp_dealloc        */
    0,                                         /* tp_print          */
    0,                                         /* tp_getattr        */
    0,                                         /* tp_setattr        */
    0,                                         /* tp_compare        */
    0,                                         /* tp_repr           */
    0,                                         /* tp_as_number      */
    0,                                         /* tp_as_sequence    */
    0,                                         /* tp_as_mapping     */
    0,                                         /* tp_hash           */
    0,                                         /* tp_call           */
    0,                                         /* tp_str            */
    0,                                         /* tp_getattro       */
    0,                                         /* tp_setattro       */
    0,                                         /* tp_as_buffer      */
    Py_TPFLAGS_DEFAULT,                        /* tp_flags          */
    Signature__doc__,                          /* tp_doc            */
    0,                                         /* tp_traverse       */
    0,                                         /* tp_clear          */
    0,                                         /* tp_richcompare    */
    0,                                         /* tp_weaklistoffset */
    0,                                         /* tp_iter           */
    0,                                         /* tp_iternext       */
    0,                                         /* tp_methods        */
    0,                                         /* tp_members        */
    Signature_getseters,                       /* tp_getset         */
    0,                                         /* tp_base           */
    0,                                         /* tp_dict           */
    0,                                         /* tp_descr_get      */
    0,                                         /* tp_descr_set      */
    0,                                         /* tp_dictoffset     */
    (initproc)Signature_init,                  /* tp_init           */
    0,                                         /* tp_alloc          */
    0,                                         /* tp_new            */
};

PyObject *
build_signature(Object *obj, const git_signature *signature,
                const char *encoding)
{
    Signature *py_signature;

    py_signature = PyObject_New(Signature, &SignatureType);
    if (!py_signature)
        goto on_error;

    py_signature->encoding = NULL;
    if (encoding) {
        py_signature->encoding = strdup(encoding);
        if (!py_signature->encoding)
            goto on_error;
    }

    Py_XINCREF(obj);
    py_signature->obj = obj;
    py_signature->signature = signature;

    return (PyObject*)py_signature;

on_error:
    git_signature_free((git_signature *) signature);
    return NULL;
}
