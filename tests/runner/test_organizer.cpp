#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "MockOrganizer.h"

using ::testing::Eq;
using ::testing::NaggyMock;
using ::testing::Return;

TEST(Organizer, Basic)
{
    MockOrganizer mock;
}
