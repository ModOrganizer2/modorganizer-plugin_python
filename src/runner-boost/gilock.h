#ifndef GILOCK_H
#define GILOCK_H


#ifndef Q_MOC_RUN
#include <boost/python.hpp>
#endif // Q_MOC_RUN

class GILock {
public:
  GILock();
  ~GILock();
private:
  PyGILState_STATE m_State;
};


#endif // GILOCK_H
