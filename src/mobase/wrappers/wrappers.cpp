
#include "wrappers.h"

#include "../pybind11_all.h"

#include <QDir>
#include <QIcon>
#include <QString>
#include <QUrl>

// IOrganizer must be bring here to properly compile the Python bindings of
// plugin requirements
#include <imoinfo.h>
#include <isavegame.h>
#include <isavegameinfowidget.h>
#include <pluginrequirements.h>

namespace py = pybind11;
using namespace MOBase;

namespace mo2::python {

    class PyPluginRequirement : public IPluginRequirement {
    public:
        std::optional<Problem> check(IOrganizer* organizer) const override
        {
            PYBIND11_OVERRIDE_PURE(std::optional<Problem>, IPluginRequirement, check,
                                   organizer);
        };
    };

    class PySaveGame : public ISaveGame {
    public:
        QString getFilepath() const override
        {
            PYBIND11_OVERRIDE_PURE(FileWrapper, ISaveGame, getFilepath, );
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
            PYBIND11_OVERRIDE_PURE(QString, ISaveGame, getSaveGroupIdentifier, );
        }

        QStringList allFiles() const override
        {
            PYBIND11_OVERRIDE_PURE(QStringList, ISaveGame, allFiles, );
        }

        ~PySaveGame() { std::cout << "~PySaveGame()" << std::endl; }
    };

    class PySaveGameInfoWidget : public ISaveGameInfoWidget {
    public:
        // Bring the constructor:
        using ISaveGameInfoWidget::ISaveGameInfoWidget;

        void setSave(ISaveGame const& save) override
        {
            PYBIND11_OVERRIDE_PURE(void, ISaveGameInfoWidget, setSave, &save);
        }

        ~PySaveGameInfoWidget() { std::cout << "~PySaveGameInfoWidget()" << std::endl; }
    };

    void add_wrapper_bindings(pybind11::module_ m)
    {
        // ISaveGame - custom type_caster<> for shared_ptr<> to keep the Python object
        // alive when returned from Python (see shared_cpp_owner.h)

        py::class_<ISaveGame, PySaveGame, std::shared_ptr<ISaveGame>>(m, "ISaveGame")
            .def(py::init<>())
            .def("getFilepath", &ISaveGame::getFilepath)
            .def("getCreationTime", &ISaveGame::getCreationTime)
            .def("getName", &ISaveGame::getName)
            .def("getSaveGroupIdentifier", &ISaveGame::getSaveGroupIdentifier)
            .def("allFiles", &ISaveGame::allFiles);

        // ISaveGameInfoWidget - custom holder to keep the Python object alive alongside
        // the widget

        py::class_<ISaveGameInfoWidget, PySaveGameInfoWidget,
                   py::qt::qholder<ISaveGameInfoWidget>>
            iSaveGameInfoWidget(m, "ISaveGameInfoWidget");
        iSaveGameInfoWidget.def(py::init<>())
            .def(py::init<QWidget*>(), py::arg("parent"))
            .def("setSave", &ISaveGameInfoWidget::setSave, py::arg("save"));
        py::qt::add_qt_delegate<QWidget>(iSaveGameInfoWidget, "_widget");

        // IPluginRequirement - custom type_caster<> for shared_ptr<> to keep the Python
        // object alive when returned from Python (see shared_cpp_owner.h)

        py::class_<IPluginRequirement, std::shared_ptr<IPluginRequirement>,
                   PyPluginRequirement>
            iPluginRequirementClass(m, "IPluginRequirement");

        py::class_<IPluginRequirement::Problem>(iPluginRequirementClass, "Problem")
            .def(py::init<QString, QString>(), py::arg("short_description"),
                 py::arg("long_description") = "")
            .def("shortDescription", &IPluginRequirement::Problem::shortDescription)
            .def("longDescription", &IPluginRequirement::Problem::longDescription);

        iPluginRequirementClass.def("check", &IPluginRequirement::check,
                                    py::arg("organizer"));
    }

}  // namespace mo2::python
