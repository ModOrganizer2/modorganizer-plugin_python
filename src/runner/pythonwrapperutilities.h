#ifndef PYTHONWRAPPERUTILITIES_H
#define PYTHONWRAPPERUTILITIES_H

#include <utility.h>

#include "error.h"

class MissingImplementation : public MOBase::MyException {
public:
  MissingImplementation(QString className, QString methodName) : MyException("Python class implementing \"" +
                                                                             className +
                                                                             "\" has no implementation of method \"" +
                                                                             methodName + "\"") {}
};

#define PYCATCH catch (const boost::python::error_already_set &) { reportPythonError(); throw MOBase::MyException("unhandled exception"); }\
                catch (const MissingImplementation &missingImplementationException) { throw missingImplementationException; }\
                catch (...) { throw MOBase::MyException("An unknown exception was thrown in python code"); }

template <typename WrapperType, typename ReturnType, typename... Args>
ReturnType basicWrapperFunctionImplementation(const WrapperType *wrapper, const char *methodName, Args... args)
{
  try {
    GILock lock;
    boost::python::override implementation = wrapper->get_override(methodName);
    if (!implementation)
      throw MissingImplementation(wrapper->className, methodName);
    return implementation(args...).as<ReturnType>();
  } PYCATCH;
}

#endif // PYTHONWRAPPERUTILITIES_H
