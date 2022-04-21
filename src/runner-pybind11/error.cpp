#ifndef Q_MOC_RUN
#include <boost/python.hpp>
#endif
#include <utility.h>
#include <QDebug>
#include "error.h"

using namespace MOBase;
namespace bpy = boost::python;

ErrWrapper & ErrWrapper::instance()
{
  static ErrWrapper err;
  return err;
}

void ErrWrapper::write(const char * message)
{
  buffer << message;
  if (buffer.tellp() != 0 && buffer.str().back() == '\n')
  {
    // actually put the string in a variable so it doesn't get destroyed as soon as we get a pointer to its data
    std::string string = buffer.str().substr(0, buffer.str().length() - 1);
    qCritical().nospace().noquote() << string.c_str();
    buffer = std::stringstream();
  }
  
  if (recordingExceptionMessage)
  {
    lastException << message;
  }
}

void ErrWrapper::startRecordingExceptionMessage()
{
  recordingExceptionMessage = true;
  lastException = std::stringstream();
}

void ErrWrapper::stopRecordingExceptionMessage()
{
  recordingExceptionMessage = false;
}

QString ErrWrapper::getLastExceptionMessage()
{
  return QString::fromStdString(lastException.str());
}
