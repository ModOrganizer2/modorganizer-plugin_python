#ifndef PYCATCH_H
#define PYCATCH_H

#include <utility.h>

#include "error.h"

#define PYCATCH catch (const boost::python::error_already_set &) { reportPythonError(); throw MOBase::MyException("unhandled exception"); }\
                catch (...) { throw MOBase::MyException("An unknown exception was thrown in python code"); }

#endif // PYCATCH_H
