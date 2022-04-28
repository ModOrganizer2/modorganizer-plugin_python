#ifndef PYTHONRUNNER_H
#define PYTHONRUNNER_H

#include <filesystem>
#include <map>

#include <QObject>
#include <QString>

#include <dllimport.h>
#include <imoinfo.h>

class IPythonRunner {
public:
    virtual QList<QObject*> load(const QString& identifier) = 0;
    virtual void unload(const QString& identifier)          = 0;

    // initialize Python
    //
    // libpath should be the folder containing the Python library (pythonxxx.zip, etc.)
    //
    virtual bool initialize(std::filesystem::path const& libpath) = 0;

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
