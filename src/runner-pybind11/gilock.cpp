#include "gilock.h"

GILock::GILock()
{
  m_State = PyGILState_Ensure();
}

GILock::~GILock()
{
  PyErr_Clear();
  PyGILState_Release(m_State);
}
