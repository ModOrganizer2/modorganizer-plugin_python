#include "pythonutils.h"

#include <filesystem>
#include <set>

#include <QCoreApplication>

#include "log.h"

namespace utils {

  void show_depreciation_warning(std::string_view name, std::string_view message, bool show_once) {

    // Contains the list of filename / line number for which a depreciation warning has already been shown.
    static std::set<std::pair<std::string, int>> DeprecatedLines;
    
    // Find the caller:
    auto inspect = bpy::import("inspect");
    auto current_frame = inspect.attr("currentframe")();
    auto callable_frame = inspect.attr("getouterframes")(current_frame, 2);
    auto filename = bpy::extract<std::string>(callable_frame[-1].attr("filename"))();
    auto function = bpy::extract<std::string>(callable_frame[-1].attr("function"))();
    auto lineno = bpy::extract<int>(callable_frame[-1].attr("lineno"));

    // Only show once if requested:
    if (show_once && DeprecatedLines.contains({ filename, lineno })) {
      return;
    }

    // Register the depreciation:
    DeprecatedLines.emplace(filename, lineno);

    auto path = relative(std::filesystem::path(filename), QCoreApplication::applicationDirPath().toStdWString());

    // Show the message:
    if (message.empty()) {
      MOBase::log::warn(
        "[deprecated] {} in {} [{}:{}].", name, function, path.native(), lineno);
    }
    else {
      MOBase::log::warn(
        "[deprecated] {} in {} [{}:{}]: {}", name, function, path.native(), lineno, message);
    }
  }

}