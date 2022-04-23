#ifndef PYTHON_WRAPPERS_UIBASE_H
#define PYTHON_WRAPPERS_UIBASE_H

#include <QDir>
#include <QIcon>
#include <QString>
#include <QUrl>

#include <pybind11/pybind11.h>

#include <ipluginlist.h>
#include <isavegame.h>
#include <isavegameinfowidget.h>
#include <pluginrequirements.h>

#include "pybind11_qt/pybind11_qt.h"

namespace mo2::python {

    // this can be extended in C++, so why not in Python
    class PyPluginRequirement : public MOBase::IPluginRequirement {
    public:
        std::optional<Problem>
        check(MOBase::IOrganizer* organizer) const override
        {
            PYBIND11_OVERRIDE_PURE(std::optional<Problem>, IPluginRequirement,
                                   check, organizer);
        };
    };

    // this needs to be extendable in Python, so actually needs a wrapper
    class PySaveGame : public MOBase::ISaveGame {
    public:
        QString getFilepath() const override
        {
            PYBIND11_OVERRIDE_PURE(QString, ISaveGame, getFilepath, );
        }

        QDateTime getCreationTime() const override
        {
            PYBIND11_OVERRIDE_PURE(QDateTime, ISaveGame, getCreationTime, );
        }

        QString getName() const override
        {
            PYBIND11_OVERRIDE_PURE(QString, ISaveGame, getName, );
        }

        QString getSaveGroupIdentifier() const override
        {
            PYBIND11_OVERRIDE_PURE(QString, ISaveGame,
                                   getSaveGroupIdentifier, );
        }

        QStringList allFiles() const override
        {
            PYBIND11_OVERRIDE_PURE(QStringList, ISaveGame, allFiles, );
        }
    };

    class PySaveGameInfoWidget : public MOBase::ISaveGameInfoWidget {
    public:
        // Bring the constructor:
        using ISaveGameInfoWidget::ISaveGameInfoWidget;

        void setSave(MOBase::ISaveGame const& save) override
        {
            PYBIND11_OVERRIDE_PURE(void, ISaveGameInfoWidget, setSave, save);
        }
    };

}  // namespace mo2::python

#endif  // UIBASEWRAPPERS_H
