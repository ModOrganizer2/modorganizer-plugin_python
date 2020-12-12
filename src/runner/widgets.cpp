#include "widgets.h"

#include <variant>

#include <boost/python.hpp>

#include "report.h"
#include "variant_helper.h"

namespace bpy = boost::python;

using namespace MOBase;

void register_widgets() {

  // TaskDialog is also in Windows System.
  using TaskDialog = MOBase::TaskDialog;

  // TaskDialog
  bpy::class_<TaskDialogButton>("TaskDialogButton", bpy::no_init)
    .def(bpy::init<QString, QString, QMessageBox::StandardButton>((bpy::arg("text"), bpy::arg("description"), bpy::arg("button"))))
    .def(bpy::init<QString, QMessageBox::StandardButton>((bpy::arg("text"), bpy::arg("button"))));

  bpy::class_<TaskDialog, TaskDialog*, boost::noncopyable>("TaskDialog", bpy::no_init)
    .def("__init__", bpy::make_constructor(+[](
      QWidget* parent, QString const& title, QString const& main,
      QString const& content, QString const& details, QMessageBox::Icon icon,
      std::vector<TaskDialogButton> const& buttons,
      std::variant<QString, std::tuple<QString, QString>> const& remember) {
        auto* dialog = new TaskDialog(parent, title);
        dialog->main(main).content(content).details(details).icon(icon);

        for (auto& button : buttons) {
          dialog->button(button);
        }

        std::visit([dialog](auto const& item) {
          QString action, file;
          if constexpr (std::is_same_v<std::decay_t<decltype(item)>, QString>) {
            action = item;
          }
          else {
            action = std::get<0>(item);
            file = std::get<1>(item);
          }
          dialog->remember(action, file);
          }, remember);

        return dialog;
      }, bpy::default_call_policies(),
        (bpy::arg("parent") = (QWidget*)nullptr, bpy::arg("title") = "", bpy::arg("main") = "", bpy::arg("content") = "",
          bpy::arg("details") = "", bpy::arg("icon") = QMessageBox::NoIcon, bpy::arg("buttons") = std::vector<TaskDialogButton>{},
          bpy::arg("remember") = "")))
    .def("setTitle", &TaskDialog::title, bpy::return_self<>(), bpy::arg("title"))
    .def("setMain", &TaskDialog::main, bpy::return_self<>(), bpy::arg("main"))
    .def("setContent", &TaskDialog::content, bpy::return_self<>(), bpy::arg("content"))
    .def("setDetails", &TaskDialog::details, bpy::return_self<>(), bpy::arg("details"))
    .def("setIcon", &TaskDialog::icon, bpy::return_self<>(), bpy::arg("icon"))
    .def("addButton", &TaskDialog::button, bpy::return_self<>(), bpy::arg("button"))
    .def("setRemember", &TaskDialog::remember, bpy::return_self<>(), (bpy::arg("action"), bpy::arg("file") = ""))
    .def("setWidth", &TaskDialog::setWidth, bpy::arg("widget"))
    .def("addContent", &TaskDialog::addContent, bpy::arg("widget"))
    .def("exec", &TaskDialog::exec)
    ;

}