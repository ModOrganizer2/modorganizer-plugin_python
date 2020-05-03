#ifndef TUPLE_HELPER_H
#define TUPLE_HELPER_H

#include <boost/python.hpp>
#include <boost/python/object.hpp> //len function

namespace boost {
  namespace python {

    template <class TTuple>
    struct to_py_tuple {

      static PyObject* convert(const TTuple& c_tuple) {
        list values;
        //add all c_tuple items to "values" list
        convert_impl(c_tuple, values, std::make_index_sequence<std::tuple_size_v<TTuple>>{});
        //create Python tuple from the list
        return incref(python::tuple(values).ptr());
      }

    private:

      template <std::size_t I0, std::size_t... Is>
      static void convert_impl(const TTuple& c_tuple, list& values, std::index_sequence<I0, Is... >) {
        values.append(std::get<I0>(c_tuple));
        convert_impl(c_tuple, values, std::index_sequence<Is... >{});
      }

      static void convert_impl(const TTuple&, list& values, std::index_sequence<>) {}

    };


    template <class TTuple>
    struct from_py_sequence {

      using tuple_type = TTuple;
      using index_sequence = std::make_index_sequence<std::tuple_size_v<TTuple>>;

      static void* convertible(PyObject* py_obj) {

        if (!PySequence_Check(py_obj)) {
          return 0;
        }

        if (!PyObject_HasAttrString(py_obj, "__len__")) {
          return 0;
        }

        python::object py_sequence(handle<>(borrowed(py_obj)));

        if (std::tuple_size_v<TTuple> != len(py_sequence)) {
          return 0;
        }

        if (convertible_impl(py_sequence, index_sequence{})) {
          return py_obj;
        }
        else {
          return 0;
        }
      }

      static void construct(PyObject* py_obj, converter::rvalue_from_python_stage1_data* data) {
        typedef converter::rvalue_from_python_storage<TTuple> storage_t;
        storage_t* the_storage = reinterpret_cast<storage_t*>(data);
        void* memory_chunk = the_storage->storage.bytes;
        TTuple* c_tuple = new (memory_chunk) TTuple();
        data->convertible = memory_chunk;

        python::object py_sequence(handle<>(borrowed(py_obj)));
        construct_impl(py_sequence, *c_tuple, index_sequence{});
      }

    private:

      template <std::size_t... Is>
      static bool convertible_impl(const python::object& py_sequence, std::index_sequence<Is... >) {
        return (... && extract<std::tuple_element_t<Is, tuple_type>>(py_sequence[Is]).check());
      }

      template <std::size_t... Is>
      static void construct_impl(const python::object& py_sequence, TTuple& c_tuple, std::index_sequence<Is... >) {
        c_tuple = tuple_type{ extract<std::tuple_element_t<Is, tuple_type>>(py_sequence[Is])... };
      }

    };

    template< class TTuple>
    void register_tuple() {

      to_python_converter< TTuple, to_py_tuple<TTuple> >();

      converter::registry::push_back(&from_py_sequence<TTuple>::convertible
        , &from_py_sequence<TTuple>::construct
        , type_id<TTuple>());
    };

  }
} //boost::python

#endif//TUPLES_HPP_16_JAN_2007