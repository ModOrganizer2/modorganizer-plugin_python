#ifndef PYTHONRUNNER_H
#define PYTHONRUNNER_H

#include <QString>
#include <QObject>
#include <map>
#include <dllimport.h>
#include <imoinfo.h>


class IPythonRunner {
public:
  virtual QObject *instantiate(const QString &pluginName) = 0;
  virtual bool isPythonInstalled() const = 0;
  virtual bool isPythonVersionSupported() const = 0;
};


#ifdef PYTHONRUNNER_LIBRARY
#define PYDLLEXPORT Q_DECL_EXPORT
#else // PYTHONRUNNER_LIBRARY
#define PYDLLEXPORT Q_DECL_IMPORT
#endif // PYTHONRUNNER_LIBRARY

extern "C" PYDLLEXPORT IPythonRunner *CreatePythonRunner(const MOBase::IOrganizer *moInfo, const QString &pythonDir);



#endif // PYTHONRUNNER_H
