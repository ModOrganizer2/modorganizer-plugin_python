#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "pythonrunner.h"

#include <QCoreApplication>

#include "MockOrganizer.h"
#include "ipluginfilemapper.h"
#include "iplugingame.h"

using namespace MOBase;

TEST(IPluginFileMapper, Simple)
{
    const auto plugins_folder = std::filesystem::path(std::getenv("PLUGIN_DIR"));

    auto runner = mo2::python::createPythonRunner();
    runner->initialize();

    // load objects
    const auto objects =
        runner->load("dummy_filemapper", plugins_folder / "dummy-filemapper.py");
    EXPECT_EQ(objects.size(), 2);

    // load the first IPluginFileMapper
    {
        ASSERT_EQ(objects[0].size(), 1);
        IPluginFileMapper* plugin = qobject_cast<IPluginFileMapper*>(objects[0][0]);
        ASSERT_NE(plugin, nullptr);

        const auto m = plugin->mappings();
        ASSERT_EQ(m.size(), 2);

        EXPECT_EQ(m[0].source, "the source");
        EXPECT_EQ(m[0].destination, "the destination");
        EXPECT_EQ(m[0].isDirectory, false);
        EXPECT_EQ(m[0].createTarget, false);

        EXPECT_EQ(m[1].source, "the other source");
        EXPECT_EQ(m[1].destination, "the other destination");
        EXPECT_EQ(m[1].isDirectory, true);
        EXPECT_EQ(m[1].createTarget, false);
    }

    // load the second one (this is cast before IPluginGame so should be before)
    {
        ASSERT_EQ(objects[1].size(), 2);
        IPluginFileMapper* plugin = qobject_cast<IPluginFileMapper*>(objects[1][0]);
        ASSERT_NE(plugin, nullptr);

        const auto m = plugin->mappings();
        ASSERT_EQ(m.size(), 1);

        EXPECT_EQ(m[0].source, "the source");
        EXPECT_EQ(m[0].destination, "the destination");
        EXPECT_EQ(m[0].isDirectory, false);
        EXPECT_EQ(m[0].createTarget, false);
    }

    // load the game plugin
    {
        IPluginGame* plugin = qobject_cast<IPluginGame*>(objects[1][1]);
        ASSERT_NE(plugin, nullptr);
    }
}
