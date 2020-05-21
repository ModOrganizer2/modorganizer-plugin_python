#ifndef PYTHONWRAPPERUTILITIES_H
#define PYTHONWRAPPERUTILITIES_H

#include <functional>

#include <boost/python.hpp>

#include <log.h>
#include <utility.h>

#include "error.h"
#include "gilock.h"

/**
 * @brief Call the given method on the wrapper with the given arguments, with proper
 *     exception handling.
 */
template <class WrapperType, class ReturnType, class... Args>
ReturnType basicWrapperFunctionImplementation(const WrapperType *wrapper, const char *methodName, Args... args)
{
  GILock lock;
  boost::python::override implementation = wrapper->get_override(methodName);
  if (!implementation) {
    throw pyexcept::MissingImplementation(wrapper->className, methodName);
  }
  try {
    return implementation(args...).as<ReturnType>();
  }
  catch (const boost::python::error_already_set&) {
    throw pyexcept::PythonError();
  }
  catch (...) {
    throw pyexcept::UnknownException();
  }
}

/**
 * @brief Similar to the first-overload but also stores the python object in the given reference.
 */
template <class WrapperType, class ReturnType, class... Args>
ReturnType basicWrapperFunctionImplementation(const WrapperType* wrapper, boost::python::object &ref, const char* methodName, Args... args)
{
  GILock lock;
  boost::python::override implementation = wrapper->get_override(methodName);
  if (!implementation) {
    throw pyexcept::MissingImplementation(wrapper->className, methodName);
  }
  try {
    ref = implementation(args...);
    return boost::python::extract<ReturnType>(ref)();
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
  GILock lock;
  boost::python::override implementation = wrapper->get_override(methodName);

  if (!implementation) {
    return std::invoke(fn, wrapper, args...);
  }

  try {
    return implementation(args...).as<ReturnType>();
  }
  catch (const boost::python::error_already_set&) {
    throw pyexcept::PythonError();
  }
  catch (...) {
    throw pyexcept::UnknownException();
  }
}

template <class WrapperType, class ReturnType, class Fn, class... Args>
ReturnType basicWrapperFunctionImplementationWithDefault(const WrapperType* wrapper, Fn fn, const char* methodName, Args... args)
{
  GILock lock;
  boost::python::override implementation = wrapper->get_override(methodName);

  if (!implementation) {
    return std::invoke(fn, wrapper, args...);
  }

  try {
    return implementation(args...).as<ReturnType>();
  }
  catch (const boost::python::error_already_set&) {
    throw pyexcept::PythonError();
  }
  catch (...) {
    throw pyexcept::UnknownException();
  }
}

#endif // PYTHONWRAPPERUTILITIES_H
