#ifndef PYTHONWRAPPERUTILITIES_H
#define PYTHONWRAPPERUTILITIES_H

#include <functional>

#include <boost/python.hpp>

#include <log.h>
#include <utility.h>

#include "error.h"
#include "gilock.h"
// #include "pybind11_qt/pybind11_qt.h"

namespace details {

    /**
     * @brief Common stuffs for all basicWrapperFunction methods.
     */
    template <class ReturnType, class WrapperTypePtr, class Fn, class... Args>
    ReturnType wrapperFunctionImplementation(WrapperTypePtr wrapper, bool apiTransfer,
                                             Fn fn, boost::python::object* objPtr,
                                             const char* methodName, Args... args)
    {
        boost::python::override implementation = [&]() {
            GILock lock;
            return wrapper->get_override(methodName);
        }();
        if (!implementation) {
            if constexpr (std::is_same_v<Fn, std::nullptr_t>) {
                throw pyexcept::MissingImplementation(wrapper->className, methodName);
            }
            else {
                return std::invoke(fn, wrapper, args...);
            }
        }

        GILock lock;
        try {
            boost::python::object result = implementation(args...);
            if (objPtr) {
                *objPtr = result;
            }
            else if (apiTransfer) {
                // pybind11::detail::qt::sipAPI()->api_transfer_to(result.ptr(),
                //                                                 Py_None);
            }
            if constexpr (!std::is_same_v<ReturnType, void>) {
                return boost::python::extract<ReturnType>(result)();
            }
        }
        catch (const boost::python::error_already_set&) {
            throw pyexcept::PythonError("");
        }
        catch (...) {
            throw pyexcept::UnknownException();
        }
    }

}  // namespace details

/**
 * @brief Call the given method on the wrapper with the given arguments, with
 * proper exception handling.
 *
 * @param wrapper The wrapper object to use to retrieve the python method. Must
 * have a publicly available `className` attribute.
 * @param methodName The name of the method.
 * @param args... Arguments for the method.
 *
 * @return the result of calling the given Python method on the wrapper.
 *
 * @throw pyexcept::MissingImplementation if the method does not exist.
 * @throw pyexcept::PythonError if an error occurs while executing the python
 * method.
 * @throw pyexecpt::UnknownException if an unknown error occurs.
 */
template <class ReturnType, class WrapperType, class... Args>
ReturnType basicWrapperFunctionImplementation(const WrapperType* wrapper,
                                              const char* methodName, Args... args)
{
    return details::wrapperFunctionImplementation<ReturnType>(
        wrapper, false, nullptr, nullptr, methodName, args...);
}

/**
 * @brief Call the given method on the wrapper with the given arguments, with
 * proper exception handling, and store the intermediate result in the given
 * python object.
 *
 * @param wrapper The wrapper object to use to retrieve the python method. Must
 * have a publicly available `className` attribute.
 * @param ref Python object to which the result of `get_override()` should be
 * stored.
 * @param methodName The name of the method.
 * @param args... Arguments for the method.
 *
 * @return the result of calling the given Python method on the wrapper.
 *
 * @throw pyexcept::MissingImplementation if the method does not exist.
 * @throw pyexcept::PythonError if an error occurs while executing the python
 * method.
 * @throw pyexecpt::UnknownException if an unknown error occurs.
 */
template <class ReturnType, class WrapperType, class... Args>
ReturnType basicWrapperFunctionImplementation(const WrapperType* wrapper,
                                              boost::python::object& ref,
                                              const char* methodName, Args... args)
{
    return details::wrapperFunctionImplementation<ReturnType>(
        wrapper, false, nullptr, &ref, methodName, args...);
}

/**
 * @brief Call the given method on the wrapper with the given arguments, with
 * proper exception handling, and transfer the responsibility of the returned
 * object to the C++ side.
 *
 * @param wrapper The wrapper object to use to retrieve the python method. Must
 * have a publicly available `className` attribute.
 * @param methodName The name of the method.
 * @param args... Arguments for the method.
 *
 * @return the result of calling the given Python method on the wrapper.
 *
 * @throw pyexcept::MissingImplementation if the method does not exist.
 * @throw pyexcept::PythonError if an error occurs while executing the python
 * method.
 * @throw pyexecpt::UnknownException if an unknown error occurs.
 */
template <class ReturnType, class WrapperType, class... Args>
ReturnType wrapperFunctionImplementationWithApiTransfer(const WrapperType* wrapper,
                                                        const char* methodName,
                                                        Args... args)
{
    return details::wrapperFunctionImplementation<ReturnType>(
        wrapper, true, nullptr, nullptr, methodName, args...);
}

/**
 * @brief Call the given method on the wrapper with the given arguments, with
 * proper exception handling, falling back to the given function if the method
 * does not exist.
 *
 * @param wrapper The wrapper object to use to retrieve the python method. Must
 * have a publicly available `className` attribute.
 * @param fn The function to call if the method does not exists.
 * @param methodName The name of the method.
 * @param args... Arguments for the method.
 *
 * Note: `fn` does not have to be a member-function of `wrapper` but
 * `std::invoke(fn, wrapper, args...)` must be valid.
 *
 * @return the result of calling the given Python method on the wrapper.
 *
 * @throw pyexcept::PythonError if an error occurs while executing the python
 * method.
 * @throw pyexecpt::UnknownException if an unknown error occurs.
 */
template <class ReturnType, class WrapperTypePtr, class Fn, class... Args>
ReturnType basicWrapperFunctionImplementationWithDefault(WrapperTypePtr wrapper, Fn fn,
                                                         const char* methodName,
                                                         Args... args)
{
    return details::wrapperFunctionImplementation<ReturnType>(
        wrapper, false, fn, nullptr, methodName, args...);
}

/**
 * @brief Call the given method on the wrapper with the given arguments, with
 * proper exception handling, and store the intermediate result in the given
 * python object, falling back to the given function if the method does not
 * exist.
 *
 * @param wrapper The wrapper object to use to retrieve the python method. Must
 * have a publicly available `className` attribute.
 * @param fn The function to call if the method does not exists.
 * @param ref Python object to which the result of `get_override()` should be
 * stored.
 * @param methodName The name of the method.
 * @param args... Arguments for the method.
 *
 * Note: `fn` does not have to be a member-function of `wrapper` but
 * `std::invoke(fn, wrapper, args...)` must be valid.
 *
 * @return the result of calling the given Python method on the wrapper.
 *
 * @throw pyexcept::PythonError if an error occurs while executing the python
 * method.
 * @throw pyexecpt::UnknownException if an unknown error occurs.
 */
template <class ReturnType, class WrapperType, class Fn, class... Args>
ReturnType
basicWrapperFunctionImplementationWithDefault(const WrapperType* wrapper, Fn fn,
                                              boost::python::object& ref,
                                              const char* methodName, Args... args)
{
    return details::wrapperFunctionImplementation<ReturnType>(wrapper, false, fn, &ref,
                                                              methodName, args...);
}

#endif  // PYTHONWRAPPERUTILITIES_H
