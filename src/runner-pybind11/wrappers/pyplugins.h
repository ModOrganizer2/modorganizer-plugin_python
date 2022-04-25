#ifndef PYTHON_WRAPPERS_PYPLUGINS_H
#define PYTHON_WRAPPERS_PYPLUGINS_H

#include <pybind11/functional.h>
#include <pybind11/operators.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "../pybind11_qt/pybind11_qt.h"

#include <iinstallationmanager.h>

#include <iplugin.h>
#include <iplugindiagnose.h>
#include <ipluginfilemapper.h>
#include <iplugingame.h>
#include <iplugininstaller.h>
#include <iplugininstallercustom.h>
#include <iplugininstallersimple.h>
#include <ipluginmodpage.h>
#include <ipluginpreview.h>
#include <iplugintool.h>

// these needs to be defined in a header file for automoc - this file is included only
// in pyplugins.cpp
namespace mo2::python {

    using namespace MOBase;

    class IPyPlugin : public QObject, public IPlugin {};

    class IPyPluginFileMapper : public IPyPlugin, public IPluginFileMapper {};

    class IPyPluginDiagnose : public IPyPlugin, public IPluginDiagnose {};

    template <class PluginBase>
    class PyPluginBase : public PluginBase {
    public:
        using PluginBase::PluginBase;

        bool init(IOrganizer* organizer) override
        {
            PYBIND11_OVERRIDE_PURE(bool, PluginBase, init, organizer);
        }
        QString name() const override
        {
            PYBIND11_OVERRIDE_PURE(QString, PluginBase, name, );
        }
        QString localizedName() const override
        {
            PYBIND11_OVERRIDE(QString, PluginBase, localizedName, );
        }
        QString master() const override
        {
            PYBIND11_OVERRIDE(QString, PluginBase, master, );
        }
        std::vector<std::shared_ptr<const IPluginRequirement>> requirements() const
        {
            PYBIND11_OVERRIDE(std::vector<std::shared_ptr<const IPluginRequirement>>,
                              PluginBase, requirements, );
        }
        QString author() const override
        {
            PYBIND11_OVERRIDE_PURE(QString, PluginBase, author, );
        }
        QString description() const override
        {
            PYBIND11_OVERRIDE_PURE(QString, PluginBase, description, );
        }
        VersionInfo version() const override
        {
            PYBIND11_OVERRIDE_PURE(VersionInfo, PluginBase, version, );
        }
        QList<PluginSetting> settings() const override
        {
            PYBIND11_OVERRIDE_PURE(QList<PluginSetting>, PluginBase, settings, );
        }
        bool enabledByDefault() const override
        {
            PYBIND11_OVERRIDE(bool, PluginBase, enabledByDefault, );
        }
    };

    class PyPlugin : public PyPluginBase<IPyPlugin> {
        Q_OBJECT
        Q_INTERFACES(MOBase::IPlugin)
    };

    class PyPluginFileMapper : public PyPluginBase<IPyPluginFileMapper> {
        Q_OBJECT
        Q_INTERFACES(MOBase::IPlugin MOBase::IPluginFileMapper)
    public:
        MappingType mappings() const override
        {
            PYBIND11_OVERRIDE_PURE(MappingType, IPluginFileMapper, mappings, );
        }
    };

    class PyPluginDiagnose : public PyPluginBase<IPyPluginDiagnose> {
        Q_OBJECT
        Q_INTERFACES(MOBase::IPlugin MOBase::IPluginDiagnose)
    public:
        std::vector<unsigned int> activeProblems() const
        {
            PYBIND11_OVERRIDE_PURE(std::vector<unsigned int>, IPluginDiagnose,
                                   activeProblems, );
        }

        QString shortDescription(unsigned int key) const
        {
            PYBIND11_OVERRIDE_PURE(QString, IPluginDiagnose, shortDescription, key);
        }

        QString fullDescription(unsigned int key) const
        {
            PYBIND11_OVERRIDE_PURE(QString, IPluginDiagnose, fullDescription, key);
        }

        bool hasGuidedFix(unsigned int key) const
        {
            PYBIND11_OVERRIDE_PURE(bool, IPluginDiagnose, hasGuidedFix, key);
        }

        void startGuidedFix(unsigned int key) const
        {
            PYBIND11_OVERRIDE_PURE(void, IPluginDiagnose, startGuidedFix, key);
        }

        // we need to bring this in public scope
        using IPluginDiagnose::invalidate;
    };

    class PyPluginTool : public PyPluginBase<IPluginTool> {
        Q_OBJECT
        Q_INTERFACES(MOBase::IPlugin MOBase::IPluginTool)
    public:
        QString displayName() const override
        {
            PYBIND11_OVERRIDE_PURE(QString, IPluginTool, displayName, );
        }
        QString tooltip() const override
        {
            PYBIND11_OVERRIDE_PURE(QString, IPluginTool, tooltip, );
        }
        QIcon icon() const override
        {
            PYBIND11_OVERRIDE_PURE(QIcon, IPluginTool, icon, );
        }
        void setParentWidget(QWidget* widget) override
        {
            PYBIND11_OVERRIDE(void, IPluginTool, setParentWidget, widget);
        }
        void display() const override
        {
            PYBIND11_OVERRIDE_PURE(void, IPluginTool, display, );
        }

        // we need to bring this in public scope
        using IPluginTool::parentWidget;
    };

    class PyPluginPreview : public PyPluginBase<IPluginPreview> {
        Q_OBJECT
        Q_INTERFACES(MOBase::IPlugin MOBase::IPluginPreview)
    public:
        std::set<QString> supportedExtensions() const override
        {
            PYBIND11_OVERRIDE_PURE(std::set<QString>, IPluginPreview,
                                   supportedExtensions, );
        }

