#include "pybind11_qt/pybind11_qt.h"

#include <pybind11/pybind11.h>

PYBIND11_MODULE(qt, m)
{
    // QString

    m.def("qstring_to_stdstring", [](QString const& qstring) {
        return qstring.toStdString();
    });

    m.def("stdstring_to_qstring", [](std::string const& string) {
        return QString::fromStdString(string);
    });
    m.def("qstring_to_int", [](QString const& qstring) {
        return qstring.toInt();
    });
    m.def("int_to_qstring", [](int value) {
        return QString::number(value);
    });

    // QStringList

    m.def("qstringlist_join", [](QStringList const& values, QString const& sep) {
        return values.join(sep);
    });

    m.def("qstringlist_at", [](QStringList const& values, int index) {
        return values.at(index);
    });
}
