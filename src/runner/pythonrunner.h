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

extern "C" QDLLEXPORT IPythonRunner *CreatePythonRunner(const MOBase::IOrganizer *moInfo, const QString &pythonDir);


#endif // PYTHONRUNNER_H
