#ifndef PYTHON_PYBIND11_QT_OBJECTS_HPP
#define PYTHON_PYBIND11_QT_OBJECTS_HPP

#include <pybind11/pybind11.h>

#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QIcon>
#include <QMainWindow>
#include <QPixmap>
#include <QSize>
#include <QUrl>
#include <QWidget>

#include "details/pybind11_qt_sip.h"
#include "details/pybind11_qt_utils.h"

#define PYQT_CLASS(QModule, QClass)                                            \
    namespace qt {                                                             \
        template <>                                                            \
        struct MetaData<QClass> {                                              \
            constexpr static const auto class_name = #QClass;                  \
            constexpr static const auto python_name =                          \
                const_name("PyQt6.") + const_name(#QModule) +                  \
                const_name(".") + const_name(#QClass);                         \
        };                                                                     \
    }                                                                          \
    template <>                                                                \
    struct type_caster<QClass> : qt::qt_type_caster<QClass> {                  \
    };                                                                         \
    template <>                                                                \
    struct type_caster<QClass*> : qt::qt_type_caster<QClass> {                 \
    }

namespace pybind11::detail {

    // add declarations below to create bindings - the first argument is simply
    // the name of the PyQt6 package containing the class, and is only used for
    // the python signature

    PYQT_CLASS(QtCore, QDateTime);
    PYQT_CLASS(QtCore, QDir);
    PYQT_CLASS(QtCore, QFileInfo);
    PYQT_CLASS(QtCore, QObject);
    PYQT_CLASS(QtCore, QSize);
    PYQT_CLASS(QtCore, QUrl);

    PYQT_CLASS(QtGui, QIcon);
    PYQT_CLASS(QtGui, QPixmap);

    PYQT_CLASS(QtWidgets, QMainWindow);
    PYQT_CLASS(QtWidgets, QWidget);

}  // namespace pybind11::detail

#undef METADATA

#endif
