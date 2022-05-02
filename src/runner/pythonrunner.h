#ifndef PYTHONRUNNER_H
#define PYTHONRUNNER_H

#include <filesystem>
#include <map>

#include <QObject>
#include <QString>
#include <QStringList>

#include <dllimport.h>
#include <imoinfo.h>

class IPythonRunner {
public:
    virtual QList<QObject*> load(const QString& identifier) = 0;
    virtual void unload(const QString& identifier)          = 0;

    // initialize Python
    //
    // paths contains the list of built-in paths for the Python library (pythonxxx.zip,
    // etc.), an empty list uses the default Python paths (e.g., the PYTHONPATH
    // environment variable)
    //
    virtual bool initialize(QStringList const& paths = {}) = 0;

    // check if the runner has been initialized, i.e., initialize() has been called and
    // succeeded
    virtual bool isInitialized() const = 0;

    virtual ~IPythonRunner() {}
};

#ifdef PYTHONRUNNER_LIBRARY
#define PYDLLEXPORT Q_DECL_EXPORT
#else  // PYTHONRUNNER_LIBRARY
#define PYDLLEXPORT Q_DECL_IMPORT
#endif  // PYTHONRUNNER_LIBRARY

// create the Python runner
//
extern "C" PYDLLEXPORT IPythonRunner* CreatePythonRunner();

#endif  // PYTHONRUNNER_H
