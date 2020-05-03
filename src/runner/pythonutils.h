#ifndef PYTHONRUNNER_UTILS_H
#define PYTHONRUNNER_UTILS_H

#include <boost/python.hpp>

namespace utils {

  namespace bpy = boost::python;

  template <class Map>
  struct map_to_python {
    static PyObject* convert(const Map& map) {
      bpy::dict result;
      for (auto& entry : map) {
        result[bpy::object{ entry.first }] = bpy::object{ entry.second };
      }
      return bpy::incref(result.ptr());
    }
  };

  template <class Map>
  struct map_from_python {

    using key_type = typename Map::key_type;
    using value_type = typename Map::mapped_type;

    static void* convertible(PyObject* objPtr) {
      return PyDict_Check(objPtr) ? objPtr : nullptr;
    }

    static void construct(PyObject* objPtr, bpy::converter::rvalue_from_python_stage1_data* data) {
      void* storage = ((bpy::converter::rvalue_from_python_storage<Map>*)data)->storage.bytes;
      Map* result = new (storage) Map();
      bpy::dict source(bpy::handle<>(bpy::borrowed(objPtr)));
      bpy::list keys = source.keys();
      int len = bpy::len(keys);
      for (int i = 0; i < len; ++i) {
        bpy::object pyKey = keys[i];
        (*result)[bpy::extract<key_type>(pyKey)] = bpy::extract<value_type>(source[pyKey]);
      }

      data->convertible = storage;
    }
  };

  template <class Container>
  struct container_to_python_list {
    static PyObject* convert(const Container& container) {
      bpy::list pyList;

      try {
        for (auto& item : container)
          pyList.append(item);
      }
      catch (const bpy::error_already_set&) {
        reportPythonError();
      }

      return bpy::incref(pyList.ptr());
    }
  };


  template <class Container>
  struct container_from_python_list {

    using value_type = typename Container::value_type;

    static void* convertible(PyObject* objPtr) {
      if (PySequence_Check(objPtr)) return objPtr;
      return nullptr;
    }

    static void construct(PyObject* objPtr, bpy::converter::rvalue_from_python_stage1_data* data) {
      void* storage = ((bpy::converter::rvalue_from_python_storage<Container>*)data)->storage.bytes;
      Container* result = new (storage) Container();
      bpy::list source(bpy::handle<>(bpy::borrowed(objPtr)));
      int length = bpy::len(source);
      for (int i = 0; i < length; ++i) {
        result->push_back(bpy::extract<value_type>(source[i]));
      }

      data->convertible = storage;
    }
  };

  template <class Container>
  struct set_to_python {
    static PyObject* convert(const Container& container) {
      bpy::list pyList;

      try {
        for (auto& item : container)
          pyList.append(item);
      }
      catch (const bpy::error_already_set&) {
        reportPythonError();
      }

      return bpy::incref(pyList.ptr());
    }
  };


  template <class Container>
  struct set_from_python {

    using value_type = typename Container::value_type;

    static void* convertible(PyObject* objPtr) {
      if (PySequence_Check(objPtr)) return objPtr;
      return nullptr;
    }

    static void construct(PyObject* objPtr, bpy::converter::rvalue_from_python_stage1_data* data) {
      void* storage = ((bpy::converter::rvalue_from_python_storage<Container>*)data)->storage.bytes;
      Container* result = new (storage) Container();
      bpy::list source(bpy::handle<>(bpy::borrowed(objPtr)));
      int length = bpy::len(source);
      for (int i = 0; i < length; ++i) {
        result->insert(bpy::extract<value_type>(source[i]));
      }

      data->convertible = storage;
    }
  };

  template <class Map>
  void register_associative_container() {
    bpy::to_python_converter<Map, map_to_python<Map>>();
    bpy::converter::registry::push_back(
      &map_from_python<Map>::convertible
      , &map_from_python<Map>::construct
      , bpy::type_id<Map>());
  };

  template <class Container>
  void register_set_container() {
    bpy::to_python_converter<Container, set_to_python<Container>>();
    bpy::converter::registry::push_back(
      &set_from_python<Container>::convertible
      , &set_from_python<Container>::construct
      , bpy::type_id<Container>());
  };

  template <class Container>
  void register_sequence_container() {
    bpy::to_python_converter<Container, container_to_python_list<Container>>();
    bpy::converter::registry::push_back(
      &container_from_python_list<Container>::convertible
      , &container_from_python_list<Container>::construct
      , bpy::type_id<Container>());
  };



}

#endif