        QWidget* genFilePreview(const QString& fileName,
                                const QSize& maxSize) const override
        {
            // TODO: transfer ownership to C++
            PYBIND11_OVERRIDE_PURE(QWidget*, IPluginPreview, genFilePreview, fileName,
                                   maxSize);
        }
    };

    class PyPluginModPage : public PyPluginBase<IPluginModPage> {
        Q_OBJECT
        Q_INTERFACES(MOBase::IPlugin MOBase::IPluginModPage)
    public:
        QString displayName() const override
        {
            PYBIND11_OVERRIDE_PURE(QString, IPluginModPage, displayName, );
        }

        QIcon icon() const override
        {
            PYBIND11_OVERRIDE_PURE(QIcon, IPluginModPage, icon, );
        }

        QUrl pageURL() const override
        {
            PYBIND11_OVERRIDE_PURE(QUrl, IPluginModPage, pageURL, );
        }

        bool useIntegratedBrowser() const override
        {
            PYBIND11_OVERRIDE_PURE(bool, IPluginModPage, useIntegratedBrowser, );
        }

        bool handlesDownload(const QUrl& pageURL, const QUrl& downloadURL,
                             ModRepositoryFileInfo& fileInfo) const override
        {
            // TODO: cannot modify fileInfo from Python
            PYBIND11_OVERRIDE_PURE(bool, IPluginModPage, handlesDownload, pageURL,
                                   downloadURL, &fileInfo);
        }

        void setParentWidget(QWidget* widget) override
        {
            PYBIND11_OVERRIDE(void, IPluginModPage, setParentWidget, widget);
        }

        // we need to bring this in public scope
        using IPluginModPage::parentWidget;
    };

    // installers
    template <class PluginInstallerBase>
    class PyPluginInstallerBase : public PyPluginBase<PluginInstallerBase> {
    public:
        using PyPluginBase<PluginInstallerBase>::PyPluginBase;

        unsigned int priority() const override
        {
            PYBIND11_OVERRIDE_PURE(unsigned int, PluginInstallerBase, priority);
        }

        bool isManualInstaller() const override
        {
            PYBIND11_OVERRIDE_PURE(bool, PluginInstallerBase, isManualInstaller, );
        }

        void onInstallationStart(QString const& archive, bool reinstallation,
                                 IModInterface* currentMod)
        {
            PYBIND11_OVERRIDE(void, PluginInstallerBase, onInstallationStart, archive,
                              reinstallation, currentMod);
        }

        void onInstallationEnd(IPluginInstaller::EInstallResult result,
                               IModInterface* newMod)
        {
            PYBIND11_OVERRIDE(void, PluginInstallerBase, onInstallationEnd, result,
                              newMod);
        }

        bool isArchiveSupported(std::shared_ptr<const IFileTree> tree) const override
        {
            PYBIND11_OVERRIDE_PURE(bool, PluginInstallerBase, isArchiveSupported, tree);
        }

        // we need to bring these in public scope
        using PluginInstallerBase::manager;
        using PluginInstallerBase::parentWidget;
    };

    class PyPluginInstallerCustom
        : public PyPluginInstallerBase<IPluginInstallerCustom> {
        Q_OBJECT
        Q_INTERFACES(MOBase::IPlugin MOBase::IPluginInstallerCustom)
    public:
        bool isArchiveSupported(const QString& archiveName) const
        {
            PYBIND11_OVERRIDE_PURE(bool, IPluginInstallerCustom, isArchiveSupported,
                                   archiveName);
        }

        std::set<QString> supportedExtensions() const
        {
            PYBIND11_OVERRIDE_PURE(std::set<QString>, IPluginInstallerCustom,
                                   supportedExtensions, );
        }

        EInstallResult install(GuessedValue<QString>& modName, QString gameName,
                               const QString& archiveName, const QString& version,
                               int nexusID) override
        {
            PYBIND11_OVERRIDE_PURE(EInstallResult, IPluginInstallerCustom, install,
                                   modName, gameName, archiveName, version, nexusID);
        }
    };

    class PyPluginInstallerSimple
        : public PyPluginInstallerBase<IPluginInstallerSimple> {
        Q_OBJECT
        Q_INTERFACES(MOBase::IPlugin MOBase::IPluginInstallerSimple)
    public:
        using py_install_return_type =
            std::variant<IPluginInstaller::EInstallResult, std::shared_ptr<IFileTree>,
                         std::tuple<IPluginInstaller::EInstallResult,
                                    std::shared_ptr<IFileTree>, QString, int>>;

        EInstallResult install(GuessedValue<QString>& modName,
                               std::shared_ptr<IFileTree>& tree, QString& version,
                               int& nexusID) override
        {
            const auto result = [&, this]() {
                PYBIND11_OVERRIDE_PURE(py_install_return_type, IPluginInstallerSimple,
                                       install, modName, tree, version, nexusID);
            }();

            return std::visit(
                [&modName, &tree, &version, &nexusID](auto const& t) {
                    using type = std::decay_t<decltype(t)>;
                    if constexpr (std::is_same_v<type, EInstallResult>) {
                        return t;
                    }
                    else if constexpr (std::is_same_v<type,
                                                      std::shared_ptr<IFileTree>>) {
                        tree = t;
                        return RESULT_SUCCESS;
                    }
                    else if constexpr (std::is_same_v<
                                           type, std::tuple<EInstallResult,
                                                            std::shared_ptr<IFileTree>,
                                                            QString, int>>) {
                        tree    = std::get<1>(t);
                        version = std::get<2>(t);
                        nexusID = std::get<3>(t);
                        return std::get<0>(t);
                    }
                },
                result);
        }
    };

}  // namespace mo2::python

#endif
