#ifndef SIPAPIACCESS_H
#define SIPAPIACCESS_H

#include <sip.h>

static const sipAPIDef *sipAPI()
{
  static const sipAPIDef *sipApi = nullptr;
  if (sipApi == nullptr) {
    sipApi = (const sipAPIDef *)PyCapsule_Import("sip._C_API", 0);
  }

  return sipApi;
}

#endif // SIPAPIACCESS_H
