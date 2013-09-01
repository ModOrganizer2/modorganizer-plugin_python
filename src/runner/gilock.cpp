#include "gilock.h"

GILock::GILock()
{
  m_State = PyGILState_Ensure();
}

GILock::~GILock()
{
  PyGILState_Release(m_State);
}
