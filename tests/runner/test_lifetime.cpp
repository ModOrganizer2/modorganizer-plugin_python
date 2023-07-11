#include "gtest/gtest.h"

#include "MockOrganizer.h"
#include "pythonrunner.h"

#include <QCoreApplication>

TEST(Lifetime, Plugins)
{
    const auto plugins_folder = std::filesystem::path(std::getenv("PLUGIN_DIR"));

    auto runner = mo2::python::createPythonRunner();
    runner->initialize();

    {
        const auto objects =
            runner->load("dummy_iplugin", plugins_folder / "dummy-iplugin.py");

        // we found one plugin
        ASSERT_EQ(objects.size(), 1);
        ASSERT_EQ(objects[0].size(), 1);

        // check that deleting the object actually destroys it
        bool destroyed = false;
        QObject::connect(objects[0][0], &QObject::destroyed, [&destroyed]() {
            destroyed = true;
        });
        delete objects[0][0];
        EXPECT_EQ(destroyed, true);

        runner->unload("dummy_iplugin", plugins_folder / "dummy-iplugin.py");
    }

    // same things but with a parent
    {
        QObject* dummy_parent = new QObject();
        const auto objects =
            runner->load("dummy_iplugin", plugins_folder / "dummy-iplugin.py");

        // we found one plugin
        ASSERT_EQ(objects.size(), 1);
        ASSERT_EQ(objects[0].size(), 1);
        objects[0][0]->setParent(dummy_parent);

        // check that deleting the object actually destroys it
        bool destroyed = false;
        QObject::connect(objects[0][0], &QObject::destroyed, [&destroyed]() {
            destroyed = true;
        });
        delete dummy_parent;
        EXPECT_EQ(destroyed, true);
    }
}
