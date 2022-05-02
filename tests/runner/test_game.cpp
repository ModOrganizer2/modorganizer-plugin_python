#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "pythonrunner.h"

#include <QCoreApplication>

#include "MockOrganizer.h"
#include "iplugingame.h"
#include "localsavegames.h"
#include "moddatachecker.h"
#include "moddatacontent.h"
#include "savegameinfo.h"

using namespace MOBase;

TEST(IPluginGame, Simple)
{
    const auto plugins_folder = QString(std::getenv("PLUGIN_DIR"));

    std::unique_ptr<IPythonRunner> runner(CreatePythonRunner());
    runner->initialize();

    // load objects
    const auto objects = runner->load(plugins_folder + "/dummy-game.py");
    EXPECT_EQ(objects.size(), 1);

    // load the IPlugin
    IPluginGame* plugin = qobject_cast<IPluginGame*>(objects[0]);
    EXPECT_NE(plugin, nullptr);

    // check features
    EXPECT_EQ(plugin->feature<LocalSavegames>(), nullptr);
    EXPECT_EQ(plugin->feature<ModDataContent>(), nullptr);
    EXPECT_NE(plugin->feature<ModDataChecker>(), nullptr);
    EXPECT_NE(plugin->feature<SaveGameInfo>(), nullptr);
}
