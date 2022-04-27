#ifndef PYTHON_PYBIND11_QT_HOLDER_HPP
#define PYTHON_PYBIND11_QT_HOLDER_HPP

#include <QObject>

#include <pybind11/pybind11.h>

namespace pybind11::detail::qt {

    class qobject_holder : public QObject {
        object p_;

    public:
        template <class U>
        qobject_holder(U* p) : p_{reinterpret_borrow<object>(cast(p))}
        {
            setParent(p);
        }

        ~qobject_holder()
        {
            gil_scoped_acquire s;
            p_ = std::move(none());
        }
    };

}  // namespace pybind11::detail::qt

namespace pybind11::qt {

    template <class Type>
    class qholder {
        using type = Type;

        type* qobj_;

    public:
        qholder(type* qobj) : qobj_{qobj} { new detail::qt::qobject_holder(qobj_); }

        type* get() { return qobj_; }
    };

}  // namespace pybind11::qt

PYBIND11_DECLARE_HOLDER_TYPE(T, ::pybind11::qt::qholder<T>)

#endif
