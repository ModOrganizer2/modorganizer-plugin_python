#ifndef UIBASEWRAPPERS_H
#define UIBASEWRAPPERS_H


#ifndef Q_MOC_RUN
#pragma warning (push, 0)
#include <boost/python.hpp>
#pragma warning (pop)
#endif

#include <QIcon>
#include <QString>
#include <QUrl>
#include <QDir>

#include "iplugingame.h"
#include <imoinfo.h>
#include <imodrepositorybridge.h>
#include <imodinterface.h>
#include <iinstallationmanager.h>
#include <idownloadmanager.h>
#include <ipluginlist.h>
#include <imodlist.h>
#include <isavegame.h>
#include <isavegameinfowidget.h>
#include "ifiletree.h"

#include "error.h"
#include "gilock.h"
#include "pythonwrapperutilities.h"

extern MOBase::IOrganizer *s_Organizer;

// This needs to be extendable in Python, so actually needs a wrapper (everything else probably doesn't):
class ISaveGameWrapper : public MOBase::ISaveGame, public boost::python::wrapper<MOBase::ISaveGame>
{
public:
  static constexpr const char* className = "ISaveGameWrapper";
  using boost::python::wrapper<MOBase::ISaveGame>::get_override;

  virtual QString getFilename() const override { return basicWrapperFunctionImplementation<ISaveGameWrapper, QString>(this, "getFilename"); };
  virtual QDateTime getCreationTime() const override { return basicWrapperFunctionImplementation<ISaveGameWrapper, QDateTime>(this, "getCreationTime"); };
  virtual QString getSaveGroupIdentifier() const override { return basicWrapperFunctionImplementation<ISaveGameWrapper, QString>(this, "getSaveGroupIdentifier"); };
  virtual QStringList allFiles() const override { return basicWrapperFunctionImplementation<ISaveGameWrapper, QStringList>(this, "allFiles"); };
  virtual bool hasScriptExtenderFile() const override { return basicWrapperFunctionImplementation<ISaveGameWrapper, bool>(this, "hasScriptExtenderFile"); };
};

// This needs a wrapper but currently I have no idea how to expose this properly to python:
class ISaveGameInfoWidgetWrapper : public MOBase::ISaveGameInfoWidget, public boost::python::wrapper<MOBase::ISaveGameInfoWidget>
{
public:
  static constexpr const char* className = "ISaveGameInfoWidgetWrapper";
  using boost::python::wrapper<MOBase::ISaveGameInfoWidget>::get_override;

  // Bring the constructor:
  using ISaveGameInfoWidget::ISaveGameInfoWidget;

  virtual void setSave(QString const& save) override { basicWrapperFunctionImplementation<ISaveGameInfoWidgetWrapper, void>(this, "setSave", save); };
};

#endif // UIBASEWRAPPERS_H
