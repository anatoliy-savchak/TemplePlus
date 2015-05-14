#include "stdafx.h"
#include "../tio/tio.h"
#include "../tig/tig_mes.h"

#include <Rocket/Core/FontDatabase.h>

/*
	Reads a file using tio and returns it's content.
*/
static PyObject* PyTioRead(PyObject*, PyObject* args) {
	char* filename;
	int asText = 0;
	if (!PyArg_ParseTuple(args, "s|i:tio_read", &filename, &asText)) {
		return 0;
	}

	auto fh = tio_fopen(filename, asText ? "rt" : "rb");
	if (!fh) {
		PyErr_Format(PyExc_IOError, "Could not find file %s", filename);
		return 0;
	}

	auto size = tio_filelength(fh);
	if (size < 0) {
		tio_fclose(fh);
		PyErr_Format(PyExc_IOError, "Could not determine size of file %s", filename);
		return 0;
	}

	auto result = PyString_FromStringAndSize(nullptr, size);
	if (!result) {
		tio_fclose(fh);
		PyErr_SetString(PyExc_MemoryError, "Unable to allocate a buffer for reading.");
		return 0;
	}

	char* buffer;
	Py_ssize_t bufferSize;
	if (PyString_AsStringAndSize(result, &buffer, &bufferSize) != 0) {
		tio_fclose(fh);
		PyErr_SetString(PyExc_MemoryError, "Unable to get buffer for reading.");
		Py_DECREF(result);
		return 0;
	}

	assert(bufferSize == size);

	if (tio_fread(buffer, 1, bufferSize, fh) != bufferSize) {
		tio_fclose(fh);
		PyErr_SetString(PyExc_IOError, "Unable to read the full file.");
		Py_DECREF(result);
		return 0;
	}

	tio_fclose(fh);
	return result;
}

static PyObject* PyReadMes(PyObject*, PyObject* arg) {

	auto filename = PyString_AsString(arg);
	if (!filename) {
		return 0;
	}

	MesFile mesFile(filename);

	auto result = PyDict_New();

	auto size = mesFile.size();
	for (auto i = 0; i < size; ++i) {
		uint32_t key;
		const char* value;
		if (mesFile.GetLineAt(i, key, value)) {
			auto pykey = PyInt_FromLong(key);
			auto pyvalue = PyString_FromString(value);
			auto res = PyDict_SetItem(result, pykey, pyvalue);
			Py_DECREF(pykey);
			Py_DECREF(pyvalue);

			if (res != 0) {
				Py_DECREF(result);
				return 0;
			}
		}
	}

	return result;
}

static PyObject* PyAddFont(PyObject*, PyObject* args, PyObject *kwargs) {

	char* filename;
	char* family;
	int italic = 0;
	int bold = 0;
	static char *kwlist[] = {
		"filename",
		"family",
		"italic",
		"bold",
		NULL
	};
	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "ss|ii:_templeplus.add_font", kwlist,
		&filename, &family, &italic, &bold)) {
		return 0;
	}

	logger->debug("Loading font from file {} (Family: {}, Italic: {}, Bold: {})", filename, family, italic, bold);

	auto style = italic ? Rocket::Core::Font::STYLE_ITALIC : Rocket::Core::Font::STYLE_NORMAL;
	auto weight = bold ? Rocket::Core::Font::WEIGHT_BOLD : Rocket::Core::Font::WEIGHT_NORMAL;

	if (!Rocket::Core::FontDatabase::LoadFontFace(filename, family, style, weight)) {
		PyErr_Format(
			PyExc_RuntimeError,
			"Unable to load font %s with family: %s, italic: %d, bold: %d",
			filename, family, italic, bold
		);
		return 0;
	}

	Py_RETURN_NONE;
}

static PyMethodDef methods[] = {
	{"tio_read", PyTioRead, METH_VARARGS, NULL},
	{"read_mes", PyReadMes, METH_O, NULL},
	{"add_font", (PyCFunction)PyAddFont, METH_VARARGS|METH_KEYWORDS, NULL},
	{NULL, NULL, NULL, NULL}
};

PyMODINIT_FUNC init_templeplus() {
	Py_InitModule("_templeplus", methods);
}
