#ifndef VARIANT_HELPER_H
#define VARIANT_HELPER_H

#include <boost/python.hpp>

/**
 * Register variant(s) from and to python object. Greatly inspired by register_tuple<>.
 */

namespace boost {
  namespace python {

    template <class TVariant>
    struct to_py_variant {

      static PyObject* convert(const TVariant& c_variant) {
        object value = std::visit([](auto const& value) {
          return object{ value };
        }, c_variant);
        //create Python object from the list
        return incref(value.ptr());
      }

    };

    template <class TVariant>
    struct variant_from_python;
   
    template <template <class... > class VTemplate, class... Args>
    struct variant_from_python<VTemplate<Args... >> {

      using variant_type = VTemplate<Args... >;

      static void* convertible(PyObject* py_obj) {

        python::object obj(handle<>(borrowed(py_obj)));

        if (impl<Args... >::convertible(obj)) {
          return py_obj;
        }
        else {
          return 0;
        }
      }

      static void construct(PyObject* py_obj, converter::rvalue_from_python_stage1_data* data) {
        typedef converter::rvalue_from_python_storage<variant_type> storage_t;
        storage_t* the_storage = reinterpret_cast<storage_t*>(data);
        void* memory_chunk = the_storage->storage.bytes;
        variant_type* c_variant = new (memory_chunk) variant_type();
        data->convertible = memory_chunk;

        python::object obj(handle<>(borrowed(py_obj)));
        impl<Args... >::construct(obj, *c_variant);
      }

    private:

      template <class... >
      struct impl;

      template <>
      struct impl<> {
        static bool convertible(const python::object& obj) { return false; }
        static void construct(const python::object& obj, variant_type& c_variant) {}
      };

      template <class UArg, class... UArgs>
      struct impl<UArg, UArgs... > {

        static bool convertible(const python::object& obj) {

          extract<UArg> type_checker(obj);
          if (type_checker.check()) {
            return true;
          }
          else {
            return impl<UArgs... >::convertible(obj);
          }
        }

        static void construct(const python::object& obj, variant_type& c_variant) {

          extract<UArg> type_checker(obj);

          if (type_checker.check()) {
            c_variant = type_checker();
          }
          else {
            impl<UArgs... >::construct(obj, c_variant);
          }
        }

      };

    };

    template< class TVariant>
    void register_variant() {

      to_python_converter<TVariant, to_py_variant<TVariant>>();

      converter::registry::push_back(&variant_from_python<TVariant>::convertible
        , &variant_from_python<TVariant>::construct
        , type_id<TVariant>());
    };

  }
} //boost::python

#endif//TUPLES_HPP_16_JAN_2007