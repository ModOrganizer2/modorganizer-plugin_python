#include "pybind11_qt/pybind11_qt_basic.h"

#include <type_traits>

#include <QColor>
#include <QIcon>
#include <QPixmap>

#include <pybind11/stl/filesystem.h>

#include "pybind11_qt/details/pybind11_qt_utils.h"

// need to import containers to get QVariantList and QVariantMap
#include "pybind11_qt/pybind11_qt_containers.h"
#include "pybind11_qt/pybind11_qt_objects.h"

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
    bool type_caster<QString>::load(handle src, bool)
    {
        PyObject* objPtr = src.ptr();

        if (PyBytes_Check(objPtr)) {
            value = QString::fromUtf8(PyBytes_AsString(objPtr));
            return true;
        }
        else if (PyUnicode_Check(objPtr)) {
            switch (PyUnicode_KIND(objPtr)) {
            case PyUnicode_1BYTE_KIND:
                value = QString::fromUtf8(PyUnicode_AsUTF8(objPtr));
                break;
            case PyUnicode_2BYTE_KIND:
                value = QString::fromUtf16(
                    reinterpret_cast<char16_t*>(PyUnicode_2BYTE_DATA(objPtr)),
                    PyUnicode_GET_LENGTH(objPtr));
                break;
            case PyUnicode_4BYTE_KIND:
                value = QString::fromUcs4(
                    reinterpret_cast<char32_t*>(PyUnicode_4BYTE_DATA(objPtr)),
                    PyUnicode_GET_LENGTH(objPtr));
                break;
            default:
                return false;
            }

            return true;
        }
        else {
            return false;
        }
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
        return PyUnicode_DecodeUTF16(reinterpret_cast<const char*>(src.utf16()),
                                     2 * src.length(), nullptr, 0);
    }

    template <class T>
    bool tryCast(QVariant& value, handle src, bool implicit)
    {
        type_caster<T> caster;
        if (caster.load(src, implicit)) {
            value = caster.value;
            return true;
        }
        return false;
    }

    bool type_caster<QVariant>::load(handle src, bool implicit)
    {
        // test for string first otherwise PyList_Check also works
        if (PyBytes_Check(src.ptr()) || PyUnicode_Check(src.ptr())) {
            value = src.cast<QString>();
            return true;
        }
        else if (PySequence_Check(src.ptr())) {
            // we could check if all the elements can be converted to QString
            // and store a QStringList in the QVariant but I am not sure that is
            // really useful.
            value = src.cast<QVariantList>();
            return true;
        }
        else if (PyMapping_Check(src.ptr())) {
            value = src.cast<QVariantMap>();
            return true;
        }
        else if (src.is(pybind11::none())) {
            value = QVariant();
            return true;
        }
        else if (PyDict_Check(src.ptr())) {
            value = src.cast<QVariantMap>();
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
        else if (PyFloat_Check(src.ptr())) {
            value = src.cast<double>();
            return true;
        }
        else if (tryCast<QColor>(value, src, implicit) ||
                 tryCast<QIcon>(value, src, implicit) ||
                 tryCast<QPixmap>(value, src, implicit)) {
            return true;
        }
        else {
            return false;
        }
    }

    handle type_caster<QVariant>::cast(QVariant var, return_value_policy policy,
                                       handle parent)
    {
        switch (var.typeId()) {
        case QMetaType::UnknownType:
            return Py_None;
        case QMetaType::Int:
        case QMetaType::Long:
            return PyLong_FromLong(var.toInt());
        case QMetaType::LongLong:
            return PyLong_FromLongLong(var.toLongLong());
        case QMetaType::UInt:
        case QMetaType::ULong:
            return PyLong_FromUnsignedLong(var.toUInt());
        case QMetaType::ULongLong:
            return PyLong_FromUnsignedLongLong(var.toULongLong());
        case QMetaType::Float:
            return PyFloat_FromDouble(var.toFloat());
        case QMetaType::Double:
            return PyFloat_FromDouble(var.toDouble());
        case QMetaType::Bool:
            return PyBool_FromLong(var.toBool());

        case QMetaType::QString:
            return type_caster<QString>::cast(var.toString(), policy, parent);

        // We need to check for StringList here because these are not considered
        // List since List is QList<QVariant> will StringList is QList<QString>:
        case QMetaType::QStringList:
            return type_caster<QStringList>::cast(var.toStringList(), policy, parent);
        case QMetaType::QVariantList:
            return type_caster<QVariantList>::cast(var.toList(), policy, parent);
        case QMetaType::QVariantMap:
            return type_caster<QVariantMap>::cast(var.toMap(), policy, parent);

        case QMetaType::QColor:
            return type_caster<QColor>::cast(var.value<QColor>(), policy, parent);

        case QMetaType::QIcon:
            return type_caster<QIcon>::cast(var.value<QIcon>(), policy, parent);

        case QMetaType::QPixmap:
            return type_caster<QPixmap>::cast(var.value<QPixmap>(), policy, parent);

        default: {
            PyErr_Format(PyExc_TypeError, "type unsupported: %d", var.userType());
            throw pybind11::error_already_set();
        }
        }
    }

}  // namespace pybind11::detail
