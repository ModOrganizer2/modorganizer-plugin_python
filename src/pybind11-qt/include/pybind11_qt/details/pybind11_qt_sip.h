#ifndef PYTHON_PYBIND11_QT_DETAILS_SIP_HPP
#define PYTHON_PYBIND11_QT_DETAILS_SIP_HPP

#include <type_traits>

#include <pybind11/pybind11.h>
#include <sip.h>

#include <iostream>
#include <pybind11/iostream.h>

#include "../pybind11_qt_holder.h"

namespace pybind11::detail::qt {

    /**
     * @brief Retrieve the SIP api.
     *
     * @return const sipAPIDef*
     */
    const sipAPIDef* sipAPI();

    template <typename T, class = void>
    struct MetaData;

    template <typename T>
    struct MetaData<T, std::enable_if_t<std::is_pointer_v<T>>>
        : MetaData<std::remove_pointer_t<T>> {};

    // template class for most Qt types that have Python equivalent (QWidget,
    // etc.)
    //
    template <class QClass>
    struct qt_type_caster {

        static constexpr bool is_pointer = std::is_pointer_v<QClass>;
        using pointer = std::conditional_t<is_pointer, QClass, QClass*>;

        QClass value;

    public:
        static constexpr auto name = MetaData<QClass>::python_name;

        operator pointer()
        {
            if constexpr (is_pointer) {
                return value;
            }
            else {
                return &value;
            }
        }

        // pybind11 requires operator T&() & and operator T&&() && but here we want to
        // use SFINAE with is_pointer so we need to template the operator
        //
        // having a template <class U> operator U&&() does not work since it will not
        // deduce the proper return type for QClass&& or QClass& so we have two separate
        // overloads, and in each one, U is actually a reference type (lvalue or rvalue)
        //

        template <class U,
                  std::enable_if_t<std::is_same_v<std::remove_reference_t<U>, QClass> &&
                                       std::is_lvalue_reference_v<U> && !is_pointer,
                                   int> = 0>
        operator U()
        {
            return value;
        }

        template <class U,
                  std::enable_if_t<std::is_same_v<std::remove_reference_t<U>, QClass> &&
                                       std::is_rvalue_reference_v<U> && !is_pointer,
                                   int> = 0>
        operator U() &&
        {
            return std::move(value);
        }

        template <typename T>
        using cast_op_type =
            std::conditional_t<is_pointer, QClass, movable_cast_op_type<T>>;

        bool load(pybind11::handle src, bool)
        {
            // special check for none for pointer classes
            if constexpr (is_pointer) {
                if (src.is_none()) {
                    value = nullptr;
                    return true;
                }
            }

            const sipTypeDef* type =
                qt::sipAPI()->api_find_type(MetaData<QClass>::class_name);
            if (type == nullptr) {
                return false;
            }
            if (!qt::sipAPI()->api_can_convert_to_type(src.ptr(), type, 0)) {
                return false;
            }

            // this would transfer responsibility for deconstructing the
            // object to C++, but pybind11 assumes l-value converters (such
            // as this) don't do that instead, this should be called within
            // the wrappers for functions which return deletable pointers.
            //
            //   sipAPI()->api_transfer_to(objPtr, Py_None);
            //
            void* data = nullptr;
            if (PyObject_TypeCheck(src.ptr(), qt::sipAPI()->api_simplewrapper_type)) {
                data = reinterpret_cast<sipSimpleWrapper*>(src.ptr())->data;
            }
            else if (PyObject_TypeCheck(src.ptr(), qt::sipAPI()->api_wrapper_type)) {
                data = reinterpret_cast<sipWrapper*>(src.ptr())->super.data;
            }

            if (data) {
                if constexpr (is_pointer) {
                    value = reinterpret_cast<QClass>(data);

                    // transfer ownership
                    sipAPI()->api_transfer_to(src.ptr(), Py_None);

                    // tie the py::object to the C++ one
                    new pybind11::detail::qt::qobject_holder_impl(value);
                }
                else {
                    value = *reinterpret_cast<QClass*>(data);
                }
                return true;
            }
            else {
                return false;
            }
        }

        template <
            typename T,
            std::enable_if_t<std::is_same<QClass, std::remove_cv_t<T>>::value, int> = 0>
        static handle cast(T* src, return_value_policy policy, handle parent)
        {
            // note: when QClass is a pointer type, e.g. a QWidget*, T is a
            // pointer to pointer, so we can defer to the standard cast()

            if (!src) {
                return none().release();
            }

            if (!is_pointer && policy == return_value_policy::take_ownership) {
                auto h = cast(std::move(*src), policy, parent);
                delete src;
                return h;
            }
            return cast(*src, policy, parent);
        }

        static pybind11::handle cast(QClass src, pybind11::return_value_policy policy,
                                     pybind11::handle /* parent */)
        {
            if constexpr (is_pointer) {
                if (!src) {
                    return none().release();
                }
            }

            const sipTypeDef* type =
                qt::sipAPI()->api_find_type(MetaData<QClass>::class_name);
            if (type == nullptr) {
                return Py_None;
            }

            PyObject* sipObj;
            void* sipData;

            if constexpr (is_pointer) {
                sipData = src;
            }
            else if (std::is_copy_assignable_v<QClass>) {
                // we send to SIP a newly allocated object, and transfer the
                // owernship to it
                sipData =
                    new QClass(policy == ::pybind11::return_value_policy::take_ownership
                                   ? std::move(src)
                                   : src);
            }
            else {
                sipData = &src;
            }

            sipObj = qt::sipAPI()->api_convert_from_type(sipData, type, 0);

            if (sipObj == nullptr) {
                return Py_None;
            }

            // ensure Python deletes the C++ component
            if constexpr (!is_pointer) {
                qt::sipAPI()->api_transfer_back(sipObj);
            }
            else {
                if (policy == return_value_policy::take_ownership) {
                    qt::sipAPI()->api_transfer_back(sipObj);
                }
            }

            return sipObj;
        }
    };

}  // namespace pybind11::detail::qt

#endif
