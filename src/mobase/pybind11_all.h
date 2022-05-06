#ifndef PYTHON_PYBIND11_ALL_H
#define PYTHON_PYBIND11_ALL_H

#include <filesystem>

#include <QDir>

#include <pybind11/operators.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/stl/filesystem.h>

#include "pybind11_qt/pybind11_qt.h"

#include "pybind11_utils/arg_wrapper.h"
#include "pybind11_utils/functional.h"
#include "pybind11_utils/shared_cpp_owner.h"

#include <isavegame.h>
#include <pluginrequirements.h>

namespace mo2::python {

    // struct to wrap "path" object between C++ and Python, allowing to mix
    // pathlib.Path, QString, QFileInfo and QDir when possible
    //
    class BasePathWrapper {
    protected:
        // we store a std::filesystem::path because it can be converted to most thing,
        // even though we lose basic functionality on QDir (name filter, etc.)
        std::filesystem::path path_;

    public:
        BasePathWrapper()                                  = default;
        BasePathWrapper(BasePathWrapper const&)            = default;
        BasePathWrapper& operator=(BasePathWrapper const&) = default;
        BasePathWrapper(BasePathWrapper&&)                 = default;
        BasePathWrapper& operator=(BasePathWrapper&&)      = default;

        BasePathWrapper(std::filesystem::path const& path) : path_{path} {}
        BasePathWrapper(QString const& path) : path_{path.toStdWString()} {}

        operator QString() const { return QString::fromStdWString(path_.native()); }
        operator std::filesystem::path() const { return path_; }
    };

    class FileWrapper : public BasePathWrapper {
    public:
        using BasePathWrapper::BasePathWrapper;

        FileWrapper(QFileInfo const& fileInfo)
            : BasePathWrapper(fileInfo.filesystemFilePath())
        {
        }

        operator QFileInfo() const { return QFileInfo(path_); }
    };

    class DirectoryWrapper : public BasePathWrapper {
    public:
        using BasePathWrapper::BasePathWrapper;

        DirectoryWrapper(QDir const& dir) : BasePathWrapper(dir.filesystemPath()) {}

        operator QDir() const { return QDir(path_); }
    };

    template <std::size_t... Is, class Fn>
    auto wrap_for_filepath(Fn&& fn)
    {
        return mo2::python::wrap_arguments<FileWrapper, Is...>(std::forward<Fn>(fn));
    }

    template <std::size_t... Is, class Fn>
    auto wrap_for_directory(Fn&& fn)
    {
        return mo2::python::wrap_arguments<DirectoryWrapper, Is...>(
            std::forward<Fn>(fn));
    }

}  // namespace mo2::python

MO2_PYBIND11_WRAP_ARGUMENT_CASTER(mo2::python::FileWrapper, "FileWrapper", QFileInfo,
                                  std::filesystem::path, QString);
MO2_PYBIND11_WRAP_ARGUMENT_CASTER(mo2::python::DirectoryWrapper, "DirectoryWrapper",
                                  QDir, std::filesystem::path, QString);

MO2_PYBIND11_SHARED_CPP_HOLDER(MOBase::IPluginRequirement)
MO2_PYBIND11_SHARED_CPP_HOLDER(MOBase::ISaveGame)

#endif
