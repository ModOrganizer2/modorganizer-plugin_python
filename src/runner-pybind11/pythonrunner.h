#ifndef PYTHONRUNNER_H
#define PYTHONRUNNER_H

#include <QString>
#include <QObject>
#include <map>
#include <dllimport.h>
#include <imoinfo.h>


class IPythonRunner {
public:

  virtual QList<QObject*> load(const QString& identifier) = 0;
  virtual void unload(const QString& identifier) = 0;

  virtual bool isPythonInitialized() const = 0;

  virtual ~IPythonRunner() { }
};


#ifdef PYTHONRUNNER_LIBRARY
#define PYDLLEXPORT Q_DECL_EXPORT
#else // PYTHONRUNNER_LIBRARY
#define PYDLLEXPORT Q_DECL_IMPORT
#endif // PYTHONRUNNER_LIBRARY

extern "C" PYDLLEXPORT IPythonRunner *CreatePythonRunner();



#endif // PYTHONRUNNER_H