#ifndef PYTHONWRAPPERUTILITIES_H
#define PYTHONWRAPPERUTILITIES_H

#include <functional>

#include <boost/python.hpp>

#include <log.h>
#include <utility.h>

#include "error.h"
#include "gilock.h"

namespace details {

  /**
   * @brief Common stuffs for all basicWrapperFunction methods.
   */
  template <class ReturnType, class WrapperTypePtr, class Fn, class... Args>
  ReturnType wrapperFunctionImplementation(WrapperTypePtr wrapper, Fn fn, boost::python::object* objPtr, const char *methodName, Args... args) {
    GILock lock;
    boost::python::override implementation = wrapper->get_override(methodName);
    if (!implementation) {
      if constexpr (std::is_same_v<Fn, std::nullptr_t>) {
        throw pyexcept::MissingImplementation(wrapper->className, methodName);
      }
      else {
        return std::invoke(fn, wrapper, args...);
      }
    }
    try {
      boost::python::object result = implementation(args...);
      if (objPtr) {
        *objPtr = result;
      }
      if constexpr (!std::is_same_v<ReturnType, void>) {
        return boost::python::extract<ReturnType>(result)();
      }
    }
    catch (const boost::python::error_already_set&) {
      throw pyexcept::PythonError();
    }
    catch (...) {
      throw pyexcept::UnknownException();
    }
  }

}

/**
 * @brief Call the given method on the wrapper with the given arguments, with proper
 *     exception handling.
 */
template <class ReturnType, class WrapperType, class... Args>
ReturnType basicWrapperFunctionImplementation(const WrapperType *wrapper, const char *methodName, Args... args)
{
  return details::wrapperFunctionImplementation<ReturnType>(wrapper, nullptr, nullptr, methodName, args...);
}

/**
 * @brief Similar to the first-overload but also stores the python object in the given reference.
 */
template <class ReturnType, class WrapperType, class... Args>
ReturnType basicWrapperFunctionImplementation(const WrapperType* wrapper, boost::python::object &ref, const char* methodName, Args... args)
{
  return details::wrapperFunctionImplementation<ReturnType>(wrapper, nullptr, &ref, methodName, args...);
}

template <class ReturnType, class WrapperTypePtr, class Fn, class... Args>
ReturnType basicWrapperFunctionImplementationWithDefault(WrapperTypePtr wrapper, Fn fn, const char* methodName, Args... args)
{
  return details::wrapperFunctionImplementation<ReturnType>(wrapper, fn, nullptr, methodName, args...);
}

#endif // PYTHONWRAPPERUTILITIES_H
