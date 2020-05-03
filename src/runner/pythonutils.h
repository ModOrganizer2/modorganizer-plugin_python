#ifndef PYTHONRUNNER_UTILS_H
#define PYTHONRUNNER_UTILS_H

#include <boost/python.hpp>

namespace utils {

  template <class Map>
  struct map_to_python {
    static PyObject* convert(const Map& map) {
      boost::python::dict result;
      for (auto& entry : map) {
        result[boost::python::object{ entry.first }] = boost::python::object{ entry.second };
      }
      return boost::python::incref(result.ptr());
    }
  };

  template <class Map>
  struct map_from_python {

    using key_type = typename Map::key_type;
    using value_type = typename Map::mapped_type;

    static void* convertible(PyObject* objPtr) {
      return PyDict_Check(objPtr) ? objPtr : nullptr;
    }

    static void construct(PyObject* objPtr, boost::python::converter::rvalue_from_python_stage1_data* data) {
      void* storage = ((boost::python::converter::rvalue_from_python_storage<Map>*)data)->storage.bytes;
      Map* result = new (storage) Map();
      boost::python::dict source(boost::python::handle<>(boost::python::borrowed(objPtr)));
      boost::python::list keys = source.keys();
      int len = boost::python::len(keys);
      for (int i = 0; i < len; ++i) {
        boost::python::object pyKey = keys[i];
        (*result)[boost::python::extract<key_type>(pyKey)] = boost::python::extract<value_type>(source[pyKey]);
      }

      data->convertible = storage;
    }
  };

  template <class Map>
  void register_map() {

    boost::python::to_python_converter<Map, map_to_python<Map>>();

    boost::python::converter::registry::push_back(
      &map_from_python<Map>::convertible
      , &map_from_python<Map>::construct
      , boost::python::type_id<Map>());
  };


}

#endif