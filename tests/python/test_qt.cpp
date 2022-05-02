#include "pybind11_qt/pybind11_qt.h"

#include <pybind11/pybind11.h>

#include <tuple>

using namespace pybind11::literals;

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

    // QMap

    m.def("qmap_to_length", [](QMap<QString, QString> const& map) {
        QMap<QString, int> res;
        for (auto it = map.begin(); it != map.end(); ++it) {
            res[it.key()] = it.value().size();
        }
        return res;
    });

    // QDateTime

    m.def(
        "datetime_from_string",
        [](QString const& date, Qt::DateFormat format) {
            return QDateTime::fromString(date, format);
        },
        "string"_a, "format"_a = Qt::DateFormat::ISODate);

    m.def(
        "datetime_to_string",
        [](QDateTime const& datetime, Qt::DateFormat format) {
            return datetime.toString(format);
        },
        "datetime"_a, "format"_a = Qt::DateFormat::ISODate);

    // QVariant

    m.def("qvariant_from_none", [](QVariant const& variant) {
        return std::make_tuple(variant.userType() == QVariant::Invalid,
                               variant.isValid());
    });
    m.def("qvariant_from_int", [](QVariant const& variant) {
        return std::make_tuple(variant.userType() == QVariant::Int, variant.toInt());
    });
    m.def("qvariant_from_bool", [](QVariant const& variant) {
        return std::make_tuple(variant.userType() == QVariant::Bool, variant.toBool());
    });
    m.def("qvariant_from_str", [](QVariant const& variant) {
        return std::make_tuple(variant.userType() == QVariant::String,
                               variant.toString());
    });
    m.def("qvariant_from_list", [](QVariant const& variant) {
        return std::make_tuple(variant.userType() == QVariant::List, variant.toList());
    });
    m.def("qvariant_from_map", [](QVariant const& variant) {
        return std::make_tuple(variant.userType() == QVariant::Map, variant.toMap());
    });

    m.def("qvariant_none", []() {
        return QVariant();
    });
    m.def("qvariant_int", []() {
        return QVariant(42);
    });
    m.def("qvariant_bool", []() {
        return QVariant(true);
    });
    m.def("qvariant_str", []() {
        return QVariant("hello world");
    });
    m.def("qvariant_list", [] {
        QVariantMap subMap;
        subMap["bar"] = 42;
        subMap["moo"] = QVariantList{44, true};
        QVariantList list;
        list.push_back(33);
        list.push_back(QVariantList{4, "foo"});
        list.push_back(false);
        list.push_back("hello");
        list.push_back(QVariant());
        list.push_back(subMap);
        list.push_back(45);
        return QVariant(list);
    });
    m.def("qvariant_map", []() {
        QVariantMap map;
        map["bar"] = 42;
        map["moo"] = true;
        map["baz"] = "world hello";
        return map;
    });
}
