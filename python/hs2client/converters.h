// Copyright 2016 Cloudera Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// Helper functions for converting from hs2client::ColumnarRowSet to other
// Python-compatible data structures

#ifndef HS2CLIENT_PYTHON_HELPER_H
#define HS2CLIENT_PYTHON_HELPER_H

#include <Python.h>

#include <cmath>
#include <cstdint>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include <numpy/arrayobject.h>

#include "hs2client/api.h"

namespace hs2client {

namespace py {

static constexpr uint8_t BITMASK[] = {1, 2, 4, 8, 16, 32, 64, 128};

using TypeId = ColumnType::TypeId;
using std::unique_ptr;
using std::vector;

static inline bool GetBit(const uint8_t* bits, int i) {
  return static_cast<bool>(bits[i / 8] & BITMASK[i % 8]);
}

#define RETURN_IF_NULL(_X_)                     \
  do {                                          \
    if (_X_ == nullptr) {                       \
      return nullptr;                           \
    }                                           \
  } while (0)

#define TRY_NPY_ALLOC(VARNAME, NPY_TYPE, LENGTH)        \
  do {                                                  \
    npy_intp dims[1] = {static_cast<npy_intp>(LENGTH)}; \
    VARNAME = PyArray_SimpleNew(1, dims, NPY_TYPE);     \
    RETURN_IF_NULL(VARNAME);                            \
  } while (0)

// Returns the total length of the columns
template <typename T>
static int64_t GetColumns(const std::vector<ColumnarRowSet*>& batches,
    int col_index, vector<unique_ptr<T>>* columns) {
  int64_t total_length = 0;
  for (size_t i = 0; i < batches.size(); ++i) {
    auto col = batches[i]->GetCol<T>(col_index);
    total_length += col->length();
    columns->push_back(std::move(col));
  }
  return total_length;
}

template <int NPY_TYPE, typename CType, typename T>
static PyObject* ConvertInteger(const std::vector<ColumnarRowSet*>& batches,
    int col_index) {
  vector<unique_ptr<CType>> columns;
  int64_t total_length = GetColumns(batches, col_index, &columns);

  // Optimistically we'll try to write into integer until a null is encountered
  PyObject* out;
  TRY_NPY_ALLOC(out, NPY_TYPE, total_length);

  T* out_values = reinterpret_cast<T*>(PyArray_DATA(out));

  const CType* col;
  const T* col_data;
  const uint8_t* nulls;

  int64_t i = 0;
  bool have_null = false;
  for (size_t chunk = 0; chunk < columns.size(); ++chunk) {
    col = columns[chunk].get();
    col_data = col->data().data();
    nulls = col->nulls();
    for (int j = 0; j < col->length(); ++j) {
      if (GetBit(nulls, j)) {
        have_null = true;
        break;
      }
      out_values[i++] = col_data[j];
    }

    if (have_null) break;
  }

  if (!have_null) {
    // Successful, no nulls
    return out;
  }

  // There were nulls, delete the array and instead write to NPY_DOUBLE
  Py_DECREF(out);

  TRY_NPY_ALLOC(out, NPY_DOUBLE, total_length);

  double* doubles = reinterpret_cast<double*>(PyArray_DATA(out));
  i = 0;
  for (size_t chunk = 0; chunk < columns.size(); ++chunk) {
    col = columns[chunk].get();
    col_data = col->data().data();
    nulls = col->nulls();
    for (int j = 0; j < col->length(); ++j) {
      doubles[i++] = GetBit(nulls, j) ? NAN : col_data[j];
    }
  }

  return out;
}

static PyObject* ConvertBoolean(const std::vector<ColumnarRowSet*>& batches,
    int col_index) {
  vector<unique_ptr<BoolColumn>> columns;
  int64_t total_length = GetColumns(batches, col_index, &columns);

  PyObject* out;
  TRY_NPY_ALLOC(out, NPY_BOOL, total_length);

  uint8_t* out_values = reinterpret_cast<uint8_t*>(PyArray_DATA(out));

  const BoolColumn* col;
  const uint8_t* nulls;

  int64_t i = 0;
  bool have_null = false;
  for (size_t chunk = 0; chunk < columns.size(); ++chunk) {
    col = columns[chunk].get();
    const std::vector<bool>& col_data = col->data();
    nulls = col->nulls();
    for (int j = 0; j < col->length(); ++j) {
      if (GetBit(nulls, j)) {
        have_null = true;
        break;
      }
      out_values[i++] = static_cast<uint8_t>(col_data[j]);
    }
    if (have_null) break;
  }

  if (!have_null) {
    // Successful, no nulls
    return out;
  }

  // There were nulls, delete the array and instead write to NPY_DOUBLE
  Py_DECREF(out);
  TRY_NPY_ALLOC(out, NPY_OBJECT, total_length);

  PyObject** objects = reinterpret_cast<PyObject**>(PyArray_DATA(out));
  i = 0;
  for (size_t chunk = 0; chunk < columns.size(); ++chunk) {
    col = columns[chunk].get();
    const std::vector<bool>& col_data = col->data();
    nulls = col->nulls();
    for (int j = 0; j < col->length(); ++j) {
      if (GetBit(nulls, j)) {
        Py_INCREF(Py_None);
        objects[i++] = Py_None;
      } else if (col_data[j]) {
        Py_INCREF(Py_True);
        objects[i++] = Py_True;
      } else {
        Py_INCREF(Py_False);
        objects[i++] = Py_False;
      }
    }
  }

  return out;
}

static inline PyObject* make_pystring(const char* data, int32_t length,
    PyObject* intern_table) {
  PyObject* str;
#if PY_MAJOR_VERSION >= 3
  str = PyUnicode_FromStringAndSize(data, length);
#else
  str = PyString_FromStringAndSize(data, length);
#endif

  // probably out of memory, Python sets exception
  RETURN_IF_NULL(str);

  // TODO: Consider a more performant hash table pass that circumvents Python
  // object indirection

  PyObject* interned = PyDict_GetItem(intern_table, str);
  if (interned == NULL) {
    // Not in the dict
    int ret = PyDict_SetItem(intern_table, str, str);
    if (ret != 0) return nullptr;

    // The reference count of str is 1
    return str;
  } else {
    // We maintain a reference to the interned string value
    Py_DECREF(str);
    Py_INCREF(interned);
    return interned;
  }
}

static PyObject* ConvertString(const std::vector<ColumnarRowSet*>& batches,
    int col_index) {
  vector<unique_ptr<StringColumn>> columns;
  int64_t total_length = GetColumns(batches, col_index, &columns);

  PyObject* out;
  TRY_NPY_ALLOC(out, NPY_OBJECT, total_length);

  PyObject** out_values = reinterpret_cast<PyObject**>(PyArray_DATA(out));

  const StringColumn* col;
  const uint8_t* nulls;

  PyObject* intern_table = PyDict_New();
  RETURN_IF_NULL(intern_table);

  PyObject* out_string;

  int64_t i = 0;
  for (size_t chunk = 0; chunk < columns.size(); ++chunk) {
    col = columns[chunk].get();
    const std::vector<std::string>& col_data = col->data();
    nulls = col->nulls();
    for (int j = 0; j < col->length(); ++j) {
      if (GetBit(nulls, j)) {
        Py_INCREF(Py_None);
        out_values[i++] = Py_None;
      } else {
        out_string = make_pystring(col_data[j].c_str(), col_data[j].size(),
            intern_table);
        RETURN_IF_NULL(out_string);
        out_values[i++] = out_string;
      }
    }
  }

  Py_DECREF(intern_table);

  return out;
}

template <int NPY_TYPE, typename CType, typename IN_TYPE, typename OUT_TYPE>
static PyObject* ConvertFloat(const std::vector<ColumnarRowSet*>& batches,
    int col_index) {
  vector<unique_ptr<CType>> columns;
  int64_t total_length = GetColumns(batches, col_index, &columns);

  PyObject* out;
  TRY_NPY_ALLOC(out, NPY_TYPE, total_length);

  OUT_TYPE* out_values = reinterpret_cast<OUT_TYPE*>(PyArray_DATA(out));

  const CType* col;
  const IN_TYPE* col_data;
  const uint8_t* nulls;

  OUT_TYPE null_value = static_cast<OUT_TYPE>(NAN);

  int64_t i = 0;
  for (size_t chunk = 0; chunk < columns.size(); ++chunk) {
    col = columns[chunk].get();
    col_data = col->data().data();
    nulls = col->nulls();
    for (int j = 0; j < col->length(); ++j) {
      out_values[i++] = GetBit(nulls, j) ? null_value : col_data[j];
    }
  }
  return out;
}

PyObject* ConvertColumnPandas(const std::vector<ColumnarRowSet*>& batches,
    int col_index, const ColumnType* type) {
  switch (type->type_id()) {
    case TypeId::BOOLEAN:
      return ConvertBoolean(batches, col_index);
    case TypeId::TINYINT:
      return ConvertInteger<NPY_INT8, ByteColumn, int8_t>(batches, col_index);
    case TypeId::SMALLINT:
      return ConvertInteger<NPY_INT16, Int16Column, int16_t>(batches, col_index);
    case TypeId::INT:
      return ConvertInteger<NPY_INT32, Int32Column, int32_t>(batches, col_index);
    case TypeId::BIGINT:
      return ConvertInteger<NPY_INT64, Int64Column, int64_t>(batches, col_index);
    case TypeId::FLOAT:
      // float32 data is transported in doubles on the wire
      return ConvertFloat<NPY_FLOAT32, DoubleColumn, double, float>(batches, col_index);
    case TypeId::DOUBLE:
      return ConvertFloat<NPY_DOUBLE, DoubleColumn, double, double>(batches, col_index);
    case TypeId::STRING:
      // TODO(wesm): Unicode encodings
      return ConvertString(batches, col_index);
    case TypeId::TIMESTAMP:
      // TODO(wesm): HS2 presents timestamps as ISO-8601-ish strings. Will
      // leave it to pandas to convert
      return ConvertString(batches, col_index);
    case TypeId::BINARY:
    case TypeId::ARRAY:
    case TypeId::MAP:
    case TypeId::STRUCT:
    case TypeId::UNION:
    case TypeId::USER_DEFINED:
    case TypeId::DECIMAL:
      return ConvertString(batches, col_index);
    case TypeId::NULL_TYPE:
    case TypeId::DATE:
      {
        const std::string name = type->ToString();
        PyErr_SetString(PyExc_NotImplementedError, name.c_str());
      }
      break;
    case TypeId::VARCHAR:
    case TypeId::CHAR:
      return ConvertString(batches, col_index);
    case TypeId::INVALID:
      {
        const std::string name = type->ToString();
        PyErr_SetString(PyExc_NotImplementedError, name.c_str());
      }
      break;
    default:
      PyErr_SetString(PyExc_NotImplementedError, "Unknown type");
      break;
  }

  return nullptr;
}

}  // namespace py

}  // namespace hs2client

#endif // HS2CLIENT_PYTHON_HELPER_H
