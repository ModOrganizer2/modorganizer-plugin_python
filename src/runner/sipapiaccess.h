#ifndef SIPAPIACCESS_H
#define SIPAPIACCESS_H

#include <sip.h>

static const sipAPIDef *sipAPI()
{
  static const sipAPIDef *sipApi = nullptr;
  if (sipApi == nullptr) {
#if defined(SIP_USE_PYCAPSULE)
    PyImport_ImportModule("PyQt5.sip");
    sipApi = (const sipAPIDef *)PyCapsule_Import("PyQt5.sip._C_API", 0);
#else
    PyObject *sip_module;
    PyObject *sip_module_dict;
    PyObject *c_api;

    /* Import the SIP module. */
    sip_module = PyImport_ImportModule("PyQt5.sip");

    if (sip_module == NULL)
      return NULL;

    /* Get the module's dictionary. */
    sip_module_dict = PyModule_GetDict(sip_module);

    /* Get the "_C_API" attribute. */
    c_api = PyDict_GetItemString(sip_module_dict, "_C_API");

    if (c_api == NULL)
      return NULL;

    /* Sanity check that it is the right type. */
    if (!PyCObject_Check(c_api))
      return NULL;

    /* Get the actual pointer from the object. */
    sipApi = (const sipAPIDef *)PyCObject_AsVoidPtr(c_api);
#endif
  }

  return sipApi;
}

#endif // SIPAPIACCESS_H
