#include "pybind11_qt_basic.h"

#include <type_traits>

#include <pybind11/stl/filesystem.h>

#include "details/pybind11_qt_utils.h"

namespace pybind11::detail {

    template <class CharT>
    QString qstring_from_stdstring(std::basic_string<CharT> const& s)
    {
        if constexpr (std::is_same_v<CharT, char>) {
            return QString::fromStdString(s);
        }
        else if constexpr (std::is_same_v<CharT, wchar_t>) {
            return QString::fromStdWString(s);
        }
        else if constexpr (std::is_same_v<CharT, char16_t>) {
            return QString::fromStdU16String(s);
        }
        else if constexpr (std::is_same_v<CharT, char32_t>) {
            return QString::fromStdU32String(s);
        }
    }

    /**
     * Conversion part 1 (Python->C++): convert a PyObject into a QString
     * instance or return false upon failure. The second argument
     * indicates whether implicit conversions should be applied.
     */
    bool type_caster<QString>::load(handle src, bool implicit)
    {

        PyObject* objPtr = src.ptr();

        if (!PyBytes_Check(objPtr) && !PyUnicode_Check(objPtr)) {
#ifndef PYBIND11_QT_QSTRING_NOFS
            // try converting from os.PathLike
            type_caster<std::filesystem::path> path_caster;
            if (!path_caster.load(src, implicit)) {
                return false;
            }

            value = qstring_from_stdstring((*path_caster).native());
            return true;
#else
            // do not try to convert from os.PathLike
            return false;
#endif
        }

        // Ensure the string uses 8-bit characters
        PyObject* strPtr =
            PyUnicode_Check(objPtr) ? PyUnicode_AsUTF8String(objPtr) : objPtr;

        // Extract the character data from the python string
        value = QString::fromUtf8(PyBytes_AsString(strPtr));

        // Deallocate local copy if one was made
        if (strPtr != objPtr) {
            Py_DecRef(strPtr);
        }

        return true;
    }

    /**
     * Conversion part 2 (C++ -> Python): convert an QString instance into
     * a Python object. The second and third arguments are used to
     * indicate the return value policy and parent object (for
     * ``return_value_policy::reference_internal``) and are generally
     * ignored by implicit casters.
     */
    handle type_caster<QString>::cast(QString src, return_value_policy /* policy */,
                                      handle /* parent */)
    {
        static_assert(sizeof(QChar) == 2);
        return PyUnicode_FromKindAndData(PyUnicode_2BYTE_KIND, src.constData(),
                                         src.length());
    }

    bool type_caster<QVariant>::load(handle src, bool implicit)
    {
        if (PyList_Check(src.ptr())) {
            // we could check if all the elements can be converted to QString
            // and store a QStringList in the QVariant but I am not sure that is
            // really useful.
            value = src.cast<QVariantList>();
            return true;
        }
        else if (src == Py_None) {
            value = QVariant();
            return true;
        }
        else if (PyDict_Check(src.ptr())) {
            value = src.cast<QVariantMap>();
            return true;
        }
        else if (PyBytes_Check(src.ptr()) || PyUnicode_Check(src.ptr())) {
            value = src.cast<QString>();
            return true;
        }
        // PyBool will also return true for PyLong_Check but not the other way
        // around, so the order here is relevant.
        else if (PyBool_Check(src.ptr())) {
            value = src.cast<bool>();
            return true;
        }
        else if (PyLong_Check(src.ptr())) {
            // QVariant doesn't have long. It has int or long long. Given that
            // on m/s, long is 32 bits for 32- and 64- bit code...
            value = src.cast<int>();
            return true;
        }
        else {
            return false;
        }
    }

    handle type_caster<QVariant>::cast(QVariant var, return_value_policy policy,
                                       handle parent)
    {
        switch (var.type()) {
        case QVariant::Invalid:
            return Py_None;
        case QVariant::Int:
            return PyLong_FromLong(var.toInt());
        case QVariant::UInt:
            return PyLong_FromUnsignedLong(var.toUInt());
        case QVariant::Bool:
            return PyBool_FromLong(var.toBool());
        case QVariant::String:
            return type_caster<QString>::cast(var.toString(), policy, parent);
        // We need to check for StringList here because these are not considered
        // List since List is QList<QVariant> will StringList is QList<QString>:
        case QVariant::StringList:
            return type_caster<QStringList>::cast(var.toStringList(), policy, parent);
        case QVariant::List:
            return type_caster<QVariantList>::cast(var.toList(), policy, parent);
        case QVariant::Map:  // return
                             // type_caster<QVariantMap>::cast(var.toList(),
                             // policy, parent);
        default: {
            PyErr_Format(PyExc_TypeError, "type unsupported: %d", var.type());
            throw pybind11::error_already_set();
        }
        }
    }

}  // namespace pybind11::detail
