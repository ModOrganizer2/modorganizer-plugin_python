#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <uibase/iplugin.h>

#include "MockOrganizer.h"
#include "pythonrunner.h"

#include <QCoreApplication>

using namespace MOBase;

TEST(IPlugin, Basic)
{
    const auto plugins_folder = std::filesystem::path(std::getenv("PLUGIN_DIR"));

    auto runner = mo2::python::createPythonRunner();
    runner->initialize();

    // load objects
    const auto objects =
        runner->load("dummy_iplugin", plugins_folder / "dummy-iplugin.py");
    EXPECT_EQ(objects.size(), 1);

    // load the IPlugin
    const IPlugin* plugin = qobject_cast<IPlugin*>(objects[0][0]);
    EXPECT_NE(plugin, nullptr);

    EXPECT_EQ(plugin->name(), "The Name");

    // settings
    const auto settings = plugin->settings();
    EXPECT_EQ(settings.size(), 1);
    EXPECT_EQ(settings[0].key, "a setting");
    EXPECT_EQ(settings[0].description, "the setting description");
    EXPECT_EQ(settings[0].defaultValue.userType(), QMetaType::Type::Int);
    EXPECT_EQ(settings[0].defaultValue.toInt(), 12);

    // no translation, no custom implementation -> name()
    EXPECT_EQ(plugin->localizedName(), "The Name");
}
