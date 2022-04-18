#ifndef PYTHON_CONVERTERS_QT_SIP_HPP
#define PYTHON_CONVERTERS_QT_SIP_HPP

#include <pybind11/pybind11.h>

#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QIcon>
#include <QMainWindow>
#include <QObject>
#include <QPixmap>
#include <QSize>
#include <QUrl>
#include <QWidget>

#include <sip.h>

namespace mo2::details {

  /**
   * @brief Retrieve the SIP api.
   *
   * @return const sipAPIDef*
   */
  const sipAPIDef* sipAPI();

  template <typename T, class = void> struct MetaData;

  template <typename T>
  struct MetaData<T, std::enable_if_t<std::is_pointer_v<T>>> :
    MetaData<std::remove_pointer_t<T>> {};

#define METADATA(QClass) template <> struct MetaData<QClass> { \
  constexpr static const char name[] = #QClass; }

  METADATA(QObject);
  METADATA(QWidget);
  METADATA(QMainWindow);
  METADATA(QDateTime);
  METADATA(QDir);
  METADATA(QFileInfo);
  METADATA(QIcon);
  METADATA(QSize);
  METADATA(QUrl);
  METADATA(QPixmap);

#undef METADATA

  // template class for most Qt types that have Python equivalent (QWidget, etc.)
  //
  template <class QtType>
  struct qt_type_caster {
  public:
    PYBIND11_TYPE_CASTER(QtType, pybind11::detail::const_name(MetaData<QtType>::name));

    bool load(pybind11::handle src, bool) {
      // this would transfer responsibility for deconstructing the object to C++,
      // but pybind11 assumes l-value converters (such as this) don't do that
      // instead, this should be called within the wrappers for functions which return
      // deletable pointers.
      //
      //   sipAPI()->api_transfer_to(objPtr, Py_None);
      //
      void* data = nullptr;
      if (PyObject_TypeCheck(src.ptr(), mo2::details::sipAPI()->api_simplewrapper_type)) {
        data = reinterpret_cast<sipSimpleWrapper*>(src.ptr())->data;
      }
      else if (PyObject_TypeCheck(src.ptr(), mo2::details::sipAPI()->api_wrapper_type)) {
        data = reinterpret_cast<sipWrapper*>(src.ptr())->super.data;
      }

      if (data) {
        if constexpr (std::is_pointer_v<QtType>) {
          value = reinterpret_cast<QtType>(data);
        }
        else {
          value = *reinterpret_cast<QtType*>(data);
        }
        return true;
      }
      else {
        return false;
      }
    }

    static pybind11::handle cast(
      QtType src, pybind11::return_value_policy /* policy */, pybind11::handle /* parent */) {

      const sipTypeDef* type =
        mo2::details::sipAPI()->api_find_type(MetaData<QtType>::name);
      if (type == nullptr) {
        return Py_None;
      }

      PyObject* sipObj;
      void* sipData;

      if constexpr (std::is_pointer_v<QtType>) {
        sipData = src;
      } else if (std::is_copy_assignable_v<QtType>) {
        // we send to SIP a newly allocated object, and transfer the owernship to it
        sipData = new QtType(src);
      }
      else {
        sipData = &src;
      }

      if constexpr (std::is_pointer_v<QtType>) {
        sipObj = mo2::details::sipAPI()->api_convert_from_type(sipData, type, 0);
      }
      else {
        sipObj = mo2::details::sipAPI()->api_convert_from_type(sipData, type, 0);
      }

      if (sipObj == nullptr) {
        return Py_None;
      }

      if constexpr (!std::is_pointer_v<QtType> && std::is_copy_constructible_v<QtType>) {
        // ensure Python deletes the C++ component
        mo2::details::sipAPI()->api_transfer_back(sipObj);
      }

      return sipObj;
    }
  };

}

namespace pybind11::detail {

  template <>
  struct type_caster<QObject*> : mo2::details::qt_type_caster<QObject*> {};

  template <>
  struct type_caster<QWidget*> : mo2::details::qt_type_caster<QWidget*> {};

  template <>
  struct type_caster<QMainWindow*> : mo2::details::qt_type_caster<QMainWindow*> {};

  template <>
  struct type_caster<QDateTime> : mo2::details::qt_type_caster<QDateTime> {};

  template <>
  struct type_caster<QDir> : mo2::details::qt_type_caster<QDir> {};

  template <>
  struct type_caster<QFileInfo> : mo2::details::qt_type_caster<QFileInfo> {};
  template <>
  struct type_caster<QIcon> : mo2::details::qt_type_caster<QIcon> {};

  template <>
  struct type_caster<QPixmap> : mo2::details::qt_type_caster<QPixmap> {};

  template <>
  struct type_caster<QSize> : mo2::details::qt_type_caster<QSize> {};

  template <>
  struct type_caster<QUrl> : mo2::details::qt_type_caster<QUrl> {};

} // namespace pybind11::detail

#endif
