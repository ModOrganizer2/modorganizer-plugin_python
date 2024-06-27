#include "pybind11_qt/pybind11_qt.h"

#include <pybind11/pybind11.h>

#include <QMap>
#include <QWidget>

#include "MockOrganizer.h"

namespace py = pybind11;
using namespace pybind11::literals;
using ::testing::NiceMock;

PYBIND11_MODULE(organizer, m)
{
    using ::testing::_;
    using ::testing::Eq;
    using ::testing::Return;

    m.def(
        "organizer",
        []() -> IOrganizer* {
            MockOrganizer* mock = new NiceMock<MockOrganizer>();
            ON_CALL(*mock, profileName).WillByDefault([&mock]() {
                return "profile";
            });
            const auto handle = (HANDLE)std::uintptr_t{4654};
            EXPECT_CALL(*mock, startApplication(Eq("valid.exe"), _, _, _, _, _))
                .WillRepeatedly(Return(handle));
            EXPECT_CALL(*mock, startApplication(Eq("invalid.exe"), _, _, _, _, _))
                .WillRepeatedly(Return(INVALID_HANDLE_VALUE));
            ON_CALL(*mock, waitForApplication)
                .WillByDefault([&mock, original_handle = handle](
                                   HANDLE handle, bool refresh, LPDWORD exitCode) {
                    if (handle == original_handle) {
                        *exitCode = 0;
                        return true;
                    }
                    else {
                        *exitCode = -1;
                        return false;
                    }
                });

            return mock;
        },
        py::return_value_policy::take_ownership);
}
