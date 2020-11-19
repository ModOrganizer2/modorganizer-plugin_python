#ifndef PYTHONRUNNER_SHARED_PTR_CONVERTER_H
#define PYTHONRUNNER_SHARED_PTR_CONVERTER_H

#include <boost/python.hpp>

#include "error.h"
#include "gilock.h"

namespace utils {

  // Shared pointers are handled in a special way by Boost.Python since they hold
  // the wrapped Python object and only release it when the ref counter of the shared
  // ptr drops to 0 using shared_ptr_deleter.
  //
  // Unfortunately for us, this will happen outside of the Python proxy for some objects
  // and thus without the GIL lock, making everything crash, so we need a custom deleter
  // that holds the GIL while releasing the lock.
  //
  // Note that this is only useful for Python -> C++ conversion, and without this, Boost
  // will automatically wrapped the pointer. The C++ -> Python conversion is handled
  // separately by boost::python::register_ptr_to_python.

  template <class SharedPtr>
  struct shared_ptr_from_python;

  namespace details {

    struct shared_ptr_deleter_with_gil_lock : boost::python::converter::shared_ptr_deleter {

      using shared_ptr_deleter::shared_ptr_deleter;

      void operator()(void const* o) {
        GILock lock;
        shared_ptr_deleter::operator()(o);
      }

    };

    template <class SharedPtr>
    struct shared_ptr_void;

    template <class T>
    struct shared_ptr_void<std::shared_ptr<T>> { using type = std::shared_ptr<void>; };

    template <class T>
    struct shared_ptr_void<boost::shared_ptr<T>> { using type = boost::shared_ptr<void>; };

    template <class SharedPtr>
    using shared_ptr_void_t = typename shared_ptr_void<SharedPtr>::type;

  }

  template <class SharedPtr>
  struct shared_ptr_from_python
  {
    using T = typename SharedPtr::element_type;

    shared_ptr_from_python()
    {
      using namespace boost::python;
      converter::registry::insert(&convertible, &construct, type_id<SharedPtr>()
#ifndef BOOST_PYTHON_NO_PY_SIGNATURES
        , &converter::expected_from_python_type_direct<T>::get_pytype
#endif
      );
    }

  private:
    static void* convertible(PyObject* p)
    {
      if (p == Py_None)
        return p;

      return boost::python::converter::get_lvalue_from_python(p, boost::python::converter::registered<T>::converters);
    }

    static void construct(PyObject* source, boost::python::converter::rvalue_from_python_stage1_data* data)
    {
      using namespace boost::python;
      void* const storage = ((converter::rvalue_from_python_storage<SharedPtr>*)data)->storage.bytes;
      // Deal with the "None" case.
      if (data->convertible == source)
        new (storage) SharedPtr();
      else
      {
        details::shared_ptr_void_t<SharedPtr> hold_convertible_ref_count(
          (void*)0, details::shared_ptr_deleter_with_gil_lock(handle<>(borrowed(source))));
        // use aliasing constructor
        new (storage) SharedPtr(hold_convertible_ref_count,
          static_cast<T*>(data->convertible));
      }

      data->convertible = storage;
    }
  };

}

#endif