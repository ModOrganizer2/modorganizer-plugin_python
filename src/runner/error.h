#ifndef ERROR_H
#define ERROR_H

#include <QString>

#include <fmt/format.h>

#include <sstream>
#include <utility.h>

struct ErrWrapper
{
  static ErrWrapper& instance();

  void write(const char* message);

  void startRecordingExceptionMessage();

  void stopRecordingExceptionMessage();

  QString getLastExceptionMessage();

  std::stringstream buffer;
  bool recordingExceptionMessage;
  std::stringstream lastException;
};

namespace pyexcept {

  /**
   * @brief Exception to throw when a python implementation does not implement
   *     a pure virtual function.
   */
  class MissingImplementation : public MOBase::MOException {
  public:
    MissingImplementation(std::string const& className, std::string const& methodName) :
      MOException(QString::fromStdString(
        fmt::format("Python class implementing \"{}\" has no implementation of method \"{}\".",
          className, methodName))) { }

  };

  /**
   * @brief Exception to throw when a python error occurs.
   */
  class PythonError : public MOBase::MOException {
  public:

    /**
     * @brief Create a new PythonError, fetching the error message from python. If the message
     *     cannot be retrieved, `defaultErrorMessage()` is used instead.
     */
    PythonError() : MOException(getPythonErrorMessage()) { }

    /**
     * @brief Create a new PythonError with the given message.
     *
     * @param message Message for the exception.
     */
    PythonError(QString message) : MOException(message) { }

  protected:

    /**
     *
     */
    static QString defaultErrorMessage() {
      return QObject::tr("An unexpected C++ exception was thrown in python code.");
    }

    /**
     *
     */
    static QString getPythonErrorMessage() {
      if (PyErr_Occurred()) {
        ErrWrapper& errWrapper = ErrWrapper::instance();

        errWrapper.startRecordingExceptionMessage();
        PyErr_Print();
        errWrapper.stopRecordingExceptionMessage();

        return errWrapper.getLastExceptionMessage();
      }
      else {
        return defaultErrorMessage();
      }
    }
  };

  /**
   * @brief Exception to throw when an unknown error occured. This is typically thrown
   *     from a catch(...) block.
   */
  class UnknownException : public MOBase::MOException {
  public:

    /**
     * @brief Create a new UnknownException with the default message.
     *
     * @see defaultErrorMessage
     */
    UnknownException() : MOException(defaultErrorMessage()) { }

    /**
     * @brief Create a new UnknownException with the given message.
     *
     * @param message Message for the exception.
     */
    UnknownException(QString message) : MOException(message) { }

  protected:

    /**
     *
     */
    static QString defaultErrorMessage() {
      return QObject::tr("An unknown exception was thrown in python code.");
    }
  };

}

#endif // ERROR_H
