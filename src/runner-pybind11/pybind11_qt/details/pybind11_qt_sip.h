#ifndef PYTHON_PYBIND11_QT_DETAILS_SIP_HPP
#define PYTHON_PYBIND11_QT_DETAILS_SIP_HPP

#include <type_traits>

#include <pybind11/pybind11.h>
#include <sip.h>

#include <iostream>
#include <pybind11/iostream.h>

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
        : MetaData<std::remove_pointer_t<T>> {
    };

    // template class for most Qt types that have Python equivalent (QWidget,
    // etc.)
    //
    template <class QClass>
    struct qt_type_caster {

        static constexpr bool is_copy = std::is_copy_constructible_v<QClass>;

        std::conditional_t<is_copy, QClass, QClass*> value;

    public:
        static constexpr auto name = MetaData<QClass>::python_name;

        operator QClass*()
        {
            if constexpr (is_copy) {
                return &value;
            }
            else {
                return value;
            }
        }
        operator QClass&()
        {
            if constexpr (is_copy) {
                return value;
            }
            else {
                return *value;
            }
        }
        operator QClass&&() &&
        {
            if constexpr (is_copy) {
                return std::move(value);
            }
            else {
                return *value;
            }
        }

        template <typename T>
        using cast_op_type =
            std::conditional_t<std::is_copy_constructible_v<QClass>,
                               movable_cast_op_type<T>, QClass*>;

        bool load(pybind11::handle src, bool)
        {
            auto src_type      = type::of(src);
            auto src_type_type = type::of(src_type);

            scoped_ostream_redirect stream(
                std::cout,                             // std::ostream&
                module_::import("sys").attr("stdout")  // Python output
            );

            std::cout
                << "trying to load '" +
                       src_type.attr("__str__")().cast<std::string>() + " ["

                       + src_type_type.attr("__str__")().cast<std::string>() +
                       "]' in '" + std::string(MetaData<QClass>::class_name) +
                       "' (" + std::string(MetaData<QClass>::python_name.text) +
                       ")\n";

            // special check for none for pointer classes
            if constexpr (std::is_pointer_v<QClass>) {
                if (src.is_none()) {
                    value = nullptr;
                    return true;
                }
            }

            // this would transfer responsibility for deconstructing the
            // object to C++, but pybind11 assumes l-value converters (such
            // as this) don't do that instead, this should be called within
            // the wrappers for functions which return deletable pointers.
            //
            //   sipAPI()->api_transfer_to(objPtr, Py_None);
            //
            void* data = nullptr;
            if (PyObject_TypeCheck(src.ptr(),
                                   qt::sipAPI()->api_simplewrapper_type)) {
                data = reinterpret_cast<sipSimpleWrapper*>(src.ptr())->data;
            }
            else if (PyObject_TypeCheck(src.ptr(),
                                        qt::sipAPI()->api_wrapper_type)) {
                data = reinterpret_cast<sipWrapper*>(src.ptr())->super.data;
            }

            if (data) {
                if constexpr (std::is_pointer_v<QClass>) {
                    value = reinterpret_cast<QClass>(data);
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

        static pybind11::handle cast(QClass* src,
                                     pybind11::return_value_policy policy,
                                     pybind11::handle /* parent */)
        {

            if (!src)
                return ::pybind11::none().release();

            const sipTypeDef* type =
                qt::sipAPI()->api_find_type(MetaData<QClass>::class_name);
            if (type == nullptr) {
                return Py_None;
            }

            PyObject* sipObj;
            void* sipData;

            if constexpr (!is_copy) {
                sipData = src;
            }
            else if (std::is_copy_assignable_v<QClass>) {
                // we send to SIP a newly allocated object, and transfer the
                // owernship to it
                sipData = new QClass(
                    policy == ::pybind11::return_value_policy::take_ownership
                        ? std::move(src)
                        : src);
            }
            else {
                sipData = &src;
            }

            if constexpr (std::is_pointer_v<QClass>) {
                sipObj = qt::sipAPI()->api_convert_from_type(sipData, type, 0);
            }
            else {
                sipObj = qt::sipAPI()->api_convert_from_type(sipData, type, 0);
            }

            if (sipObj == nullptr) {
                return Py_None;
            }

            if constexpr (!std::is_pointer_v<QClass> &&
                          std::is_copy_constructible_v<QClass>) {
                // ensure Python deletes the C++ component
                qt::sipAPI()->api_transfer_back(sipObj);
            }

            return sipObj;
        }
    };

}  // namespace pybind11::detail::qt

#endif
