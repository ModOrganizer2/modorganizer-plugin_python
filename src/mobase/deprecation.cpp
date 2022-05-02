#include "deprecation.h"

#include <filesystem>
#include <set>

#include <pybind11/pybind11.h>

#include <QCoreApplication>

#include "log.h"

namespace py = pybind11;

namespace mo2::python {

    void show_deprecation_warning(std::string_view name, std::string_view message,
                                  bool show_once)
    {

        // Contains the list of filename / line number for which a deprecation
        // warning has already been shown.
        static std::set<std::pair<std::string, int>> DeprecatedLines;

        // Find the caller:
        auto inspect                = py::module_::import("inspect");
        auto current_frame          = inspect.attr("currentframe")();
        py::sequence callable_frame = inspect.attr("getouterframes")(current_frame, 2);
        auto filename = callable_frame[-1].attr("filename").cast<std::string>();
        auto function = callable_frame[-1].attr("function").cast<std::string>();
        auto lineno   = callable_frame[-1].attr("lineno").cast<int>();

        // Only show once if requested:
        if (show_once && DeprecatedLines.contains({filename, lineno})) {
            return;
        }

        // Register the deprecation:
        DeprecatedLines.emplace(filename, lineno);

        auto path = relative(std::filesystem::path(filename),
                             QCoreApplication::applicationDirPath().toStdWString());

        // Show the message:
        if (message.empty()) {
            MOBase::log::warn("[deprecated] {} in {} [{}:{}].", name, function,
                              path.native(), lineno);
        }
        else {
            MOBase::log::warn("[deprecated] {} in {} [{}:{}]: {}", name, function,
                              path.native(), lineno, message);
        }
    }

}  // namespace mo2::python
