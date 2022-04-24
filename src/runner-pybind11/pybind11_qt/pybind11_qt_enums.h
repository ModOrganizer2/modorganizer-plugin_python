#ifndef PYTHON_PYBIND11_QT_ENUMS_HPP
#define PYTHON_PYBIND11_QT_ENUMS_HPP

#include <QColor>
#include <QMessageBox>

#include "details/pybind11_qt_enum.h"
#include "details/pybind11_qt_utils.h"

#define PYQT_ENUM(QPackage, QEnum)                                                     \
    namespace qt {                                                                     \
        template <>                                                                    \
        struct EnumData<QEnum> {                                                       \
            constexpr static const auto package =                                      \
                const_name("PyQt6.") + const_name(#QPackage);                          \
            constexpr static const auto name = qt_name_cpp2py(#QEnum);                 \
        };                                                                             \
    }                                                                                  \
    template <>                                                                        \
    struct type_caster<QEnum> : qt::qt_enum_caster<QEnum> {                            \
    }

namespace pybind11::detail {

    PYQT_ENUM(QtCore, Qt::GlobalColor);
    PYQT_ENUM(QtWidgets, QMessageBox::Icon);
    PYQT_ENUM(QtWidgets, QMessageBox::StandardButton);

}  // namespace pybind11::detail

#undef PYQT_ENUM

#endif
