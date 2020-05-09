#ifndef PYTHONWRAPPERUTILITIES_H
#define PYTHONWRAPPERUTILITIES_H

#include <functional>

#include <utility.h>

#include "error.h"

template <class WrapperType, class ReturnType, class... Args>
ReturnType basicWrapperFunctionImplementation(const WrapperType *wrapper, const char *methodName, Args... args)
{
  try {
    GILock lock;
    boost::python::override implementation = wrapper->get_override(methodName);
    if (!implementation)
      throw pyexcept::MissingImplementation(wrapper->className, methodName);
    return implementation(args...).as<ReturnType>();
  }
  catch (const boost::python::error_already_set&) {
    throw pyexcept::PythonError();
  }
  catch (pyexcept::MissingImplementation const& missingImplementation) {
    throw missingImplementation;
  }
  catch (...) {
    throw pyexcept::UnknownException();
  }
}

template <class WrapperType, class ReturnType, class Fn, class... Args>
ReturnType basicWrapperFunctionImplementationWithDefault(WrapperType* wrapper, Fn fn, const char* methodName, Args... args)
{
  try {
    GILock lock;
    boost::python::override implementation = wrapper->get_override(methodName);
    if (implementation) {
      return implementation(args...).as<ReturnType>();
    }
  }
  catch (const boost::python::error_already_set&) {
    throw pyexcept::PythonError();
  }
  catch (...) {
    throw pyexcept::UnknownException();
  }

  return std::invoke(fn, wrapper, args...);
}

template <class WrapperType, class ReturnType, class Fn, class... Args>
ReturnType basicWrapperFunctionImplementationWithDefault(const WrapperType* wrapper, Fn fn, const char* methodName, Args... args)
{
  try {
    GILock lock;
    boost::python::override implementation = wrapper->get_override(methodName);
    if (implementation) {
      return implementation(args...).as<ReturnType>();
    }
  }
  catch (const boost::python::error_already_set&) {
    throw pyexcept::PythonError();
  }
  catch (...) {
    throw pyexcept::UnknownException();
  }
  
  return std::invoke(fn, wrapper, args...);
}

#endif // PYTHONWRAPPERUTILITIES_H
