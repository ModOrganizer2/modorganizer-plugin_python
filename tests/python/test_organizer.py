import pytest
from PyQt6.QtWidgets import QWidget

m = pytest.importorskip("mobase_tests.organizer")


def test_getters():
    o = m.organizer()
    assert o.profileName() == "profile"
    assert o.startApplication("") == 4654
    assert o.waitForApplication(42) == (False, -1)
    assert o.waitForApplication(4654) == (True, 0)
