#ifndef PYTHONRUNNER_UTILS_H
#define PYTHONRUNNER_UTILS_H

#include <boost/python.hpp>

#include "error.h"

namespace utils {

  namespace bpy = boost::python;

  namespace details {

    template <class It, class = void>
    struct is_stdmap_iterator : std::false_type {};

    template <class It>
    struct is_stdmap_iterator<It, std::void_t<decltype(std::declval<It>()->first)>> : std::true_type {};

    // Note: QMap and standard maps do not have the same type of iterators:
    template <class It, std::enable_if_t<is_stdmap_iterator<It>{}, int> = 0>
    inline auto set_dict_entry(bpy::dict& result, It const& it) {
      result[bpy::object{ it->first }] = bpy::object{ it->second };
    }

    template <class It, std::enable_if_t<!is_stdmap_iterator<It>{}, int > = 0>
    inline auto set_dict_entry(bpy::dict& result, It const& it) {
      result[bpy::object{ it.key() }] = bpy::object{ it.value() };
    }

  }

  template <class Map>
  struct map_to_python {
    static PyObject* convert(const Map& map) {
      bpy::dict result;
      for (auto it = map.begin(); it != map.end(); ++it) {
        details::set_dict_entry(result, it);
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
        for (auto& item : container) {
          pyList.append(item);
        }
      }
      catch (const bpy::error_already_set&) {
        throw pyexcept::PythonError();
      }

      return bpy::incref(pyList.ptr());
    }
  };


  template <class Container>
  struct container_from_python_list {

    using value_type = typename Container::value_type;

    static void* convertible(PyObject* objPtr) {
      // Check that the object can be iterated or is a sequence. There is no "clean"
      // way checking that an object is iterable apparently (PyIter_Check checks that
      // an object is an iterator, which is very different).
      if (objPtr->ob_type->tp_iter != 0 || PySequence_Check(objPtr)) return objPtr;
      return nullptr;
    }

    static void construct(PyObject* objPtr, bpy::converter::rvalue_from_python_stage1_data* data) {
      void* storage = ((bpy::converter::rvalue_from_python_storage<Container>*)data)->storage.bytes;
      Container* result = new (storage) Container();
      bpy::object source(bpy::handle<>(bpy::borrowed(objPtr)));
      bpy::stl_input_iterator<value_type> begin(source), end;
      std::copy(begin, end, std::back_inserter(*result));
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
        throw pyexcept::PythonError();
      }

      return bpy::incref(pyList.ptr());
    }
  };


  template <class Container>
  struct set_from_python {

    using value_type = typename Container::value_type;

    static void* convertible(PyObject* objPtr) {
      // See container_from_python.
      if (objPtr->ob_type->tp_iter != 0 && PySequence_Check(objPtr)) return objPtr;
      return nullptr;
    }

    static void construct(PyObject* objPtr, bpy::converter::rvalue_from_python_stage1_data* data) {
      void* storage = ((bpy::converter::rvalue_from_python_storage<Container>*)data)->storage.bytes;
      Container* result = new (storage) Container();
      bpy::list source(bpy::handle<>(bpy::borrowed(objPtr)));
      bpy::stl_input_iterator<value_type> begin(source), end;
      std::copy(begin, end, std::inserter(*result, result->begin()));
      data->convertible = storage;
    }
  };


  template <class Optional>
  struct optional_to_python {
    static PyObject* convert(const Optional& optional) {
      if (optional) {
        return bpy::incref(bpy::object(*optional).ptr());
      }
      else {
        return bpy::incref(Py_None);
      }
    }
  };


  template <class Optional>
  struct optional_from_python {

    using value_type = typename Optional::value_type;

    static void* convertible(PyObject* objPtr) {

      if (objPtr == Py_None) {
        return objPtr;
      }

      bpy::object source(bpy::handle<>(bpy::borrowed(objPtr)));
      return bpy::extract<value_type>(source).check() ? objPtr : nullptr;
    }

    static void construct(PyObject* objPtr, bpy::converter::rvalue_from_python_stage1_data* data) {
      void* storage = ((bpy::converter::rvalue_from_python_storage<Optional>*)data)->storage.bytes;
      Optional* result = new (storage) Optional();

      bpy::object source(bpy::handle<>(bpy::borrowed(objPtr)));
      if (!source.is_none()) {
        *result = bpy::extract<value_type>(source)();
      }

      data->convertible = storage;
    }
  };


  /**
   * @brief Register from and to python converters for (at least) map, unordered_map and QMap.
   *
   * Any standard compliant associative container with key/value should work here.
   *
   * @tparam Map The container type to register.
   */
  template <class Map>
  void register_associative_container() {
    bpy::to_python_converter<Map, map_to_python<Map>>();
    bpy::converter::registry::push_back(
      &map_from_python<Map>::convertible
      , &map_from_python<Map>::construct
      , bpy::type_id<Map>());
  };

  /**
   * @brief Register from and to python converters for (at least) set, unordered_set and QSet.
   *
   * Any standard compliant associative container with key should work here.
   *
   * @tparam Map The container type to register.
   */
  template <class Container>
  void register_set_container() {
    bpy::to_python_converter<Container, set_to_python<Container>>();
    bpy::converter::registry::push_back(
      &set_from_python<Container>::convertible
      , &set_from_python<Container>::construct
      , bpy::type_id<Container>());
  };

  /**
   * @brief Register from and to python converters for sequence container.
   *
   * Any standard compliant container should work here.
   *
   * @tparam Map The container type to register.
   */
  template <class Container>
  void register_sequence_container() {
    bpy::to_python_converter<Container, container_to_python_list<Container>>();
    bpy::converter::registry::push_back(
      &container_from_python_list<Container>::convertible
      , &container_from_python_list<Container>::construct
      , bpy::type_id<Container>());
  };

  /**
   * @brief Register from and to python converters for optional.
   *
   * @tparam T The optional type (std::optional<X> or boost::optional<X>).
   */
  template <class Optional>
  void register_optional() {
    bpy::to_python_converter<Optional, optional_to_python<Optional>>();
    bpy::converter::registry::push_back(
      &optional_from_python<Optional>::convertible
      , &optional_from_python<Optional>::construct
      , bpy::type_id<Optional>());
  };


  /**
   * @brief Show a deprecation warning.
   *
   * This methods will print a warning in MO2 log containing the location of the call to
   * the deprecated function. If show_once is true, the deprecation warning will only be
   * logged the first time the function is called at this location.
   *
   * @param name Name of the deprecated function.
   * @param message Deprecation message.
   * @param show_once Only show the message once per call location.
   */
  void show_deprecation_warning(std::string_view name, std::string_view message = "", bool show_once = true);

}

#endif