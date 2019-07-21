#ifndef ERROR_H
#define ERROR_H
#include <QString>
#include <sstream>

// turn an error from the python interpreter into an exception
void reportPythonError();

struct ErrWrapper
{
  static ErrWrapper & instance();
  
  void write(const char * message);

  void startRecordingExceptionMessage();

  void stopRecordingExceptionMessage();

  QString getLastExceptionMessage();

  std::stringstream buffer;
  bool recordingExceptionMessage;
  std::stringstream lastException;
};

#endif // ERROR_H
