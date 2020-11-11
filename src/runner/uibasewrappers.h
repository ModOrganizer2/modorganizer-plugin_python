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

#include <ipluginlist.h>
#include <isavegame.h>
#include <isavegameinfowidget.h>
#include <pluginrequirements.h>

#include "error.h"
#include "gilock.h"
#include "pythonwrapperutilities.h"

// This can be extended in C++, so why not in Python:
class IPluginRequirementWrapper : public MOBase::IPluginRequirement, public boost::python::wrapper<MOBase::IPluginRequirement>
{
public:
  static constexpr const char* className = "IPluginRequirement";
  using boost::python::wrapper<MOBase::IPluginRequirement>::get_override;

  virtual std::optional<Problem> check(MOBase::IOrganizer *o) const override {
    return basicWrapperFunctionImplementation<std::optional<Problem>>(this, "check", boost::python::ptr(o));
  };
};

// This needs to be extendable in Python, so actually needs a wrapper:
class ISaveGameWrapper : public MOBase::ISaveGame, public boost::python::wrapper<MOBase::ISaveGame>
{
public:
  static constexpr const char* className = "ISaveGameWrapper";
  using boost::python::wrapper<MOBase::ISaveGame>::get_override;

  virtual QString getFilename() const override { return basicWrapperFunctionImplementation<QString>(this, "getFilename"); };
  virtual QDateTime getCreationTime() const override { return basicWrapperFunctionImplementation<QDateTime>(this, "getCreationTime"); };
  virtual QString getSaveGroupIdentifier() const override { return basicWrapperFunctionImplementation<QString>(this, "getSaveGroupIdentifier"); };
  virtual QStringList allFiles() const override { return basicWrapperFunctionImplementation<QStringList>(this, "allFiles"); };
  virtual bool hasScriptExtenderFile() const override { return basicWrapperFunctionImplementation<bool>(this, "hasScriptExtenderFile"); };
};

// This needs a wrapper but currently I have no idea how to expose this properly to python:
class ISaveGameInfoWidgetWrapper : public MOBase::ISaveGameInfoWidget, public boost::python::wrapper<MOBase::ISaveGameInfoWidget>
{
public:
  static constexpr const char* className = "ISaveGameInfoWidgetWrapper";
  using boost::python::wrapper<MOBase::ISaveGameInfoWidget>::get_override;

  // Bring the constructor:
  using ISaveGameInfoWidget::ISaveGameInfoWidget;

  virtual void setSave(QString const& save) override { basicWrapperFunctionImplementation<void>(this, "setSave", save); };
};

#endif // UIBASEWRAPPERS_H
